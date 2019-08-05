<?php

namespace ProvisionOps\YamlTests;

use ProvisionOps\Tools\PowerProcess as Process;
use ProvisionOps\Tools\Style;
use GuzzleHttp\Psr7\Response;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Composer\Command\BaseCommand;
use Symfony\Component\Yaml\Yaml;
use TQ\Git\Repository\Repository;

/**
 * Class Command
 *
 * Provides the `git-build` command to composer.
 *
 * @package jonpugh\ComposerGitBuild
 */
class Command extends BaseCommand
{
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
    protected $gitSha;

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
        $dotenv = \Dotenv\Dotenv::create(array(

          // Current user's home directory
          isset($_SERVER['HOME'])? $_SERVER['HOME']: '',

          // Git repo holding the tests file.
          dirname($this->gitRepo->getRepositoryPath()),

          // Current directory
          getcwd(),
        ));
        $dotenv->safeLoad();

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
        if (!empty($_SERVER['TRAVIS_PULL_REQUEST_SHA'])) {
            $this->repoSha = $_SERVER['TRAVIS_PULL_REQUEST_SHA'];
            $this->warningLite("Travis PR detected. Using PR SHA: " . $this->repoSha);
        }

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
        $this->repoOwner = $parts[1];
        $this->repoName = $parts[2];

        $this->io->title("Yaml Tests Initialized");

        // Force dry run if there is no token set.
        if (empty($token)) {
            $input->setOption('dry-run', true);
            $this->warningLite('No GitHub token found. forcing --dry-run');
            $this->io->writeln('');
        }

        $this->say("Git Remote: <comment>{$remote_url}</comment>");
        $this->say("Composer working directory: <comment>{$this->workingDir}</comment>");
        $this->say("Git Repository directory: <comment>{$this->workingDir}</comment>");
        $this->say("Git Commit: <comment>{$this->gitRepo->getCurrentCommit()}</comment>");
        $this->say("Tests File: <comment>{$this->testsFilePath}</comment>");

        if (!$input->getOption('dry-run')) {
            $this->githubClient = new \Github\Client();

            if ($input->getOption('ignore-ssl')) {
                $this->githubClient->getHttpClient()->client->setDefaultOption('verify', false);
            }

            $this->githubClient->authenticate($token, \Github\Client::AUTH_HTTP_TOKEN);
            $commit = $this->githubClient->repository()->commits()->show($this->repoOwner, $this->repoName, $this->repoSha);
            $this->say("GitHub Commit: <comment>" . $commit['html_url'] . "</>");
        }

        $this->io->table(array("Tests found in " . $this->testsFile), $this->testsToTableRows());
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
                        $comment['body'] = implode(
                            "\n",
                            array(
                                "###### :x: Test Failed: `$test_name`",
                                '  ```bash',
                                '  ' . $command,
                                '  ```',
                                '  ```bash',
                                '  ' . $process->getOutput(),
                                '  ' . $process->getErrorOutput(),
                                '  ```',
                                '*AUTOMATED COMMENT FROM provision-ops/yaml-tests, running on ' . $input->getOption('hostname') . '*',
                            )
                        );

                        try {
                            $comment_response = $client->repos()->comments()->create($this->repoOwner, $this->repoName, $this->repoSha, $comment);
                            $this->successLite("Comment Created: {$comment_response['html_url']}");
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
