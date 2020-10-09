<?php

namespace DevShop\Command;

use DevShop\Console\Command;

use Phar;

use Herrera\Json\Exception\JsonException;
use Herrera\Phar\Update\Manager;
use Herrera\Phar\Update\Manifest;
use Herrera\Json\Exception\FileException;

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
use Symfony\Component\Console\Exception\RuntimeException;

use TQ\Vcs\Cli\CallException;
use TQ\Vcs\Cli\CallResult;

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
      ->addOption('ignore-working-copy-changes',
        'i',
        InputOption::VALUE_NONE,
        'Ignore working copy changes. Used during development.'
      )
    ;
  }

  protected function execute(InputInterface $input, OutputInterface $output)
  {
    parent::execute($input, $output);

//    if (Phar::running()) {
//
//      $file = self::MANIFEST_FILE;
//      $output->writeln("Loading <info>DevShop</info> release information from <comment>{$file}</comment>");
//
//      try {
//        $manager = new Manager(Manifest::loadFile(self::MANIFEST_FILE));
//        $manager->update($this->getApplication()->getVersion(), true);
//      } catch (JsonException $e) {
//        $output->writeln('<error>'.$e->getMessage().'</error>');
//        $output->writeln(
//          'Contact the DevShop maintainers if the problem persists.'
//        );
//      }
//      $output->writeln("<error>Self-update for Phar is not yet supported. If you installed this phar you are a developer anyway!</error>");
//    }
//    // When this is not a PHAR...
//    else {

    try {

      // 1. Check if this script is a git repo.
      $this->checkCliVersion();
      $version = $this->getApplication()->getVersion();
      $target_version = $this->input->getArgument('devshop-version');
      $path = realpath(__DIR__ . '/../../../');

      // If target version is missing, load the latest.
      if (empty($target_version)) {
        $output->writeln('Checking for latest releases...');
        $target_version = $this->getLatestVersion();
      }

      // Confirm version with GitHub if interactive.
      if ($input->isInteractive()) {
        $helper = $this->getHelper('question');
        $version_found = FALSE;
        while ($version_found == FALSE) {
          $question = new Question("Target Version: (Default: $target_version) ", $target_version);
          $target_version = $helper->ask($input, $output, $question);

          if (!$this->checkVersion($target_version)) {
            $output->writeln("<fg=red>Version $target_version not found</>");
          }
          else {
            $output->writeln("Version $target_version confirmed.");
            $version_found = TRUE;
          }
        }
      }

      // Bail if there are working copy changes ignoring untracked files, or if repo is ahead.
      $is_ahead = strpos($this->callGit('status', array('--short', '--branch'))->getStdOut(), 'ahead') !== FALSE;
      $is_dirty = $this->getRepository()->isDirty();
      if  (!$input->getOption('ignore-working-copy-changes') && $is_ahead) {
        $this->IO->block('Your local clone of the DevShop source code has un-pushed commits.', 'WARNING', 'fg=white;bg=yellow', ' ', true);
        $output->write($this->callGit('status')->getStdOut());
        throw new RuntimeException("Cancelling self-update to avoid losing your commits. Run 'git push' to save your commits or use the --ignore-working-copy-changes option to skip this check.");
      }
      elseif (!$input->getOption('ignore-working-copy-changes') && $is_dirty) {
        $this->IO->block('DevShop source code git repository is "dirty".', 'WARNING', 'fg=white;bg=yellow', ' ', true);
        $output->write($this->callGit('status')->getStdOut());
        throw new RuntimeException("There are changes to your working copy at $path. Commit or revert the changes, or use the --ignore-working-copy-changes option to skip this check.");
      }

      // Checkout the desired version.
      if (isset($_SERVER['GITHUB_HEAD_REF']) && $_SERVER['GITHUB_HEAD_REF'] == $target_version) {
        $output->writeln('<comment>Target version is the same as PR Branch. Skipping git checkout.</comment>');
      }
      else {
        $this->callGit('fetch', array(
           '--all',
           '--tags',
        ), 'Unable to fetch tags.');

        $this->callGit('checkout', array(
          $target_version,
        ), sprintf('Checkout of target version "%s" failed.', $target_version));

        // If we are on a branch, reset to origin.
        // This is the better than using git pull because it can handle force pushed branches.
        try {
          $this->getRepository()->getCurrentBranch();
          $this->callGit('reset', array(
            '--hard',
            sprintf('origin/%s', $target_version),
          ), sprintf('Unable to reset working copy to target version "%s".', $target_version));
        }
        catch (CallException $e) {
          $output->writeln("DevShop CLI version <info>{$target_version}</info> has been checked out.");
        }
      }

      $output->writeln("DevShop CLI version <info>{$target_version}</info> has been checked out.");

      // Run 'composer install' in the directory.
      $output->writeln("Running <info>composer install</info> in <comment>{$git->getDirectory()}</comment>...");
      $process = new Process('composer install --no-plugins --no-scripts --ansi', $git->getDirectory());
      $process->setTimeout(NULL);
      $process->mustRun(function ($type, $buffer) {
        if (Process::ERR === $type) {
          echo $buffer;
        } else {
          echo $buffer;
        }
      });
    } catch (GitException $e) {
      throw new \Exception('Git failed: ' . $e->getMessage());
    } catch (ProcessFailedException $e) {
      throw new \Exception('Process Failed: ' . $e->getMessage());
    }

    $output->writeln("<info>DevShop CLI Updated to version $target_version.</info>");

  }
}