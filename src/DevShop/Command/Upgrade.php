<?php

namespace DevShop\Command;

use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Question\Question;
use Symfony\Component\Console\Question\ChoiceQuestion;
use Symfony\Component\Console\Question\ConfirmationQuestion;

use Symfony\Component\Process\Process;
use Github\Client;

use Symfony\Component\Filesystem\Filesystem;
use Symfony\Component\Filesystem\Exception\IOExceptionInterface;
use Symfony\Component\Finder\Finder;

class Upgrade extends Command
{
  protected function configure()
  {
    $this
      ->setName('upgrade')
      ->setDescription('Upgrade devshop')
    ;
  }

  protected function execute(InputInterface $input, OutputInterface $output)
  {
    $formatter = $this->getHelper('formatter');
    $helper = $this->getHelper('question');

    $errorMessages = array(
      '╔═══════════════════════════════════════════════════════════════╗',
      '║           ____  Welcome to  ____  _                           ║',
      '║          |  _ \  _____   __/ ___|| |__   ___  _ __            ║',
      '║          | | | |/ _ \ \ / /\___ \|  _ \ / _ \|  _ \           ║',
      '║          | |_| |  __/\ V /  ___) | | | | (_) | |_) |          ║',
      '║          |____/ \___| \_/  |____/|_| |_|\___/| .__/           ║',
      '║                  Upgrade                     |_|              ║',
      '╚═══════════════════════════════════════════════════════════════╝',
    );
    $formattedBlock = $formatter->formatBlock($errorMessages, 'fg=black;bg=green');
    $output->writeln($formattedBlock);

    // Check for existing devshop install.
    // Look for aegir user
    $users = file_get_contents('/etc/passwd');
    if (strpos($users, 'aegir') === FALSE) {
      $output->writeln('<error>WARNING:</error> aegir user does not exist! DevShop is not installed!');
      $output->writeln('<fg=red>Installation aborted.');
      $output->writeln('');
      return;
    }
    $output->writeln('');

    // Look for .devshop-version (Pre 0.3 does not have this file.)
    if (file_exists('/var/aegir/.devshop-version')) {
      $current_version = file_get_contents('/var/aegir/.devshop-version');
      if (!empty($current_version)) {
        $output->writeln("<info>Current Version:</info> $current_version");
      }
      else {
        $output->writeln("<error> ERROR: </error> /var/aegir/.devshop-version was found but was empty!");
        return;
      }
    }
    else {
      $current_version = 'unknown';
      $output->writeln("<fg=red>WARNING:</> Unable to detect current version of devshop.");
      $output->writeln("There is no <comment>/var/aegir/.devshop-version</comment> file.");
      $output->writeln('');
    }

    // Look for an active hostmaster
    require('/var/aegir/.drush/hostmaster.alias.drushrc.php');
    $devmaster_root = $aliases['hostmaster']['root'];
    if (!file_exists($devmaster_root)) {
      $output->writeln("<error>WARNING:</error> No active drush alias <comment>@hostmaster</comment> was found!");
      $output->writeln("<fg=red>Aborting upgrade</>");
      $output->writeln('');
      return;
    }

    $devmaster_root = $aliases['hostmaster']['root'];

    // Lookup latest version.
    $output->writeln('Checking for latest releases...');
    $client = new \Github\Client();
    $release = $client->api('repo')->releases()->latest('opendevshop', 'devshop');

    $target_version = $release['tag_name'];
    $target_path = "/var/aegir/devmaster-{$target_version}";

    $output->writeln("<info>Current DevShop Path: </info> $devmaster_root");
    $output->writeln("<info>Current Version: </info> " . $current_version);

    $output->writeln("<info>Latest Release: </info> " . $target_version);
    $output->writeln("<info>Target DevShop Path: </info> " . $target_path);

    $output->writeln('');
  }
}