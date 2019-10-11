<?php

namespace ProvisionOps\YamlTests;

use Github\Exception\RuntimeException;
use ProvisionOps\Tools\PowerProcess as Process;
use ProvisionOps\Tools\Style;
use GuzzleHttp\Psr7\Response;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Composer\Command\BaseCommand;
use Symfony\Component\Yaml\Yaml;
use TQ\Git\Repository\Repository;

// @TODO: Figure out why our plugin isn't properly autoloading.
if (file_exists(__DIR__.'/../../../../vendor/autoload.php')) {
    $autoloaderPath = __DIR__.'/../../../../vendor/autoload.php';
} elseif (file_exists(__DIR__.'/vendor/autoload.php')) {
    $autoloaderPath = __DIR__.'/vendor/autoload.php';
} elseif (file_exists(__DIR__.'/../../autoload.php')) {
    $autoloaderPath = __DIR__ . '/../../autoload.php';
} elseif (file_exists(__DIR__.'/../vendor/autoload.php')) {
    $autoloaderPath = __DIR__ . '/../vendor/autoload.php';
} else {
    die("Could not find autoloader. Run 'composer install'.");
}

require_once $autoloaderPath;

/**
 * Class Command
 */
class Command extends BaseCommand
{
    const GITHUB_COMMENT_MAX_SIZE = 65536;

    protected $createTag = false;
    protected $tagName = null;
    protected $branchName;
    protected $commitMessage;
    protected $excludeFileTemp;

    /**
     * @var SymfonyStyle
     */
    protected $io;

    /**
     * The directory containing composer.json. Loaded from composer option --working-dir.
     *
     * @var String
     */
    protected $workingDir;

    /**
     * The directory at the root of the git repository.
     *
     * @var String
     */
    protected $gitDir;

    /**
     * @var Repository
     */
    protected $gitRepo;

    /**
     * The current git commit SHA.
     *
     * @var String
     */
    protected $repoSha;

    /**
     * The "name" of the repo, when using the scheme "owner/name"
     *
     * @var String
     */
    protected $repoName;

    /**
     * The "owner" of the repo, when using the scheme "owner/name"
     *
     * @var String
     */
    protected $repoOwner;
    /**
     * The pull request data associated with the current local branch.
     *
     * @var Array
     */
    protected $pullRequest;

    /**
     * The options from the project's composer.json "config" section.
     *
     * @var array
     */
    protected $config = [];

    /** @var \Github\Client */
    protected $githubClient;

    private $addTokenUrl = "https://github.com/settings/tokens/new?description=yaml-tests&scopes=repo:status,public_repo";

    protected function configure()
    {
        $this->setName('yaml-tests');
        $this->setDescription('Read tests.yml and runs all commands in it, passing results to GitHub Commit Status API.');

        $this->addOption(
            'tests-file',
            null,
            InputOption::VALUE_OPTIONAL,
            'Relative path to a yml file to run.',
            'tests.yml'
        );
        $this->addOption(
            'github-token',
            null,
            InputOption::VALUE_REQUIRED,
            'An active github token. Create a new token at ' . $this->addTokenUrl
        );
        $this->addOption(
            'ignore-dirty',
            null,
            InputOption::VALUE_NONE,
            'Allow testing even if git working copy is dirty (has modified files).'
        );
        $this->addOption(
            'dry-run',
            null,
            InputOption::VALUE_NONE,
            'Run tests but do not post to GitHub.'
        );
        $this->addOption(
            'ignore-ssl',
            null,
            InputOption::VALUE_NONE,
            'Ignore SSL certificate validation errors. Use only if you receive errors trying to reach the GitHub API with this tool.'
        );
        $this->addOption(
            'hostname',
            null,
            InputOption::VALUE_OPTIONAL,
            'The hostname to use in the status description. Use if automatically detected hostname is not desired.',
            gethostname()
        );
        $this->addArgument(
            'filter',
            InputArgument::IS_ARRAY,
            'A list of strings to filter tests by.'
        );
    }

    /**
     *
     */
    public function initialize(InputInterface $input, OutputInterface $output)
    {

        $this->io = new Style($input, $output);
        $this->input = $input;
        $this->output = $input;
        $this->logger = $this->io;
        $this->workingDir = getcwd();

        $this->gitRepo = Repository::open($this->workingDir);
        $this->gitRepo->getCurrentCommit();

        $this->config = $this->getComposer()->getPackage()->getConfig();

        $this->testsFile = $input->getOption('tests-file');
        $this->testsFilePath = realpath($this->testsFile);
        if (!file_exists($this->testsFilePath) || empty($this->testsFilePath)) {
            throw new \Exception("Specified tests file does not exist at {$this->workingDir}/{$this->testsFile}");
        }

        // Validate YML
        $this->loadTestsYml();

        // Load Environment variables
        $dotenv = new \Dotenv\Dotenv(__DIR__);
        $dotenv->safeLoad(array(

          // Current user's home directory
          isset($_SERVER['HOME'])? $_SERVER['HOME']: '',

          // Git repo holding the tests file.
          dirname($this->gitRepo->getRepositoryPath()),

          // Current directory
          getcwd(),
        ));

        // Look for token.
        if (!empty($_SERVER['GITHUB_TOKEN'])) {
            $token = $_SERVER['GITHUB_TOKEN'];
        } else {
            $token = $input->getOption('github-token');
        }

        // This is the actual SHA of the working copy clone.
        $this->repoSha = $this->gitRepo->getCurrentCommit();

        // Detect a TRAVIS_PULL_REQUEST_SHA
        // Travis tests from a commit created from master and our commit.
        // It's not the same commit as the pull request branch.
        if (!empty($_SERVER['TRAVIS_PULL_REQUEST_SHA']) && $this->gitRepo->getRepositoryPath() == $_SERVER['TRAVIS_BUILD_DIR']) {
            $this->repoSha = $_SERVER['TRAVIS_PULL_REQUEST_SHA'];
            $this->warningLite("Travis PR detected. Using PR SHA: " . $this->repoSha);
        }

        // Parse remote to retrieve git repo "owner" and "name".
        // @TODO: This is hard coded to GitHub right now. Must support other hosts eventually.
        $remotes = $this->gitRepo->getCurrentRemote();
        $remote_url = current($remotes)['push'];

        $remote_url = strtr(
            $remote_url,
            array(
            'git@' => 'https://',
            'git://' => 'https://',
            '.git' => '',
            'github.com:' => 'github.com/',
            )
        );

        $parts = explode('/', parse_url($remote_url, PHP_URL_PATH));
        if (isset($parts[1]) && isset($parts[2])) {
            $this->repoOwner = isset($parts[1])? $parts[1]: '';
            $this->repoName =isset($parts[2])? $parts[2]: '';
        } else {
            $this->repoOwner = '';
            $this->repoName = '';
        }

        $this->io->title("Yaml Tests Initialized");

        // Force dry run if there is no token set.
        if (empty($token)) {
            $input->setOption('dry-run', true);
            $this->warningLite('No GitHub token found. forcing --dry-run');
            $this->io->writeln('');
        }

        $this->say("Git Remote: <comment>{$remote_url}</comment>");
        $this->say("Local Git Branch: <comment>{$this->gitRepo->getCurrentBranch()}</comment>");
        $this->say("Composer working directory: <comment>{$this->workingDir}</comment>");
        $this->say("Git Repository directory: <comment>{$this->workingDir}</comment>");
        $this->say("Git Commit: <comment>{$this->gitRepo->getCurrentCommit()}</comment>");
        $this->say("Tests File: <comment>{$this->testsFilePath}</comment>");

        // @TODO: Dry run could still read info from the repo.
        if (!$input->getOption('dry-run')) {
            $this->githubClient = new \Github\Client();

            if ($input->getOption('ignore-ssl')) {
                $this->githubClient->getHttpClient()->client->setDefaultOption('verify', false);
            }

            $this->githubClient->authenticate($token, \Github\Client::AUTH_HTTP_TOKEN);

            // Load the commit object. Catch an exception, and change the message. Our users will wonder, "but there is a commit!"
            try {
                $commit = $this->githubClient->repository()->commits()->show($this->repoOwner, $this->repoName, $this->repoSha);
            } catch (RuntimeException $exception) {
                throw new RuntimeException("Commit not found in the remote repository. Yaml-tests cannot post commit status until the commits are pushed to the remote repository.");
            }

            $this->say("GitHub Commit URL: <comment>" . $commit['html_url'] . "</>");

            // Load Repo info to determine if it is a fork. We must post to the fork's parent in the API.
            $repo = $this->githubClient->repository()->show($this->repoOwner, $this->repoName);
            if (!empty($repo['parent'])) {
                $this->successLite('Forked repository. Posting to the parent repo...');
                $this->repoOwner = $repo['parent']['owner']['login'];
                $this->repoName = $repo['parent']['name'];
            }

            // Lookup Pull Request, if there is one.
            $string = $this->repoOwner . ':' .  $this->gitRepo->getCurrentBranch();
            $prs = $this->githubClient->pullRequests()->all($this->repoOwner, $this->repoName, array(
              'head' => $string,
            ));

            if (empty($prs)) {
                $this->warningLite("No pull requests were found using the current local branch <comment>{$this->gitRepo->getCurrentBranch()}</comment>. Make sure a Pull Request has been created in addition to the branch being pushed. Errors will be sent as comments on the Commit, instead of on the Pull Request. This means error logs will appear on any Pull Request that contains the commit being tested.");
            } else {
                $this->pullRequest = $prs[0];
            }
        }

        $this->io->table(array("Tests found in " . $this->testsFile), $this->testsToTableRows());

        // If there are filters, shorten the list of tests to run.
        $filters = $input->getArgument('filter');
        $filter_string = implode(' ', $filters);
        if (count($filters)) {
            foreach ($this->yamlTests as $name => $test) {
                $run_the_test = false;
                foreach ($filters as $filter) {
                    // If the filter string was found in the test name, run the test.
                    if (strpos($name, $filter) !== false) {
                        $run_the_test = true;
                    }
                }

                if (!$run_the_test) {
                    unset($this->yamlTests[$name]);
                }
            }
        }

        // If there are no matches
        if (count($filters) && count($this->yamlTests) > 0) {
            $this->io->table(array("Tests to run based on filter '$filter_string'"), $this->testsToTableRows());
        } elseif (count($filters)) {
            // If there are filters but tests were NOT removed, show a warning.
            $this->warningLite("The filter '$filter_string' was specified but it did not match any tests.");
            exit(1);
        }
    }

    /**
     * @param \Symfony\Component\Console\Input\InputInterface   $input
     * @param \Symfony\Component\Console\Output\OutputInterface $output
     */
    protected function execute(InputInterface $input, OutputInterface $output)
    {
        $tests_failed = false;

        try {
            if (!$input->getOption('dry-run')) {
                $client = $this->githubClient;

                foreach ($this->yamlTests as $test_name => $test) {
                    // Set a commit status for this REF
                    $params = new \stdClass();
                    $params->state = 'pending';
                    $params->target_url = 'https:///path/to/file';
                    $params->description = implode(
                        ' — ',
                        array(
                        $input->getOption('hostname'),
                        !empty($test['description'])? $test['description']: $test_name
                        )
                    );
                    $params->context = $test_name;

                    // Post status to github
                    try {
                        /**
                         * @var Response $response
                         */
                        $response = $client->getHttpClient()->post("/repos/{$this->repoOwner}/{$this->repoName}/statuses/$this->repoSha", [], json_encode($params));
                        $this->commitStatusMessage($response, $test_name, $test, $params->state);
                    } catch (\Exception $e) {
                        if ($e->getCode() == 404) {
                            throw new \Exception('Unable to reach commit status API. Check the allowed scopes of your GitHub Token. Skip github interaction with --dry-run, or create a new token with the right scopes at ' . $this->addTokenUrl);
                        }
                    }
                    $tests[] = $test_name;
                }
            } else {
                $this->warningLite('Skipping commit status posting, dry-run enabled.');
            }

            $this->io->newLine();
            $rows = array();

            foreach ($this->yamlTests as $test_name => $test) {
                if (is_array($test) && isset($test['command'])) {
                    $command = $test['command'];
                } elseif (is_array($test)) {
                    $command = implode(' && ', $test);
                } else {
                    $command = $test;
                }

                $results_row = array(
                    $test_name,
                    $command,
                );

                $process = new Process($command, $this->io);
                $process->setTimeout(null);
                $process->setIo($this->io);

                $process->setEnv($_SERVER);

                $title = "Running test <fg=white>$test_name</>";

                if (!empty($test['description'])) {
                    $title .= ": <fg=white>{$test['description']}</>";
                }

                $this->io->section($title);

                $exit = $process->run();

                // Set a commit status for this REF
                $params = new \stdClass();
                $params->state = 'pending';
                $params->target_url = 'https:///';
                $params->description = implode(
                    ' — ',
                    array(
                    $input->getOption('hostname'),
                    !empty($test['description'])? $test['description']: $test_name
                    )
                );
                $params->context = $test_name;

                if ($exit == 0) {
                    $results_row[] = '<info>✔</info> Passed';
                    $params->state = 'success';
                } else {
                    // If the test has the ignore failure flag, ignore it.
                    if (!empty($test['ignore-failure'])) {
                        $results_row[] = '<fg=red>✘</> Failed (Ignoring)';
                        $params->state = 'success';
                        $params->description .= ' | TEST FAILED but is set to ignore.';
                    } else {
                        $results_row[] = '<fg=red>✘</> Failed';
                        $tests_failed = true;
                        $params->state = 'failure';
                    }

                    if (!$input->getOption('dry-run')) {
                        // @TODO: Make the commenting optional/configurable
                        // Write a comment on the commit with the results
                        // @see https://developer.github.com/v3/repos/comments/#create-a-commit-comment

                        $comment = array();
                        $comment['commit_id'] = $this->repoSha;
                        $comment['position'] = 1;

                        // @TODO: Allow tests.yml to define the path to post.
                        $comment['body'] = <<<BODY
<details>
    <summary>:x: Test Failed: <code>$test_name</code></summary>
    <pre>$command</pre>
   
```
{{output}}
```
    
- **On:** {$input->getOption('hostname')}
- **In:** {$process->duration}
    
</details>
BODY;
                        // Prevent exceeding of comment size.
                        $remaining_chars = self::GITHUB_COMMENT_MAX_SIZE - strlen($comment['body']);

                        // @TODO: Nooooo, getAllOutput()!
                        $process_output = $process->getOutput() . $process->getErrorOutput();

                        if (strlen($process_output) > $remaining_chars) {
                            $output = substr($process_output, 0, $remaining_chars) . "...";
                        } else {
                            $output = $process_output;
                        }

                        $comment['body'] = str_replace('{{output}}', self::stripAnsi(trim($output)), $comment['body']);

                        try {
                            // @TODO: If this branch is a PR, we will submit a Review or a PR comment. Neither work yet.
                            if (!empty($this->pullRequest)) {
                              // @TODO: This is NOT working. I can't get a PR Comment to submit.
                              // $comment['path'] = $input->getOption('tests-file');
//                              $comment_response = $client->pullRequest()->comments()->create($this->repoOwner, $this->repoName, $this->pullRequest['number'], $comment);

                                $comment_response = $client->repos()->comments()->create($this->repoOwner, $this->repoName, $this->repoSha, $comment);
                            } // If the branch is not yet a PR, we will just post a commit comment.
                            else {
                                $comment_response = $client->repos()->comments()->create($this->repoOwner, $this->repoName, $this->repoSha, $comment);
                            }

                            $this->successLite("Comment Created: {$comment_response['html_url']}");

                            // @TODO: Set Target URL from yaml-test options.
                            $params->target_url = $comment_response['html_url'];
                        } catch (\Github\Exception\RuntimeException $e) {
                            $this->errorLite("Unable to create GitHub Commit Comment: " . $e->getMessage() . ': ' . $e->getCode());
                        }
                    }
                }

                // If TRAVIS_JOB_WEB_URL is present and the target_url was not changed, use that as the target_url.
                if ($params->target_url == 'https:///' && !empty($_SERVER['TRAVIS_JOB_WEB_URL'])) {
                    $params->target_url = $_SERVER['TRAVIS_JOB_WEB_URL'];
                }

                if (!$input->getOption('dry-run')) {
                    $response = $client->getHttpClient()->post("/repos/$this->repoOwner/$this->repoName/statuses/$this->repoSha", [], json_encode($params));
                    $this->commitStatusMessage($response, $test_name, $test, $params->state);
                }

                $this->io->newLine();
                $rows[] = $results_row;
            }
        } catch (\Github\Exception\RuntimeException $e) {
            if ($output->isVerbose()) {
                $output->writeln($e->getTraceAsString());
            }
            if ($e->getCode() == 404) {
                throw new \Exception('Something went wrong: ' . $e->getMessage());
            } else {
                throw new \Exception("Bad token. Set with --github-token option or GITHUB_TOKEN environment variable. Create a new token at {$this->addTokenUrl} Message: " . $e->getMessage());
            }
        }


        $this->io->title("Executed all tests");
        $this->io->table(array('Test Results'), $rows);

        if ($tests_failed) {
            exit(1);
        }
    }

    private function loadTestsYml()
    {
        $this->yamlTests = Yaml::parse(file_get_contents($this->testsFilePath));
    }

    private function testsToTableRows()
    {
        $rows = array();
        foreach ($this->yamlTests as $test_name => $test) {
            if (is_array($test) && isset($test['command'])) {
                $command = $test['command'];
            } elseif (is_array($test)) {
                $command = implode("\n", $test);
            } else {
                $command = $test;
            }
            $rows[] = array($test_name, $command);
        }
        return $rows;
    }

    /**
     * Strips all ansi codes from a string. Used for posting plaintext github comments.
     *
     * @param $string
     *
     * @return string|string[]|null
     */
    protected function stripAnsi($string)
    {
        return preg_replace('#\\x1b[[][^A-Za-z]*[A-Za-z]#', '', $string);
    }

    protected function commitStatusMessage(Response $response, $test_name, $test, $state)
    {
        $message = implode(
            ': ',
            array(
            'GitHub Status',
            $test_name,
            $state
            )
        );

        if (strpos((string) $response->getStatusCode(), '2') === 0) {
            if ($state == 'pending') {
                $this->customLite($message, "⏺", "fg=yellow");
            } elseif ($state == 'error' || $state == 'failure') {
                $this->errorLite($message);
            } elseif ($state == 'success') {
                $this->successLite($message);
            }
        } else {
            // Big error for actual API error.
            $this->io->error($message);
        }
    }

    /**
     * Wrapper for $this->io->comment().
     *
     * @param $message
     */
    protected function say($message)
    {
        $this->io->text($message);
    }

    /**
     * Wrapper for $this->io->ask().
     *
     * @param $message
     */
    protected function ask($question)
    {
        return $this->io->ask($question);
    }

    /**
     * Wrapper for $this->io->ask().
     *
     * @param $message
     */
    protected function askDefault($question, $default)
    {
        return $this->io->ask($question, $default);
    }

    public function successLite($message, $newLine = false)
    {
        $message = sprintf('<info>✔</info> %s', $message);
        $this->io->text($message);
        if ($newLine) {
            $this->io->newLine();
        }
    }

    public function errorLite($message, $newLine = false)
    {
        $message = sprintf('<fg=red>✘</> %s', $message);
        $this->io->text($message);
        if ($newLine) {
            $this->io->newLine();
        }
    }

    public function warningLite($message, $newLine = false)
    {
        $message = sprintf('<comment>!</comment> %s', $message);
        $this->io->text($message);
        if ($newLine) {
            $this->io->newLine();
        }
    }

    public function customLite($message, $prefix = '*', $style = '', $newLine = false)
    {
        if ($style) {
            $message = sprintf(
                '<%s>%s</%s> %s',
                $style,
                $prefix,
                $style,
                $message
            );
        } else {
            $message = sprintf(
                '%s %s',
                $prefix,
                $message
            );
        }
        $this->io->text($message);
        if ($newLine) {
            $this->io->newLine();
        }
    }
}
