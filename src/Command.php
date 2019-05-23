<?php

namespace ProvisionOps\YamlTests;

use Symfony\Component\Console\Exception\InvalidOptionException;
use Symfony\Component\Console\Exception\LogicException;
use Symfony\Component\Console\Exception\RuntimeException;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Composer\Command\BaseCommand;
use Symfony\Component\Console\Style\SymfonyStyle;
use Symfony\Component\Finder\Finder;
use Symfony\Component\Process\Process;
use Symfony\Component\Yaml\Parser;
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
            'tests-file',
            NULL,
            InputOption::VALUE_OPTIONAL,
            'Relative path to a yml file to run.',
            'tests.yml'
        );
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

        $this->testsFile = $input->getOption('tests-file');
        $this->testsFilePath = realpath($this->testsFile);
        if (!file_exists($this->testsFilePath) || empty($this->testsFilePath)) {
            throw new \Exception("Specified tests file does not exist at {$this->workingDir}/{$this->testsFile}");
        }

        $this->io->title("Yaml Tests Initialized");
        $this->say("Composer working directory: <comment>{$this->workingDir}</comment>");
        $this->say("Git Repository directory: <comment>{$this->workingDir}</comment>");
        $this->say("Git Commit: <comment>{$this->gitRepo->getCurrentCommit()}</comment>");
        $this->say("Tests File: <comment>{$this->testsFilePath}</comment>");

        // Validate YML
        $this->loadTestsYml();

        foreach ($this->yamlTests as $name => $value){

        }

        $this->io->table(array("Tests found in " . $this->testsFile), $this->testsToTableRows());

    }
    
    /**
     * @param \Symfony\Component\Console\Input\InputInterface $input
     * @param \Symfony\Component\Console\Output\OutputInterface $output
     */
    protected function execute(InputInterface $input, OutputInterface $output)
    {

        foreach ($this->yamlTests as $test_name => $test) {
            if (is_array($test) && isset($test['command'])) {
                $command = $test['command'];
            }
            else {
                $command = $test;
            }

            $this->io->writeln("<fg=cyan>{$command}</>");
            $this->io->writeln('<fg=cyan>╔══════════════════════════════════════</>');
            $process = new Process($command);

            $process->setEnv($_SERVER);

            /** @var  $result */
            $exit = $process->run(function ($type, $buffer) use ($test_name, $output) {
                    $output->write(' ' . $buffer);
            });
            $this->io->writeln('<fg=cyan>╚══════════════════════════════════════</>');

            if ($exit == 0) {
                $this->successLite('Passed');
            }
            else {
                $this->errorLite('Failed');
            }
            $this->io->newLine();
        }

        $this->io->title("Executed");
    }

    private function loadTestsYml() {
        $this->yamlTests = Yaml::parseFile($this->testsFilePath);
    }

    private function testsToTableRows() {
        foreach ($this->yamlTests as $test_name => $test) {
            if (is_array($test) && isset($test['command'])) {
                $command = $test['command'];
            } else {
                $command = $test;
            }
            $rows[] = array($test_name, $command);
        }
        return $rows;
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