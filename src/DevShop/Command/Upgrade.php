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
      $output->writeln("This is probably because you are running a version prior to 0.3.");
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
    $devmaster_uri = $aliases['hostmaster']['uri'];

    // Lookup latest version.
    $output->writeln('Checking for latest releases...');
    $client = new \Github\Client();
    $release = $client->api('repo')->releases()->latest('opendevshop', 'devshop');
    $target_version = $release['tag_name'];

    // Confirm version
    $question = new Question("Target Version: (Default: $target_version) ", $target_version);
    $target_version = $helper->ask($input, $output, $question);

    // @TODO: Verify version exists.

    $target_path = "/var/aegir/devmaster-{$target_version}";
    $output->writeln('');

    $output->writeln('UPGRADE OPTIONS');
    $output->writeln("<info>Current DevMaster Path: </info> $devmaster_root");
    $output->writeln("<info>Current Version: </info> " . $current_version);
    $output->writeln("<info>Current DevMaster Site: </info> " . $devmaster_uri);
    $output->writeln('');

    $output->writeln("<info>Target DevMaster Path: </info> " . $target_path);
    $output->writeln("<info>Latest Release: </info> " . $target_version);

    $output->writeln('');

    //@TODO: Finalize the upgrade process.
    // Aegir's process is split between 'upgrade.sh.txt' and a drush command "hostmaster-migrate"

    // 0. Update composer.
    // 1. Update drush  (postponed until we figure out how to leverage composer for this.)
    // 2. Download updated drush components
    // 3. Git checkout /usr/share/devshop to get the latest release.
    // 4. Run `drush hostmaster-migrate $HOSTNAME $PLATFORM_PATH --makefile=$MAKEFILE_PATH.
    // 5. Hope for the best.
    // 6. Run "install.sh" as root (should be renamed) to run the ansible playbook on the server.

    // 3. Git checkout /usr/share/devshop to get the latest release.

    // Upgrade DevMaster
    $output->writeln('STEP 1: Upgrade DevMaster');
    $cmd = "drush hostmaster-migrate $devmaster_uri $target_path --makefile=https://raw.githubusercontent.com/opendevshop/devshop/$target_version/build-devmaster.make --root=$devmaster_root -y";
    $question = new ConfirmationQuestion("Run the command: <comment>$cmd</comment> (y/n) ", false);

    // If they say no, exit.
    if (!$helper->ask($input, $output, $question)) {
      $output->writeln("<fg=red>Aborting upgrade</>");
      $output->writeln('');
      return;
    }

    // If they say yes, run the command.
    $output->writeln('');

    $process = new Process($cmd);
    $process->setTimeout(NULL);
    $process->run(function ($type, $buffer) {
      echo $buffer;
    });


  }
}