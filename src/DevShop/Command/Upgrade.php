<?php

namespace DevShop\Command;

use Github\Exception\RuntimeException;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Input\ArrayInput;
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

      $fs = new Filesystem();
      $fs->dumpFile('/var/aegir/.devshop-version', '0.x');

      $output->writeln("We have created this file for you.");
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
    $release = $client->repositories()->releases()->latest('opendevshop', 'devshop');
    $default_version = $release['tag_name'];
    $target_version = '';

    // Confirm version
    while ($this->checkVersion($target_version) == FALSE) {
      $question = new Question("Target Version: (Default: $default_version) ", $default_version);
      $target_version = $helper->ask($input, $output, $question);
      $devmaster_makefile = "https://raw.githubusercontent.com/opendevshop/devshop/$target_version/build-devmaster.make";

      if (!$this->checkVersion($target_version)) {
        $output->writeln("<fg=red>Version $target_version not found</>");
      }
    }
    $output->writeln("Version $target_version confirmed.");


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
    $output->writeln("<info>Current Version: </info> " . $current_version);
    $output->writeln("<info>Current DevMaster Path: </info> $devmaster_root");
    $output->writeln("<info>Current DevMaster Site: </info> " . $devmaster_uri);
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
    if (!$helper->ask($input, $output, $question)) {
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
      $output->writeln();
      return;
    }

    // Announce devmaster upgrade.
    $output->writeln('');
    $output->writeln("<info>Devmaster Upgraded to {$target_version}.</info>");

    // Run the ansible playbook.
    $output->writeln('');
    $question = new ConfirmationQuestion("STEP 2: Run playbook (y/n) ", false);

    // If they say no, exit.
    if (!$helper->ask($input, $output, $question)) {
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
    $cmd = "drush @hostmaster platform-remove $devmaster_root";

    $question = new ConfirmationQuestion("Run the command: <comment>$cmd</comment> (y/N) ");

    // If they say no, exit.
    if (!$helper->ask($input, $output, $question)) {
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

  public function checkVersion($version) {
    $client = new \Github\Client();

    try {
      $ref = $client->getHttpClient()->get('repos/opendevshop/devshop/git/refs/heads/' . $version);
      return TRUE;
    }
    catch (RuntimeException $e) {
    }

    try {
      $ref = $client->getHttpClient()->get('repos/opendevshop/devshop/git/refs/tags/' . $version);
      return TRUE;
    }
    catch (RuntimeException $e) {
      return FALSE;
    }
  }
}