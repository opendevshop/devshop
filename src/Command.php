<?php

namespace ProvisionOps\YamlTests;

use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Composer\Command\BaseCommand;
use Symfony\Component\Console\Style\SymfonyStyle;
use Symfony\Component\Finder\Finder;
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
     * @var Repository
     */
    protected $gitRepo;

    /**
     * The current git commit SHA.
     * @var String
     */
    protected $gitSha;

    /**
     * The options from the project's composer.json "config" section.
     *
     * @var array
     */
    protected $config = [];

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
        $this->workingDir = getcwd();

        $this->gitRepo = Repository::open($this->workingDir);
        $this->gitRepo->getCurrentCommit();

        $this->config = $this->getComposer()->getPackage()->getConfig();

        $this->io->title("Yaml Tests Initialized");
        $this->say("Composer working directory: <comment>{$this->workingDir}</comment>");
        $this->say("Git Repository directory: <comment>{$this->workingDir}</comment>");
        $this->say("Git Commit: <comment>{$this->gitRepo->getCurrentCommit()}</comment>");

    }
    
    /**
     * @param \Symfony\Component\Console\Input\InputInterface $input
     * @param \Symfony\Component\Console\Output\OutputInterface $output
     */
    protected function execute(InputInterface $input, OutputInterface $output)
    {
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
}