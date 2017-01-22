<?php

namespace DevShop\Command;

use DevShop\Console\Command;
use Symfony\Component\Console\Input\ArrayInput;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Question\ConfirmationQuestion;
use Symfony\Component\Console\Question\Question;
use Symfony\Component\Process\Process;

class Upgrade extends Command
{
  protected function configure()
  {
    $this
      ->setName('upgrade')
      ->setDescription('Upgrade devshop')
      ->addArgument(
        'devshop-version',
        InputArgument::OPTIONAL,
        'The git tag or branch to install.'
      )

      // makefile
      ->addOption(
        'makefile', NULL, InputOption::VALUE_OPTIONAL,
        'The makefile to use to build the devmaster platform.'
      )

    ;
  }

  protected function execute(InputInterface $input, OutputInterface $output)
  {

    // Attaches input and output to the Command class.
    parent::execute($input, $output);

    $helper = $this->getHelper('question');

    // Announce ourselves.
    $output->writeln($this->getApplication()->getLogo());
    $this->announce('Upgrade');

    $output->writeln(
      '<info>Welcome to the DevShop Upgrader!</info>'
    );

    // @TODO: Check the CLI for new releases.  If, we should tell the user to run "self-update" then "upgrade".

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

    // Check current user is root
    $pwu_data = posix_getpwuid(posix_geteuid());
    if ($pwu_data['name'] != 'root') {
      $output->writeln('<error>WARNING:</error> You must run this command as the root user.');
      $output->writeln('Run "sudo devshop upgrade" to run as root.');
      $output->writeln('<fg=red>Installation aborted.</>');
      $output->writeln('');
      return;
    }
    $output->writeln('');
    $current_version = $this->getApplication()->getVersion();

    // Look for an active devmaster
    $devmaster_version = $this->getApplication()->getDevmasterVersion();
    $devmaster_uri = $this->getApplication()->getDevmasterUri();
    $devmaster_root = $this->getApplication()->getDevmasterRoot();
    if (empty($devmaster_root) || !file_exists($devmaster_root)) {
      throw new \Exception('Unable to find a devmaster.  The drush @hostmaster alias does not exist.  Unable to upgrade.');
    }

    // Lookup latest version.
    $output->writeln('Checking for latest releases...');
    $client = new \Github\Client();
    $release = $client->repositories()->releases()->latest('opendevshop', 'devshop');

    // Make sure we got the release info
    if (empty($release)) {
      $output->writeln("<fg=red>Unable to retrieve releases from GitHub.  Try again later, or specify a release.</>");
      $latest_release = '0.x';
      $output->writeln("Assuming development version <info>$latest_release</info>.");
    }
    else {
      $latest_release = $release['tag_name'];
      $output->writeln("<info>Latest Version</info> $latest_release");
    }

    $default_version = $input->getArgument('devshop-version')? $input->getArgument('devshop-version'): $latest_release;
    $target_version = '';

    // Warn if default is not latest
    if ($latest_release != $default_version) {
        $output->writeln("<fg=red>WARNING:</> You have specified a release that is not the latest.");
    }

    // Confirm version
    while ($this->checkVersion($target_version) == FALSE) {
      $question = new Question("Target Version: (Default: $default_version) ", $default_version);
      $target_version = $helper->ask($input, $output, $question);

      if (!$this->checkVersion($target_version)) {
        $output->writeln("<fg=red>Version $target_version not found</>");
      }
    }
    $output->writeln("Version $target_version confirmed.");

    $devmaster_makefile = $input->getOption('makefile');
    if (empty($devmaster_makefile)) {
      $devmaster_makefile = "https://raw.githubusercontent.com/opendevshop/devshop/$target_version/build-devmaster.make";
    }


    // Determine the target path.
    $target_path = "/var/aegir/devmaster-{$target_version}";

    // Check for existing path.  If exists, append the date.
    if ($target_version == '0.x' || file_exists($target_path)) {
      $variant = date('Y-m-d');
      $target_path = "/var/aegir/devmaster-{$target_version}-{$variant}";
    }

    // If this path exists, add a number until we find one that doesn't exist.
    if (file_exists($target_path)) {
      $number = 1;
      while (file_exists($target_path . '-' . $number)) {
        $number++;
      }
      $variant .= '-' . $number;
      $target_path = "/var/aegir/devmaster-{$target_version}-{$variant}";
    }

    $output->writeln('');

    $output->writeln('UPGRADE OPTIONS');
    $output->writeln("<info>Current CLI Version: </info>       $current_version");
    $output->writeln("<info>Current DevMaster Version: </info> $devmaster_version");
    $output->writeln("<info>Current DevMaster Path: </info>    $devmaster_root");
    $output->writeln("<info>Current DevMaster Site: </info>    $devmaster_uri");
    $output->writeln('');

    $output->writeln("<info>Target Version: </info> " . $target_version);
    $output->writeln("<info>Target DevMaster Path: </info> " . $target_path);
    $output->writeln("<info>Target DevMaster Makefile: </info> " . $devmaster_makefile);

    $output->writeln('');

    // Check for site in target path
    if (file_exists($target_path)) {
      $output->writeln("<fg=red>WARNING:</> There is already a site located at <comment>$target_path</comment>. Please check your version and paths and try again.");
      $output->writeln('');
      return;
    }

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
    $cmd = "drush hostmaster-migrate $devmaster_uri $target_path --makefile=$devmaster_makefile --root=$devmaster_root -y";
    $question = new ConfirmationQuestion("Run the command: <comment>$cmd</comment> (y/n) ", false);

    // If they say no, exit.
    if ($input->isInteractive() && !$helper->ask($input, $output, $question)) {
      $output->writeln("<fg=red>Upgrade cancelled.</>");
      $output->writeln('');
      return;
    }

    // If they say yes, run the command.
    $output->writeln('');

    $process = new Process("su aegir - -c '$cmd'");
    $process->setTimeout(NULL);
    $process->run(function ($type, $buffer) {
      echo $buffer;
    });

    // Only continue on successfull hostmaster-migrate.
    if (!$process->isSuccessful()) {
      $output->writeln("<fg=red>Upgrade failed.</>  The command failed:");
      $output->writeln($cmd);
      $output->writeln('');
      return;
    }

    // Announce devmaster upgrade.
    $output->writeln('');
    $output->writeln("<info>Devmaster Upgraded to {$target_version}.</info>");

    // Run the ansible playbook.
    $output->writeln('');
    $question = new ConfirmationQuestion("STEP 2: Run playbook (y/n) ", false);

    // If they say no, exit.
    if ($input->isInteractive() && !$helper->ask($input, $output, $question)) {
      $output->writeln("<fg=red>Upgrade cancelled.</>");
      $output->writeln('');
      return;
    }

    // If they say yes, run the command.
    $output->writeln('');
    $command = $this->getApplication()->find('install');

    $arguments = array(
        'command' => 'install',
        'devshop-version' => $target_version,
        '--yes' => 1,
    );

    $upgradeInput = new ArrayInput($arguments);
    $output->writeln('');

    if ($command->run($upgradeInput, $output) != 0) {
      $output->writeln("<fg=red>Playbook run failed!</>");
      $output->writeln('');
    }

    $output->writeln("<info>Upgrade completed!  You may use the link above to login or run the command 'devshop login'.</info>");


    // Schedule the command for deletion.
    $output->writeln('');
    $question = new ConfirmationQuestion("STEP 3: Schedule deletion of old platform ($devmaster_root) ", false);
    $cmd = "drush @hostmaster platform-delete $devmaster_root -y";

    $question = new ConfirmationQuestion("Run the command: <comment>$cmd</comment> (y/N) ");

    // If they say no, exit.
    if ($input->isInteractive() && !$helper->ask($input, $output, $question)) {
      $output->writeln("<fg=red>Old devmaster platform was not deleted.</> You should find and delete the platform at {$devmaster_root}");
      $output->writeln('');
      return;
    }

    // If they say yes, run drush @hostmaster platform-delete /var/aegir/devmaster-PATH
    $process = new Process("su aegir - -c '$cmd'");
    $process->setTimeout(NULL);
    $process->run(function ($type, $buffer) {
      echo $buffer;
    });
  }
}