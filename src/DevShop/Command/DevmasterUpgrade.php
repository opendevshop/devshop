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

class DevmasterUpgrade extends Command
{
  protected function configure()
  {
    $this
      ->setName('devmaster:upgrade')
      ->setDescription('Upgrade Devmaster: the Drupal front end. This command should only be run by the Ansible playbooks, which is triggered by the `devshop verify:system` command.')
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
      // Option to allow ansible runs to tell this command that it is being run as part of the playbooks.
      ->addOption(
        'run-from-playbooks', NULL, InputOption::VALUE_NONE,
        'A flag to indicate this command is being run from the playbooks. Dont use if manually running this command.'
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
    $this->announce('Devmaster Upgrade');

    $output->writeln(
      '<info>Welcome to the DevShop Devmaster Upgrader!</info>'
    );

    if ($input->getOption('run-from-playbooks') == FALSE) {
      $output->writeln('<error>This command should not be run manually unless you have a very specific reason. Please run the `devshop verify:system` command instead.</error>');
    }

    // @TODO: Check the CLI for new releases.  If, we should tell the user to run "self-update" then "upgrade".

    // Check for existing devshop install.
    // Look for aegir user
    $users = file_get_contents('/etc/passwd');
    if (strpos($users, 'aegir') === FALSE) {
      throw new \Exception('aegir user does not exist! DevShop is not installed! Installation aborted.');
    }
    $output->writeln('');

    // Check current user is root
    $pwu_data = posix_getpwuid(posix_geteuid());
    if ($pwu_data['name'] != 'root' && $pwu_data['name'] != 'aegir') {
      throw new \Exception('You must run this command as the root or aegir user. Run "sudo devshop upgrade" to run as root. 
       Devmaster upgrade aborted.');
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
    if ($input->getOption('run-from-playbooks')) {
      $target_version = $input->getArgument('devshop-version');
      $this->IO->note("Skipping GitHub version verification because --run-from-playbooks option was specified.");
    }
    else {
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
      $this->IO->note("Version $target_version confirmed.");
    }

    $this->IO->success("Upgrading DevShop to version $target_version...");

    $devmaster_makefile = $input->getOption('makefile');
    if (empty($devmaster_makefile)) {
      $devmaster_makefile = "https://raw.githubusercontent.com/opendevshop/devshop/$target_version/build-devmaster.make";
    }


    // Determine the target path.
    $target_path = "/var/aegir/devmaster-{$target_version}";

    // Check for existing path.  If exists, append the date.
    $variant = date('Y-m-d');
    if (file_exists($target_path) && $this->targetVersionRef == 'branch') {
      $target_path = "/var/aegir/devmaster-{$target_version}-{$variant}";
    }
    elseif (file_exists($target_path) && $this->targetVersionRef == 'tag') {
      $output->writeln("<comment>Version $target_version is already installed and is a tag. Nothing to do.</comment>");
      return;
    }

    // If this path exists, add a number until we find one that doesn't exist.
    if (file_exists($target_path)) {
      $number = 1;
      while (file_exists($target_path . '-' . $number)) {
        $output->writeln("File exists at " . $target_path . '-' . $number);
        $number++;
      }
      $output->writeln("File DOES NOT exists at " . $target_path . '-' . $number);
      $target_path = $target_path . '-' . $number;
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
    $output->writeln('Running hostmaster-migrate command...');
    $drush = dirname(dirname(dirname(__DIR__))) . '/bin/drush';
    $cmd = "$drush hostmaster-migrate $devmaster_uri $target_path --makefile=$devmaster_makefile --root=$devmaster_root -y";
    $question = new ConfirmationQuestion("Run the command: <comment>$cmd</comment> (y/n) ", false);

    // If they say no, exit.
    if ($input->isInteractive() && !$helper->ask($input, $output, $question)) {
      $output->writeln("<fg=red>Upgrade cancelled.</>");
      $output->writeln('');
      return;
    }

    // If they say yes, run the command.
    $output->writeln('');

    $command = $pwu_data['name'] == 'root'? "su aegir - -c '$cmd'": $cmd;
    $process = new Process($command);
    $process->setTimeout(NULL);
    $process->run(function ($type, $buffer) {
      echo $buffer;
    });

    // Only continue on successfull hostmaster-migrate.
    if (!$process->isSuccessful()) {
      throw new \Exception("Upgrade failed. The command errored: $cmd");
    }

    // Announce devmaster upgrade.
    $output->writeln('');
    $output->writeln("<info>Devmaster Upgraded to {$target_version}.</info>");

    $output->writeln("<info>Upgrade completed!  You may use the link above to login or run the command 'devshop login'.</info>");

    // Schedule the platform for deletion.
    $output->writeln('');
    $cmd = "$drush @hostmaster platform-delete $devmaster_root -y";

    $question = new ConfirmationQuestion("Run the command: <comment>$cmd</comment> (y/N) ");

    // If they say no, exit.
    if ($input->isInteractive() && !$helper->ask($input, $output, $question)) {
      $output->writeln("<fg=red>Old devmaster platform was not deleted.</> You should find and delete the platform at {$devmaster_root}");
      $output->writeln('');
      return;
    }

    // If they say yes, run drush @hostmaster platform-delete /var/aegir/devmaster-PATH
    $this->IO->note("Running $cmd");
    $command = $pwu_data['name'] == 'root'? "su aegir - -c '$cmd'": $cmd;
    $process = new Process($command);
    $process->setTimeout(NULL);
    $process->run(function ($type, $buffer) {
      echo $buffer;
    });
  }
}