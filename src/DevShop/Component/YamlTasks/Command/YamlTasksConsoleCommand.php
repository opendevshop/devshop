<?php

namespace DevShop\Component\YamlTasks\Command;

use Github\Exception\RuntimeException;
use DevShop\Component\PowerProcess\PowerProcess;
use DevShop\Component\PowerProcess\PowerProcessStyle;

use Guzzle\Http\Message\Response;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Command\Command as BaseCommand;
use Symfony\Component\Console\Style\SymfonyStyle;
use Symfony\Component\Yaml\Yaml;
use TQ\Git\Repository\Repository;

/**
 * Class Command
 */
class YamlTasksConsoleCommand extends BaseCommand
{
    const GITHUB_COMMENT_MAX_SIZE = 65536;
    const GITHUB_STATUS_DESCRIPTION_MAX_SIZE = 140;

    protected $createTag = false;
    protected $tagName = null;
    protected $branchName;
    protected $commitMessage;
    protected $excludeFileTemp;

    /**
     * @var PowerProcessStyle
     */
    protected $io;

    /**
     * @var InputInterface
     */
    protected $input;

    /**
     * @var OutputInterface
     */
    protected $output;

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

    private $addTokenUrl = "https://github.com/settings/tokens/new?description=yaml-tasks&scopes=repo:status,public_repo";

    protected function configure()
    {
        $this->setName('yaml-tasks');
        $this->setDescription('Read tasks,yml and runs all commands in it, passing results to GitHub Commit Status API.');

        $this->addOption(
            'tasks-file',
            null,
            InputOption::VALUE_OPTIONAL,
            'Relative path to a yml file to run.',
            'tasks.yml'
        );
        $this->addOption(
            'github-token',
            null,
            InputOption::VALUE_REQUIRED,
            'An active github token. Create a new token at ' . $this->addTokenUrl,
            !empty($_SERVER['GITHUB_TOKEN'])? $_SERVER['GITHUB_TOKEN']: null
        );
        $this->addOption(
            'ignore-dirty',
            null,
            InputOption::VALUE_NONE,
            'Allow running tasks even if git working copy is dirty (has modified files).'
        );
        $this->addOption(
            'dry-run',
            null,
            InputOption::VALUE_NONE,
            'Run tasks but do not post to GitHub.'
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
        $this->addOption(
            'status-url',
            null,
            InputOption::VALUE_OPTIONAL,
            'The url used for the "Details" link on GitHub.com.',
            !empty($_SERVER['STATUS_URL'])? $_SERVER['STATUS_URL']: null
        );
        $this->addArgument(
            'filter',
            InputArgument::IS_ARRAY,
            'A list of strings to filter tasks by.'
        );
    }

    /**
     *
     */
    public function initialize(InputInterface $input, OutputInterface $output)
    {

        $this->io = new PowerProcessStyle($input, $output);
        $this->input = $input;
        $this->output = $output;
        $this->logger = $this->io;
        $this->workingDir = getcwd();

        $this->gitRepo = Repository::open($this->workingDir);
        $this->gitRepo->getCurrentCommit();

        $composer_json = $this->workingDir . '/composer.json';
        if (!is_readable($composer_json)) {
            throw new \Exception("Unable to read composer data from $composer_json");
        }

        $this->config = json_decode(file_get_contents($this->workingDir . '/composer.json'));

        $this->tasksFile = $input->getOption('tasks-file');
        $this->tasksFilePath = realpath($this->tasksFile);
        if (!file_exists($this->tasksFilePath) || empty($this->tasksFilePath)) {
            throw new \Exception("Specified tasks file does not exist at {$this->workingDir}/{$this->tasksFile}");
        }

        // Validate YML
        $this->loadTasksYml();

        // Load token.
        $token = $input->getOption('github-token');

        // This is the actual SHA of the working copy clone.
        $this->repoSha = $this->gitRepo->getCurrentCommit();

        // Detect a TRAVIS_PULL_REQUEST_SHA
        // Travis runs from a commit created from master and our commit.
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

        $this->io->title("Yaml Tasks Initialized");

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
        $this->say("Tasks File: <comment>{$this->tasksFilePath}</comment>");

        if ($this->getTargetUrl()) {
            $this->say("Target URL: <comment>{$this->getTargetUrl()}</comment>");
        }

        // Lookup composer bin path and add to PATH if it is not there already.
        $composer_bin_path = $this->workingDir . '/' . (!empty($this->config->config->{"bin-dir"})? $this->config->config->{"bin-dir"}: 'vendor/bin');
        $this->say("Composer Bin Path: <comment>$composer_bin_path</comment>");
        if (is_readable($composer_bin_path) && strpos($_SERVER['PATH'], $composer_bin_path) === false) {
            $this->warningLite("Composer Bin Path was not found in existing PATH variable, so it was added.");
            $_SERVER['PATH'] .= ':' . $composer_bin_path;
        }

        // If debug flag was used, print the PATH.
        if ($this->io->isDebug()) {
            $this->say("Path: <comment>{$_SERVER['PATH']}</comment>");
        }

        // @TODO: Dry run could still read info from the repo.
        if (!$input->getOption('dry-run')) {
            $this->githubClient = new \Github\Client();

            if ($input->getOption('ignore-ssl') || getenv('DEVSHOP_GITHUB_API_IGNORE_SSL')) {
                $this->githubClient->getHttpClient()->client->setDefaultOption('verify', false);
            }

            $this->githubClient->authenticate($token, \Github\Client::AUTH_HTTP_TOKEN);

            // Load the commit object. Catch an exception, and change the message. Our users will wonder, "but there is a commit!"
            try {
                $commit = $this->githubClient->repository()->commits()->show($this->repoOwner, $this->repoName, $this->repoSha);
            } catch (RuntimeException $exception) {
                // @TODO This can fail because the commits don't exist, or from an SSL cert problem. Change the message for each.
                throw new RuntimeException("Commit not found in the remote repository. YamlTasks cannot post commit status until the commits are pushed to the remote repository. The message was: " . $exception->getMessage());
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
                $this->warningLite("No pull requests were found using the current local branch <comment>{$this->gitRepo->getCurrentBranch()}</comment>. Make sure a Pull Request has been created in addition to the branch being pushed. Errors will be sent as comments on the Commit, instead of on the Pull Request. This means error logs will appear on any Pull Request that contains the commit being tasked.");
            } else {
                $this->pullRequest = $prs[0];
            }
        }

        $this->io->table(array("Tasks found in " . $this->tasksFile), $this->tasksToTableRows());

        // If there are filters, shorten the list of tasks to run.
        $filters = $input->getArgument('filter');
        $filter_string = implode(' ', $filters);
        if (count($filters)) {
            foreach ($this->yamlTasks as $name => $task) {
                $run_the_task = false;
                foreach ($filters as $filter) {
                    // If the filter string was found in the task name, run the task.
                    if (strpos($name, $filter) !== false) {
                        $run_the_task = true;
                    }
                }

                if (!$run_the_task) {
                    unset($this->yamlTasks[$name]);
                }
            }
        }

        // If there are no matches
        if (count($filters) && count($this->yamlTasks) > 0) {
            $this->io->table(array("Tasks to run based on filter '$filter_string'"), $this->tasksToTableRows());
        } elseif (count($filters)) {
            // If there are filters but tasks were NOT removed, show a warning.
            $this->warningLite("The filter '$filter_string' was specified but it did not match any tasks.");
            exit(1);
        }
    }

    /**
     * @param \Symfony\Component\Console\Input\InputInterface   $input
     * @param \Symfony\Component\Console\Output\OutputInterface $output
     */
    protected function execute(InputInterface $input, OutputInterface $output)
    {
        $tasks_failed = false;

        try {
            if (!$input->getOption('dry-run')) {
                $client = $this->githubClient;

                foreach ($this->yamlTasks as $task_name => $task) {
                    // Set a commit status for this REF
                    $params = new \stdClass();

                    // Reserve "Pending" for the earliest possible commit status update (a curl request at the beginning.)
                    // Use "queued" once it is in the task system.
                    $params->state = 'queued';
                    $params->target_url = $this->getTargetUrl($task_name);
                    $params->description = implode(
                        ' — ',
                        array(
                            !empty($task['description'])? $task['description']: $task_name,
                            $input->getOption('hostname'),
                        )
                    );
                    $params->context = $task_name;

                    if (strlen($params->description) > self::GITHUB_STATUS_DESCRIPTION_MAX_SIZE) {
                        $params->description = substr($params->description, 0, self::GITHUB_STATUS_DESCRIPTION_MAX_SIZE - 1) . '…';
                    }

                    // Post status to github
                    try {
                        /**
                         * @var Response $response
                         */
                        $response = $client->getHttpClient()->post("/repos/{$this->repoOwner}/{$this->repoName}/statuses/$this->repoSha", json_encode($params));
                        $this->commitStatusMessage($response, $task_name, $task, $params->state);
                    } catch (\Exception $e) {
                        if ($e->getCode() == 404) {
                            throw new \Exception('Unable to reach commit status API. Check the allowed scopes of your GitHub Token. Skip github interaction with --dry-run, or create a new token with the right scopes at ' . $this->addTokenUrl);
                        }
                    }
                    $tasks[] = $task_name;
                }
            } else {
                $this->warningLite('Skipping commit status posting, dry-run enabled.');
            }

            $this->io->newLine();
            $rows = array();

            foreach ($this->yamlTasks as $task_name => $task) {
                $command = implode(" && ", $task['command']);
                $command_view = implode("\n", $task['command']);

                $results_row = array(
                    $task_name,
                    $command_view,
                );

                $process = new PowerProcess($command, $this->io);
                $process->setTimeout(null);
                $process->setIo($this->io);

                // Set some environment variables to indicate YAML_TASKS is running.
                $env = $_SERVER;
                $env['YAML_TASKS'] = 1;
                $env['YAML_TASKS_NAME'] = $task_name;
                $env['YAML_TASKS_COMMAND'] = $command;
                $env['YAML_TASKS_DESCRIPTION'] = $task['description'];

                $process->setEnv($env);

                // If there is a target URL, print it.
                if ($this->getTargetUrl()) {
                    $title = "Running task <fg=white>$task_name</>  -  {$this->getTargetUrl($task_name)}";
                } else {
                    $title = "Running task <fg=white>$task_name</>";
                }

                $this->io->section($title);

                if ($task['description']) {
                    $this->io->text($task['description']);
                }

                if ($task['show-output'] == false) {
                    $process->disableOutput();
                }

                // Set a commit status for this REF
                $params = new \stdClass();
                $params->state = 'in_progress';
                $params->target_url = $this->getTargetUrl($task_name);
                $params->context = $task_name;

                $exit = $process->run();

                // Set a commit status for this REF
                $params = new \stdClass();
                $params->target_url = $this->getTargetUrl($task_name);
                $params->context = $task_name;

                if ($exit == 0) {
                    $results_row[] = '<info>✔</info> Passed';
                    $params->state = 'success';
                    $params->description = implode(
                        ' — ',
                        array(
                            "Successful in {$process->duration} on {$input->getOption('hostname')}",
                        )
                    );
                } else {
                    // If the task has the ignore failure flag, ignore it.
                    if (!empty($task['ignore-failure'])) {
                        $results_row[] = '<fg=red>✘</> Failed (Ignoring)';
                        $params->state = 'success';
                        $params->description = implode(
                            ' — ',
                            array(
                                "Failed after {$process->duration} on {$input->getOption('hostname')} (ignored).",
                                !empty($task['description'])? $task['description']: $task_name,
                            )
                        );
                    } else {
                        $results_row[] = '<fg=red>✘</> Failed';
                        $tasks_failed = true;
                        $params->state = 'failure';
                        $params->description = implode(
                            ' — ',
                            array(
                                "Failed after {$process->duration} on {$input->getOption('hostname')}",
                                !empty($task['description'])? $task['description']: $task_name,
                            )
                        );
                    }

                    if (!$input->getOption('dry-run')) {
                        // @TODO: Make the commenting optional/configurable
                        // Write a comment on the commit with the results
                        // @see https://developer.github.com/v3/repos/comments/#create-a-commit-comment

                        $comment = array();
                        $comment['commit_id'] = $this->repoSha;
                        $comment['position'] = 1;

                        // @TODO: Allow tasks,yml to define the path to post.
                        $comment['body'] = <<<BODY
<details>
    <summary>:x: Task Failed: <code>$task_name</code></summary>
    <pre>$command</pre>
   
```
{{output}}
```
    
- **On:** {$input->getOption('hostname')}
- **In:** {$process->duration}
    
</details>
BODY;
                        // Prevent exceeding of comment size by truncating.
                        $comment_template_length = strlen($comment['body']) - 10;
                        $truncate_message =  "... *(truncated)*";
                        $truncate_message_length = strlen($truncate_message);

                        $remaining_chars = self::GITHUB_COMMENT_MAX_SIZE - ($comment_template_length + $truncate_message_length);

                        // @TODO: Nooooo, getAllOutput()!
                        if ($process->isOutputDisabled()) {
                            $process_output = "OUTPUT HIDDEN";
                        } else {
                            $process_output = $process->getOutput() . $process->getErrorOutput();
                        }

                        if (strlen($process_output) > $remaining_chars) {
                            $output = substr($process_output, 0, $remaining_chars) . $truncate_message;
                        } else {
                            $output = $process_output;
                        }

                        $comment['body'] = str_replace('{{output}}', self::stripAnsi(trim($output)), $comment['body']);

                        // Catch ourselves if our math is wrong.
                        if (strlen($comment['body']) > self::GITHUB_COMMENT_MAX_SIZE) {
                            throw new \Exception('Comment body is STILL too long... the math must be wrong.');
                        }

                        // Only post github comment if "post-errors-as-comments" is set.
                        if ($task['post-errors-as-comments']) {
                            try {
                                // @TODO: If this branch is a PR, we will submit a Review or a PR comment. Neither work yet.
                                if (!empty($this->pullRequest)) {
                                    // @TODO: This is NOT working. I can't get a PR Comment to submit.
                                    // $comment['path'] = $input->getOption('tasks-file');
                                    //                              $comment_response = $client->pullRequest()->comments()->create($this->repoOwner, $this->repoName, $this->pullRequest['number'], $comment);

                                    $comment_response = $client->repos()->comments()->create($this->repoOwner, $this->repoName, $this->repoSha, $comment);

                                // If the branch is not yet a PR, we will just post a commit comment.
                                } else {
                                    $comment_response = $client->repos()->comments()->create($this->repoOwner, $this->repoName, $this->repoSha, $comment);
                                }

                                $this->successLite("Comment Created: {$comment_response['html_url']}");

                                // GitHub Actions, we want to test posting a comment but we don't want to see the comment.
                                if (getenv('GITHUB_ACTIONS')) {
                                    $client->repos()->comments()->remove($this->repoOwner, $this->repoName, $comment_response['id']);
                                }

                                // @TODO: Set Target URL from yaml-task options.
                                // $params->target_url = $this->getTargetUrl($comment_response['html_url']);
                                // Always use the main target url... If this is overridable, it should be configurable by the user in their tasks,yml.
                                $params->target_url = $this->getTargetUrl($task_name);
                            } catch (\Github\Exception\RuntimeException $e) {
                                $this->errorLite("Unable to create GitHub Commit Comment: " . $e->getMessage() . ': ' . $e->getCode());
                            }
                        }
                    }
                }

                if ($task['show-output'] == false) {
                    $this->warningLite("Output was hidden, as configured in " . $this->tasksFile);
                }

                if (!$input->getOption('dry-run')) {
                    $params->description = substr($params->description, 0, self::GITHUB_STATUS_DESCRIPTION_MAX_SIZE);
                    $response = $client->getHttpClient()->post("/repos/$this->repoOwner/$this->repoName/statuses/$this->repoSha", json_encode($params));
                    $this->commitStatusMessage($response, $task_name, $task, $params->state);
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


        $this->io->title("Executed all tasks");
        $this->io->table(array('Task Results'), $rows);

        if ($tasks_failed) {
            exit(1);
        }
    }

    private function loadTasksYml()
    {
        $this->yamlTasks = Yaml::parse(file_get_contents($this->tasksFilePath));

        // Set Defaults
        foreach ($this->yamlTasks as $name => $task) {
            $commands = array();

            // task is a string
            if (is_string($task)) {
                $commands[] = $task;
                $task = array(
                    'command' => $commands
                );

            // task.command is a string
            } elseif (is_array($task) && isset($task['command']) && is_string($task['command'])) {
                $commands[] = $task['command'];

            // task is an array of commands
            } elseif (!isset($task['command']) && is_array($task)) {
                $commands += $task;

            // task.command is an array
            } elseif (is_array($task) && is_array($task['command'])) {
                $commands += $task['command'];
            }

            $task['command'] = $commands;
            $task['description'] = isset($task['description'])? $task['description']: null;
            $task['post-errors-as-comments'] = isset($task['post-errors-as-comments'])? $task['post-errors-as-comments']: false;
            $task['show-output'] = isset($task['show-output'])? $task['show-output']: true;

            $this->yamlTasks[$name] = $task;
        }
    }

    private function tasksToTableRows()
    {
        $rows = array();
        foreach ($this->yamlTasks as $task_name => $task) {
            $rows[] = array($task_name,  implode("\n", $task['command']));
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

    protected function commitStatusMessage(Response $response, $task_name, $task, $state)
    {
        $message = implode(
            ': ',
            array(
                'GitHub Status',
                $task_name,
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

    /**
     * Return the target URL used in the GitHub "Details" link, using either param, command line option, or the ENV var.
     */
    protected function getTargetUrl($anchor = null)
    {
        // Return the alternate URL if it is present. If not, the command line option. (which defaults to the ENV var.)
        $url = $this->input->getOption('status-url') . '#' . $anchor;

        // Switch link to use HTTPS, it is required by GitHub API.
        return empty($url)? null: str_replace('http://', 'https://', $url);
    }
}
