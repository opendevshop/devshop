<?php

namespace ProvisionOps\YamlTests;

use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Composer\Command\BaseCommand;
use Symfony\Component\Console\Style\SymfonyStyle;
use Symfony\Component\Finder\Finder;

/**
 * Class Command
 *
 * Provides the `git-build` command to composer.
 *
 * @package jonpugh\ComposerGitBuild
 */
class Command extends BaseCommand
{
    protected $createTag = FALSE;
    protected $tagName = NULL;
    protected $branchName;
    protected $commitMessage;
    protected $excludeFileTemp;
    
    /**
     * @var SymfonyStyle
     */
    protected $io;
    
    /**
     * The directory containing composer.json. Loaded from composer option --working-dir.
     * @var String
     */
    protected $workingDir;
    
    /**
     * The directory at the root of the git repository.
     * @var String
     */
    protected $gitDir;
    
    /**
     * The directory containing the git repository.
     * @var String
     */
    protected $buildDir;
    
    /**
     * The git reference of this project before any changes are made.
     * @var String
     */
    protected $initialGitRef;

    /**
     * The options from the project's composer.json "config" section.
     *
     * @var array
     */
    protected $config = [];

    /**
     * List of git remotes to push the artifact to.
     * @var string[]
     */
    protected $git_remotes = [];

    /**
     * Relative path to exposed document root.
     * @var string
     */
    protected $documentRoot;
    
    protected $ignoreDelimeter = "## IGNORED IN GIT BUILD ARTIFACTS: ##";
    
    protected function configure()
    {
        $this->setName('yaml-tests');
        $this->setDescription('Read tests.yml and runs all commands in it, passing results to GitHub Commit Status API.');

        $this->addOption(
            'ignore-dirty',
            NULL,
            InputOption::VALUE_NONE,
            'Allow testing even if git working copy is dirty (has modified files).'
        );
        $this->addOption(
            'dry-run',
            NULL,
            InputOption::VALUE_NONE,
            'Run tests but do not post to GitHub.'
        );
    }
    
    /**
     *
     */
    public function initialize(InputInterface $input, OutputInterface $output) {
    
        $this->io = new Style($input, $output);
        $this->input = $input;
        $this->output = $input;
        $this->logger = $this->io;

        $this->gitDir = $this->getGitDir();
        $this->workingDir = getcwd();

        $config_defaults = [
            'repo.root' => $this->gitDir,
        ];

        $this->config = array_merge($config_defaults, $this->getComposer()->getPackage()->getConfig());

        $this->io->title("Yaml Tests Initialized");
    }
    
    /**
     * @param \Symfony\Component\Console\Input\InputInterface $input
     * @param \Symfony\Component\Console\Output\OutputInterface $output
     */
    protected function execute(InputInterface $input, OutputInterface $output)
    {
        $options = $input->getOptions();
        
        if (!$this->isGitMinimumVersionSatisfied()) {
            $this->io->error("Your git is out of date. Please update git to 2.0 or newer.");
            exit(1);
        }
        
        if ($input->getOption('dry-run')) {
            $this->io->warning("This will be a dry run, the results will not be posted.");
        }
        $this->checkDirty($options);
        

        // Get and Check Current git reference.
        if ($this->getCurrentBranchName()) {
            $this->initialGitRef = $this->getCurrentBranchName();
            $this->say("Current git reference: <comment>{$this->initialGitRef}</comment>");
        }
        else {
            $this->io->error('No git reference detected in ' . $this->workingDir. '. You must have at least one commit. ');
            exit(1);
        }

        $this->say("Composer working directory: <comment>{$this->workingDir}</comment>");
        $this->io->title("Executed");

    }
    
    /**
     * Wrapper for $this->io->comment().
     * @param $message
     */
    protected function say($message) {
        $this->io->text($message);
    }
    
    /**
     * Wrapper for $this->io->ask().
     * @param $message
     */
    protected function ask($question) {
        return $this->io->ask($question);
    }
    
    /**
     * Wrapper for $this->io->ask().
     * @param $message
     */
    protected function askDefault($question, $default) {
        return $this->io->ask($question, $default);
    }
    
    /**
     * Gets the default branch name for the deployment artifact.
     */
    protected function getCurrentBranchName() {

        try{
            return $this->shell_exec("git rev-parse --abbrev-ref HEAD", $this->workingDir, false, true);
        } catch (\Exception $e) {
            throw new \ErrorException('No commits detected. You must have at least one commit.');
        }
    }

    /**
     * Gets the default remote URL from the source repo.
     */
    protected function getCurrentRemoteUrl() {
        return $this->shell_exec("git remote get-url origin", $this->workingDir, false, true);;
    }
    
    /**
     * Find the git repo directory that contains the workingDir.
     * @return string
     */
    private function getGitDir()
    {
        try{
            return $this->shell_exec('git rev-parse --show-toplevel', $this->workingDir, false, true);
        } catch (\ErrorException $e) {
            throw new \ErrorException('Working directory is not a git repository.');
        }
    }

    /**
     * Helper for running commands.
     *
     * @param $cmd
     * @param string $dir
     * @param bool $stop_on_fail
     * @return string
     * @throws \ErrorException
     */
    protected function shell_exec($cmd, $dir = '', $stop_on_fail = TRUE, $quiet = FALSE) {
        if ($dir && !file_exists($dir)) {
            throw new \Exception("Directory $dir does not exist in " . getcwd());
        }
        $oldWorkingDir = getcwd();
        chdir($dir? $dir: getcwd());

        if ($quiet) {
            $cmd .= "> /dev/null 2>&1";
        }
        exec($cmd, $output, $return);
        $output = trim(implode("\n", $output));
        chdir($oldWorkingDir);
        if ($return !== 0 && $stop_on_fail) {
            throw new \ErrorException("The command `$cmd` failed with exit code $return.", $return);
        }
        return $output;
    }
    
    /**
     * Verifies that installed minimum git version is met.
     *
     * @param string $minimum_version
     *   The minimum git version that is required.
     *
     * @return bool
     *   TRUE if minimum version is satisfied.
     */
    public function isGitMinimumVersionSatisfied($minimum_version = '2.0') {
        if (version_compare($this->shell_exec("git --version | cut -d' ' -f3"), $minimum_version, '>=')) {
            return TRUE;
        }
        return FALSE;
    }
    
    /**
     * Checks to see if current git branch has uncommitted changes.
     *
     * @throws \Exception
     *   Thrown if deploy.git.failOnDirty is TRUE and there are uncommitted
     *   changes.
     */
    protected function checkDirty($options) {
        exec('git status --porcelain', $result, $return);
        if (!$options['ignore-dirty'] && $return !== 0) {
            throw new \Exception("Unable to determine if local git repository is dirty.");
        }
        
        $dirty = (bool) $result;
        if ($dirty) {
            if ($options['ignore-dirty']) {
                $this->io->warning("There are uncommitted changes in your repository.");
            }
            else {
                throw new \Exception("There are uncommitted changes. Commit or stash these changes before running yaml-tests, or pass the option --ignore-dirty.");
            }
        }
    }
}