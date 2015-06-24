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

class Install extends Command
{
  protected function configure()
  {
    $this
      ->setName('install')
      ->setDescription('Install devshop')
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
      '║               Interactive Installer          |_|              ║',
      '╚═══════════════════════════════════════════════════════════════╝',
    );
    $formattedBlock = $formatter->formatBlock($errorMessages, 'fg=black;bg=green');
    $output->writeln($formattedBlock);

    $output->writeln('<info>Welcome to the Interactive DevShop Installer!</info>');
    $output->writeln('');

    // Check for existing devshop install.
    // Look for aegir user
    $users = file_get_contents('/etc/passwd');
    if (strpos($users, 'aegir') !== FALSE) {
      $output->writeln('<error>WARNING:</error> aegir user already exists.');
      $question = new ConfirmationQuestion('Do you want to continue with the installation? ', false);
      if (!$helper->ask($input, $output, $question)) {
        $output->writeln('<fg=red>Installation aborted.');
        $output->writeln('');

        return;
      }
    }
    $output->writeln('');

//    if (file_exists('/var/aegir/.devshop-version')) {
//      $current_version = file_get_contents('/var/aegir/.devshop-version');
//      if (!empty($current_version)) {
//        $output->writeln("<error>WARNING:</error> /var/aegir/.devshop-version was found. Version $current_version appears to already be installed.  You should run <info>devshop upgrade</info> command if you wish to upgrade it.");
//        $output->writeln('<fg=red>Installation aborted.');
//        $output->writeln('');
//        return;
//      }
//    }

    // Lookup latest version.
    $output->writeln('Checking for latest releases...');
    $client = new \Github\Client();
    $release = $client->api('repo')->releases()->latest('opendevshop', 'devshop');
    $version = $release['tag_name'];

    // Confirm version
    $question = new Question("Target Version: (Default: $version) ", $version);
    $version = $helper->ask($input, $output, $question);

    $output->writeln("<info>Selected version $version</info>");
    $output->writeln('');

    // Confirm running on Install script as ROOT
    $hostname = gethostname();
    $ip = gethostbyname($hostname);
    $script_url = "https://raw.githubusercontent.com/opendevshop/devshop/$version/install.sh";
    $output->writeln("Installation Options");
    $output->writeln("<info>Version:</info> $version");
    $output->writeln("<info>Hostname:</info> $hostname");
    $output->writeln("<info>IP:</info> $ip");
    $output->writeln("<info>Install Script URL:</info> $script_url");
    $output->writeln("The hostname is how you access the devshop front end through a web browser. The IP must resolve from the internet. If you wish to change the hostname or need to update DNS, cancel the installer by entering 'n'.");

    $question = new ConfirmationQuestion("<comment>Run the install script as root?</comment> ", FALSE);

    if (!$helper->ask($input, $output, $question)){
      $output->writeln('<fg=red>Installation aborted.');
      $output->writeln('');
      return;
    }

    // Get and Run the install script.
    $fs = new Filesystem();

    try {
      $fs->copy($script_url, '/tmp/devshop-install.sh');
    } catch (IOExceptionInterface $e) {
      $output->writeln("<error>An error occurred while retrieving the install script.</error>");
    }

    // If running as root, just run bash.
    if (get_current_user() == 'root') {
      $process = new Process("bash /tmp/devshop-install.sh");
    }
    // If not running as root, ask for password.
    else {
      $question = new Question('Password: ', NULL);
      $question->setHidden(TRUE);
      $password = $helper->ask($input, $output, $question);
      $process = new Process("echo $password | sudo su - -c 'bash /tmp/devshop-install.sh'");
    }

    $process->setTimeout(NULL);
    $process->run(function ($type, $buffer) {
      echo $buffer;
    });
  }
}