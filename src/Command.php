<?php

namespace jonpugh\ComposerGitBuild;

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
        $this->setName('git-build');
        $this->setDescription('Add all vendor code and ignored dependencies to git.');
        
        $this->addOption(
            'build-dir',
            'b',
            InputOption::VALUE_OPTIONAL,
            'Directory to create the git artifact. Defaults to the composer working-dir option.',
            'build'
        );
        $this->addOption(
            'branch',
            NULL,
            InputOption::VALUE_REQUIRED,
            'Branch to create.'
        );
        $this->addOption(
            'document_root',
            NULL,
            InputOption::VALUE_OPTIONAL,
            'Relative path to exposed document root.'
        );
        $this->addOption(
            'tag',
            NULL,
            InputOption::VALUE_REQUIRED,
            'Tag to create.'
        );
        $this->addOption(
            'commit-msg',
            'm',
            InputOption::VALUE_REQUIRED,
            'Commit message to use.'
        );
        $this->addOption(
            'ignore-dirty',
            NULL,
            InputOption::VALUE_NONE,
            'Allow committing even if git working copy is dirty (has modified files).'
        );
        $this->addOption(
            'dry-run',
            NULL,
            InputOption::VALUE_NONE,
            'Build and commit to the repository but do not push.'
        );
    }
    
    /**
     * Set workingDir and buildDir.
     * buildDir is workingDir in our tool. We are trying to avoid creating a separate deploy dir.
     */
    public function initialize(InputInterface $input, OutputInterface $output) {
    
        $this->io = new Style($input, $output);
        $this->input = $input;
        $this->output = $input;
        $this->logger = $this->io;
    
        // Using $input->getOption() does not work here because working-dir option is not set if you are already in the dir.
        // Composer always sets the cwd to the working-dir option.
        $this->workingDir = getcwd();
        $this->gitDir = $this->getGitDir();
        
        $config_defaults = [
            'repo.root' => $this->gitDir,
        ];

        $this->config = array_merge($config_defaults, $this->getComposer()->getPackage()->getConfig());

        $this->io->title("Preparing for Build");
        $this->buildDir = $this->getBuildDir();

        // Add remotes and fetch upstream refs.
        if (!isset($this->config['git']) || empty($this->config['git']['remotes'])) {

          $this->io->comment('No git remotes found. You can define the remote repository to push to in composer.json:
  {
      "config": {
          "git": {
              "remotes": [
                "git@github.com:org/repo.git"
              ]
          }
      }
  }
  ');
          $this->git_remotes[] = $this->askDefault('Remote Artifact Git repository', $this->getCurrentRemoteUrl());
        }
        else {
          $this->git_remotes = $this->config['git']['remotes'];
        }

        if (empty($options['document_root'])) {
            $this->documentRoot = $this->ask('Document root');
        }
        else {
            $this->documentRoot = $options['document_root'];
        }
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
            $this->io->warning("This will be a dry run, the artifact will not be pushed.");
        }
        $this->checkDirty($options);
        
        $this->say("Composer working directory: <comment>{$this->workingDir}</comment>");

        // Get and Check Current git reference.
        if ($this->getCurrentBranchName()) {
            $this->initialGitRef = $this->getCurrentBranchName();
            $this->say("Current git reference: <comment>{$this->initialGitRef}</comment>");
        }
        else {
            $this->io->error('No git reference detected in ' . $this->workingDir);
            exit(1);
        }
        
        if (realpath($this->buildDir) == $this->gitDir) {
            $this->buildDir = realpath($this->buildDir);
            $this->say("Build Directory: <comment>$this->buildDir</comment> <warning>Building in place. Data will be altered.</warning>");
        }
        elseif (file_exists($this->buildDir)) {
            $this->say("Build Directory: <comment>$this->buildDir</comment> <error>Directory already exists. Data will be destroyed!</error>");
    
        }
        else {
            $this->say("Build Directory: <comment>$this->buildDir</comment>");
    
        }

        $documentRoot = $this->documentRoot? $this->documentRoot: 'Repository Root';
        $this->say("Document Root: <comment>$documentRoot</comment>");

        $this->say("Artifact Remote Repositories:");
        foreach ($this->git_remotes as $remote) {
            if ($remote == $this->getCurrentRemoteUrl()) {
                $this->say("<comment>$remote</comment> <error>Same as artifact remote.</error>");
            }
            else {
                $this->say("<comment>$remote</comment>");
            }
        }

        if (!$options['tag'] && !$options['branch']) {
//            $this->say("Typically, you would only create a tag if you currently have a tag checked out on your source repository.");
            $this->createTag = $this->io->confirm("Would you like to create a tag?", $this->createTag);
        }
        $this->commitMessage = $this->getCommitMessage($options);
        
        if ($options['tag'] || $this->createTag) {
            $this->deployToTag($options);
        }
        else {
            $this->deployToBranch($options);
        }
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
        return $this->shell_exec("git rev-parse --abbrev-ref HEAD", $this->workingDir);
    }

    /**
     * Gets the default remote URL from the source repo.
     */
    protected function getCurrentRemoteUrl() {
        return $this->shell_exec("git remote get-url origin", $this->workingDir);
    }
    
    //    /**
    //     * Gets the default branch name for the deployment artifact.
    //     */
    //    protected function getDefaultBranchName() {
    //        $default_branch = $this->getCurrentBranchName() . '-build';
    //        return $default_branch;
    //    }
    
    /**
     * Find the git repo directory that contains the workingDir.
     * @return string
     */
    private function getGitDir()
    {
        return $this->shell_exec('git rev-parse --show-toplevel', $this->workingDir);
    }
    
    protected function shell_exec($cmd, $dir = '', $stop_on_fail = TRUE) {
        if ($dir && !file_exists($dir)) {
            throw new \Exception("Directory $dir does not exist in " . getcwd());
        }
        $oldWorkingDir = getcwd();
        chdir($dir? $dir: getcwd());
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
                $this->io->warning("There are uncommitted changes on the source repository.");
            }
            else {
                throw new \Exception("There are uncommitted changes, commit or stash these changes before running git-build.");
            }
        }
    }
    
    /**
     * Gets the commit message to be used for committing deployment artifact.
     *
     * Defaults to the last commit message on the source branch.
     *
     * @return string
     *   The commit message.
     */
    protected function getCommitMessage($options) {
        if (!$options['commit-msg']) {
            //        chdir($this->getConfigValue('repo.root'));
            $log = explode(' ', shell_exec("git log --oneline -1"), 2);
            $git_last_commit_message = trim($log[1]);
            
            return $this->io->ask('Enter a valid commit message', $git_last_commit_message);
        }
        else {
            $this->io->comment("Commit message is set to <comment>{$options['commit-msg']}</comment>.");
            return $options['commit-msg'];
        }
    }
    
    /**
     * Gets the build dir either from an option or from asking the user.
     *
     * Defaults to the current workingDir.
     *
     * @return string
     *   The branch name.
     */
    protected function getBuildDir() {
        $options = $this->input->getOptions();
        if (!$options['build-dir']) {
            $options['build-dir'] = $this->askDefault('Enter the directory to build the git repo in', $this->workingDir);
        }
        else {
            $options['build-dir'] = $this->askDefault('Enter the directory to build the git repo in', $options['build-dir']);
        }
        
        // If not an absolute path, append cwd.
        if (strpos($options['build-dir'], '/') !== 0) {
            $options['build-dir'] = getcwd() . '/' . $options['build-dir'];
        }
    
        $this->input->setOption('build-dir', $options['build-dir']);
        return $options['build-dir'];
    }
    
    /**
     * Gets the branch name for the deployment artifact.
     *
     * Defaults to [current-branch]-build.
     *
     * @return string
     *   The branch name.
     */
    protected function getBranchName($options) {
        if ($options['branch']) {
            $this->say("Branch is set to <comment>{$options['branch']}</comment>.");
            return $options['branch'];
        }
        else {
            return $this->askDefault('Enter the branch name for the deployment artifact', $this->getDefaultBranchName());
        }
    }
    
    /**
     * Gets the name of the tag to cut.
     *
     * @param $options
     *
     * @return string
     *
     * @throws \Exception
     */
    protected function getTagName($options) {
        if ($options['tag']) {
            $tag_name = $options['tag'];
        }
        else {
            $tag_name = $this->ask('Enter the tag name for the deployment artifact. E.g., 1.0.0-build');
        }
        
        if (empty($tag_name)) {
            // @todo Validate tag name is valid. E.g., no spaces or special characters.
            throw new \Exception("You must enter a valid tag name.");
        }
        else {
            $this->say("Tag is set to <comment>$tag_name</comment>.");
        }
        
        return $tag_name;
    }
    
    /**
     * Gets the default branch name for the deployment artifact.
     */
    protected function getDefaultBranchName() {
        chdir($this->getConfigValue('repo.root'));
        $git_current_branch = trim(shell_exec("git rev-parse --abbrev-ref HEAD"));
        $default_branch = $git_current_branch . '-build';
        
        return $default_branch;
    }
    
    /**
     * Creates artifact, cuts new tag, and pushes.
     */
    protected function deployToTag($options) {
        $this->tagName = $this->getTagName($options);
        
        // If we are building a tag, then we assume that we will NOT be pushing the
        // build branch from which the tag is created. However, we must still have a
        // local branch from which to cut the tag, so we create a temporary one.
        $this->branchName = $this->getDefaultBranchName() . '-temp';
        $this->prepareDir();
        $this->addGitRemotes();
        $this->checkoutLocalDeployBranch();
        
        // Added this to deployTag, because we're not running buildCopy anymore.
        $this->mergeUpstreamChanges();
        $this->build();
        $this->commit();
        $this->cutTag();
        $this->push($this->tagName, $options);
        $this->cleanup();
    }
    
    /**
     * Creates artifact on branch and pushes.
     */
    protected function deployToBranch($options) {
        $this->branchName = $this->getBranchName($options);
        $this->prepareDir();
        $this->addGitRemotes();
        $this->checkoutLocalDeployBranch();
        $this->mergeUpstreamChanges();
        $this->build();
        $this->commit();
        $this->push($this->branchName, $options);
    }
    
    /**
     * Deletes the existing deploy directory and initializes git repo.
     */
    protected function prepareDir() {
        $this->say("Preparing artifact directory...");
        if ($this->buildDir != $this->workingDir) {
            $this->shell_exec("rm -rf $this->buildDir");
            $this->shell_exec("mkdir $this->buildDir");
            $this->shell_exec("git init", $this->buildDir);
        }
    
            //              $this->taskExecStack()
//                ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE)
//                ->stopOnFail()
//                ->dir($this->buildDir)
//                ->exec("git config --local --add user.name '$git_user'")
//                ->exec("git config --local --add user.email '$git_email'")
//                ->run();
//            }
        //            $deploy_dir = $this->buildDir;
//            $this->taskDeleteDir($deploy_dir)
//              ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE)
//              ->run();
//            $this->taskFilesystemStack()
//              ->mkdir($this->buildDir)
//              ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE)
//              ->stopOnFail()
//              ->run();
//            $this->taskExecStack()
//              ->dir($deploy_dir)
//              ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE)
//              ->stopOnFail()
//              ->exec("git init")
//              ->exec("git config --local core.excludesfile false")
//              ->run();
//            $this->say("Global .gitignore file is being disabled for this repository to prevent unexpected behavior.");
//            if ($this->getConfig()->has("git.user.name") &&
//              $this->getConfig()->has("git.user.email")) {
//              $git_user = $this->getConfigValue("git.user.name");
//              $git_email = $this->getConfigValue("git.user.email");
//              $this->taskExecStack()
//                ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE)
//                ->stopOnFail()
//                ->dir($this->buildDir)
//                ->exec("git config --local --add user.name '$git_user'")
//                ->exec("git config --local --add user.email '$git_email'")
//                ->run();
//            }
    }
    
    /**
     * Adds remotes from git.remotes to /deploy repository.
     */
    protected function addGitRemotes() {
//        // Add remotes and fetch upstream refs.
//        $git_remotes = $this->getConfigValue('git.remotes');
//        if (empty($git_remotes)) {
//            throw new \Exception("git.remotes is empty. Please define at least one value for git.remotes in composer.json 'config' section.");
//        }
        foreach ($this->git_remotes as $remote_url) {
            $this->addGitRemote($remote_url);
        }
    }
    
    /**
     * Adds a single remote to the /deploy repository.
     */
    protected function addGitRemote($remote_url) {
        $this->say("Fetching from git remote $remote_url");
        
        // Generate an md5 sum of the remote URL to use as remote name.
        $remote_name = md5($remote_url);
        
        // Remote may already exist.
        try {
            $this->shell_exec("git remote add $remote_name $remote_url", $this->buildDir);
        }
        catch (\ErrorException $e) {
            if ($e->getCode() == 128) {
                $this->io->warning("Remote already exists.");
            }
            else {
                throw new \ErrorException($e->getMessage());
            }
        }
        
        //    $this->taskExecStack()
        //      ->stopOnFail()
        //      ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE)
        //      ->dir($this->buildDir)
        //      ->exec("git remote add $remote_name $remote_url")
        //      ->run();
        
    }
    
    /**
     * Checks out a new, local branch for artifact.
     */
    protected function checkoutLocalDeployBranch() {
        //    $this->taskExecStack()g
        //      ->dir($this->buildDir)
        //      // Create new branch locally.We intentionally use stopOnFail(FALSE) in
        //      // case the branch already exists. `git checkout -B` does not seem to work
        //      // as advertised.
        //      // @todo perform this in a way that avoid errors completely.
        //      ->stopOnFail(FALSE)
        //      ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE)
        //      ->exec("git checkout -b {$this->branchName}")
        //      ->run();
        
        $this->shell_exec("git checkout -b {$this->branchName}", $this->buildDir);
    }
    
    /**
     * Merges upstream changes into deploy branch.
     */
    protected function mergeUpstreamChanges() {
        $git_remotes = $this->git_remotes;
        $remote_url = reset($git_remotes);
        $remote_name = md5($remote_url);
        
        $this->say("Merging upstream changes into local artifact...");
        $this->shell_exec("git fetch $remote_name {$this->initialGitRef}", $this->buildDir, FALSE);
        $this->shell_exec("git merge $remote_name/{$this->initialGitRef}", $this->buildDir, FALSE);
        
        //    $this->taskExecStack()
        //      ->dir($this->buildDir)
        //      // This branch may not exist upstream, so we do not fail the build if a
        //      // merge fails.
        //      ->stopOnFail(FALSE)
        //      ->exec("git fetch $remote_name {$this->branchName}")
        //      ->exec("git merge $remote_name/{$this->branchName}")
        //      ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE)
        //      ->run();
    }
    
    /**
     * Builds deployment artifact.
     *
     * @command deploy:build
     */
    public function build() {
    
        $this->say("Altering .gitignore...");
    
        if (strpos(file_get_contents($this->buildDir . '/.gitignore'), $this->ignoreDelimeter) === FALSE) {
            $this->say("The git build ignore delimiter was not found in your .gitignore file. All entries will be removed from your .gitignore file. Add '{$this->ignoreDelimeter} to save entries to .gitignore in the build.");
        }
    
        $this->shell_exec("sed -i '1,/{$this->ignoreDelimeter}/d' .gitignore", $this->buildDir);
    
        if ($this->getConfigValue("git.user.name") &&
            $this->getConfigValue("git.user.email")) {
            $git_user = $this->getConfigValue("git.user.name");
            $git_email = $this->getConfigValue("git.user.email");
            $this->shell_exec("git config --local --add user.name '$git_user'", $this->buildDir);
            $this->shell_exec(
                "git config --local --add user.email '$git_email'"
                , $this->buildDir);
        }
        
        $this->say("Generating build artifact...");
        $this->say("For more detailed output, use the -v flag.");
        //    $this->invokeCommands([
        //      // Execute `blt frontend` to ensure that frontend artifact are generated
        //      // in source repo.
        //      'frontend',
        //      // Execute `setup:hash-salt` to ensure that salt.txt exists. There's a
        //      // slim chance this has never been generated.
        //      'setup:hash-salt',
        //    ]);
        
        //    $this->buildCopy();
        $this->composerInstall();
        $this->sanitize();
        //    $this->deploySamlConfig();
        if (!empty($this->tagName)) {
            $this->createDeployId($this->tagName);
        }
        else {
            $this->createDeployId(uniqid());
        }
        //    $this->invokeHook("post-deploy-build");
        $this->say("<info>The deployment artifact was generated at {$this->buildDir}.</info>");
        
    }
    
// @TODO: Figure out why this is needed. We use git pull and alter .gitignore to get all files into buildDir().
//    /**
//     * Copies files from source repo into artifact.
//     */
//    protected function buildCopy() {
//
//        if (!$this->getConfigValue('deploy.build-dependencies')) {
//            $this->logger->warning("Dependencies will not be built because deploy.build-dependencies is not enabled");
//            $this->logger->warning("You should define a custom deploy.exclude_file to ensure that dependencies are copied from the root repository.");
//
//            return FALSE;
//        }
//
//        $exclude_list_file = $this->getExcludeListFile();
//        $source = $this->getConfigValue('repo.root');
//        $dest = $this->buildDir;
//
//        $this->setMultisiteFilePermissions(0777);
//        $this->say("Rsyncing files from source repo into the build artifact...");
//        $this->taskExecStack()->exec("rsync -a --no-g --delete --delete-excluded --exclude-from='$exclude_list_file' '$source/' '$dest/' --filter 'protect /.git/'")
//            ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE)
//            ->stopOnFail()
//            ->dir($this->getConfigValue('repo.root'))
//            ->run();
//        $this->setMultisiteFilePermissions(0755);
//
//        // Remove temporary file that may have been created by
//        // $this->getExcludeListFile().
//        $this->taskFilesystemStack()
//            ->remove($this->excludeFileTemp)
//            ->copy(
//                $this->getConfigValue('deploy.gitignore_file'),
//                $this->buildDir . '/.gitignore', TRUE
//            )
//            ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE)
//            ->run();
//
//    }
    
    /**
     * Installs composer dependencies for artifact.
     */
    protected function composerInstall() {
        $this->say("Rebuilding composer dependencies for production...");
        $this->shell_exec("composer install --no-dev --no-interaction --optimize-autoloader", $this->buildDir);
        //    $this->taskDeleteDir([$this->buildDir . '/vendor'])
        //      ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE)
        //      ->run();
        //    $this->taskFilesystemStack()
        //      ->copy($this->getConfigValue('repo.root') . '/composer.json', $this->buildDir . '/composer.json', TRUE)
        //      ->copy($this->getConfigValue('repo.root') . '/composer.lock', $this->buildDir . '/composer.lock', TRUE)
        //      ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE)
        //      ->run();
        //    $this->taskExecStack()->exec("composer install --no-dev --no-interaction --optimize-autoloader")
        //      ->stopOnFail()
        //      ->dir($this->buildDir)
        //      ->run();
    }
    
    /**
     * Creates deployment_identifier file.
     */
    protected function createDeployId($id) {
        //    $this->taskExecStack()->exec("echo '$id' > deployment_identifier")
        //      ->dir($this->buildDir)
        //      ->stopOnFail()
        //      ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE)
        //      ->run();
        $this->shell_exec("echo '$id' > deployment_identifier");
    }
    
    /**
     * Removes sensitive files from the deploy dir.
     */
    protected function sanitize() {
        $this->say("Sanitizing artifact...");
        
        $this->logger->comment("Removing .git subdirectories...");
        
        $this->shell_exec("find '{$this->buildDir}/vendor' -type d | grep '\.git' | xargs rm -rf", $this->buildDir);
        $this->shell_exec("find '{$this->buildDir}/{$this->documentRoot}' -type d | grep '\.git' | xargs rm -rf", $this->buildDir);
        //    $this->taskExecStack()
        //      ->exec("find '{$this->buildDir}/vendor' -type d | grep '\.git' | xargs rm -rf")
        //      ->exec("find '{$this->buildDir}/docroot' -type d | grep '\.git' | xargs rm -rf")
        //      ->stopOnFail()
        //      ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE)
        //      ->run();
        //
        //    $taskFilesystemStack = $this->taskFilesystemStack()
        //      ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE);
        
        $finder = new Finder();
        $files = $finder
            ->in($this->buildDir)
            ->files()
            ->name('CHANGELOG.txt');
        
        foreach ($files->getIterator() as $item) {
            $filepath = $item->getRealPath();
            $this->shell_exec("rm -rf $filepath", $this->buildDir);
        }
        
        $finder = new Finder();
        $files = $finder
            ->in("{$this->buildDir}/{$this->documentRoot}/core")
            ->files()
            ->name('*.txt');
        
        foreach ($files->getIterator() as $item) {
            $filepath = $item->getRealPath();
            $this->shell_exec("rm -rf $filepath", $this->buildDir);
        }
        
        $this->logger->comment("Removing .txt files...");
        //    $taskFilesystemStack->run();
    }
    
//    /**
//     * Gets the file that lists the excludes for the artifact.
//     */
//    protected function getExcludeListFile() {
//        $exclude_file = $this->getConfigValue('deploy.exclude_file');
//        $exclude_additions = $this->getConfigValue('deploy.exclude_additions_file');
//        if (file_exists($exclude_additions)) {
//            $this->say("Combining exclusions from deploy.deploy-exclude-additions and deploy.deploy-exclude files...");
//            $exclude_file = $this->mungeExcludeLists($exclude_file, $exclude_additions);
//        }
//
//        return $exclude_file;
//    }
//
//    /**
//     * Combines deploy.exclude_file with deploy.exclude_additions_file.
//     *
//     * Creates a temporary file containing the combination.
//     *
//     * @return string
//     *   The filepath to the temporary file containing the combined list.
//     */
//    protected function mungeExcludeLists($file1, $file2) {
//        $file1_contents = file($file1);
//        $file2_contents = file($file2);
//        $merged = array_merge($file1_contents, $file2_contents);
//        $merged_without_dups = array_unique($merged);
//        file_put_contents($this->excludeFileTemp, $merged_without_dups);
//
//        return $this->excludeFileTemp;
//    }
//
//    /**
//     * Sets permissions for all multisite directories.
//     */
//    protected function setMultisiteFilePermissions($perms) {
//        $taskFilesystemStack = $this->taskFilesystemStack();
//        $multisites = $this->getConfigValue('multisites');
//        foreach ($multisites as $multisite) {
//            $multisite_dir = $this->getConfigValue('docroot') . '/sites/' . $multisite;
//            $taskFilesystemStack->chmod($multisite_dir, $perms);
//        }
//        $taskFilesystemStack->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE);
//        $taskFilesystemStack->run();
//    }
    
    /**
     * Creates a commit on the artifact.
     */
    protected function commit() {
        $this->say("Committing artifact to <comment>{$this->branchName}</comment>...");
        $this->shell_exec("git add -A", $this->buildDir);
        $this->shell_exec("git commit --quiet -m '{$this->commitMessage}'", $this->buildDir);
        
        //    $result = $this->taskExecStack()
        //      ->dir($this->buildDir)
        //      ->exec("git add -A")
        //      ->exec("git commit --quiet -m '{$this->commitMessage}'")
        //      ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE)
        //      ->run();
        //
        //    if (!$result->wasSuccessful()) {
        //      throw new \Exception("Failed to commit deployment artifact!");
        //    }
    }
    
    /**
     * Pushes the artifact to git.remotes.
     */
    protected function push($identifier, $options) {
        if ($options['dry-run']) {
            $this->logger->warning("Skipping push of deployment artifact. deploy.dryRun is set to true.");
            return FALSE;
        }
        else {
            $this->say("Pushing artifact to git.remotes...");
        }
        
        //    $task = $this->taskExecStack()
        //      ->dir($this->buildDir);
        foreach ($this->git_remotes as $remote) {
            $remote_name = md5($remote);
            $this->shell_exec("git push $remote_name $identifier", $this->buildDir);
            //      $task->exec("git push $remote_name $identifier");
            
        }
        //    $result = $task->run();
        
        //    if (!$result->wasSuccessful()) {
        //      throw new \Exception("Failed to push deployment artifact!");
        //    }
    }
    
    /**
     * Creates a tag on the source repository.
     */
    protected function cutTag() {
        
        $this->shell_exec("git tag -a {$this->tagName} -m '{$this->commitMessage}'", $this->buildDir);
        //    $this->taskExecStack()
        //      ->exec("git tag -a {$this->tagName} -m '{$this->commitMessage}'")
        //      ->setVerbosityThreshold(VerbosityThresholdInterface::VERBOSITY_VERBOSE)
        //      ->stopOnFail()
        //      ->dir($this->buildDir)
        //      ->run();
        $this->say("The tag {$this->tagName} was created for the build artifact.");
    }
    
    /**
     * @param $value
     *
     * @return mixed
     */
    protected function getConfigValue($value) {
        if (isset($this->config[$value])) {
            return $this->config[$value];
        }
    }
    
    /**
     * Checkout initial git reference.
     */
    protected function cleanup() {
        if ($this->workingDir == $this->buildDir) {
            $this->say("Returning {$this->workingDir} to git reference {$this->initialGitRef}...");
            $this->shell_exec("git checkout {$this->initialGitRef}", $this->workingDir);

            $this->say("Deleting temporary branch...");
            $branch_name = $this->getDefaultBranchName() . '-temp';
            $this->shell_exec("git branch -D {$branch_name}", $this->workingDir, $this->buildDir);
        }
    
    }
}