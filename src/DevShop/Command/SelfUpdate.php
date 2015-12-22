<?php

namespace DevShop\Command;

use DevShop\Console\Command;

use Phar;
use GitWrapper\GitWrapper;
use GitWrapper\GitException;
use Herrera\Json\Exception\JsonException;
use Herrera\Phar\Update\Manager;
use Herrera\Phar\Update\Manifest;
use Herrera\Json\Exception\FileException;

use Github\Exception\RuntimeException;
use Symfony\Component\Console\Input\ArrayInput;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Question\Question;
use Symfony\Component\Console\Question\ChoiceQuestion;
use Symfony\Component\Console\Question\ConfirmationQuestion;

use Symfony\Component\Process\Process;
use Symfony\Component\Process\Exception\ProcessFailedException;
use Github\Client;

use Symfony\Component\Filesystem\Filesystem;
use Symfony\Component\Filesystem\Exception\IOExceptionInterface;
use Symfony\Component\Finder\Finder;

class SelfUpdate extends Command
{
  // @TODO: We don't have self-update fully working in PHAR mode.
  // The URL of the DevShop Version manifest.  See the gh-pages branch.
  const MANIFEST_FILE = 'http://opendevshop.github.io/devshop/manifest.json';

  protected function configure()
  {
    $this
      ->setName('self-update')
      ->setDescription('Updates the DevShop CLI to the latest version.')
      ->setHelp(<<<EOT
The <info>self-update</info> command checks http://github.com/opendevshop/devshop for newer
versions of the DevShop CLI and if found, installs the latest.

The DevShop CLI uses git to configure releases. The current git reference is the version of the DevShop CLI.

<info>devshop self-update</info>

EOT
      )
      ->addArgument(
        'devshop-version',
        InputArgument::OPTIONAL,
        'The git tag or branch to install.'
      )
    ;
  }

  protected function execute(InputInterface $input, OutputInterface $output)
  {
    if (Phar::running()) {

      $file = self::MANIFEST_FILE;
      $output->writeln("Loading <info>DevShop</info> release information from <comment>{$file}</comment>");

      try {
        $manager = new Manager(Manifest::loadFile(self::MANIFEST_FILE));
        $manager->update($this->getApplication()->getVersion(), true);
      } catch (JsonException $e) {
        $output->writeln('<error>'.$e->getMessage().'</error>');
        $output->writeln(
          'Contact the DevShop maintainers if the problem persists.'
        );
      }
      $output->writeln("<error>Self-update for Phar is not yet supported. If you installed this phar you are a developer anyway!</error>");
    }
    // When this is not a PHAR...
    else {

      try {

        // 1. Check if this script is a git repo.
        $git_wrapper = new GitWrapper();
        $path = realpath(__DIR__ . '/../../../');

        $git = $git_wrapper->workingCopy($path);
        $git->status();
        $output->writeln("Git repo found at <info>$path</info>");

        $latest = '0.x';
        $name_question = new Question("Version? [{$latest}] ", $latest);
        $version = $this->getAnswer($input, $output, $name_question, 'devshop-version', TRUE);

        // Bail if there are working copy changes, ignoring untracked files.
        // This is similar to \GitWrapper\GitWorkingCopy::hasChanges()
        if ($git->getWrapper()->git('status -s --untracked-files=no', $git->getDirectory())) {
          throw new \Exception("There are changes to your working copy at $path.  Please resolve this and try again.");
        }

        // Checkout the desired version.
        $git->fetchAll();
        $git->checkout($version);

        $output->writeln("DevShop CLI version <info>{$version}</info> has been checked out.");

        // Run 'composer install' in the directory.
        $process = new Process('composer install --ansi', $git->getDirectory());
        $process->setTimeout(NULL);
        $process->mustRun(function ($type, $buffer) {
          if (Process::ERR === $type) {
            echo $buffer;
          } else {
            echo $buffer;
          }
        });
      } catch (GitException $e) {
        $output->writeln('<error>ERROR: ' . $e->getMessage() . '</error>');
      } catch (ProcessFailedException $e) {
        echo $e->getMessage();
      }
    }
  }
}