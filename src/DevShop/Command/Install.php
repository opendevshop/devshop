<?php

namespace DevShop\Command;

use DevShop\Console\Command;
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
      ->addArgument(
        'devshop-version',
        InputOption::VALUE_OPTIONAL,
        'The git tag or branch to install.'
      )
      ->addOption(
        'yes',
        'y',
        InputOption::VALUE_NONE,
        'Answer "yes" to all questions.'
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
    $this->announce('Interactive Installer');

    $output->writeln('<info>Welcome to the Interactive DevShop Installer!</info>');
    $output->writeln('');

    // Check current user is root
    $pwu_data = posix_getpwuid(posix_geteuid());
    if ($pwu_data['name'] != 'root') {
      $output->writeln('<error>WARNING:</error> You must run this command as the root user.');
      $output->writeln('Run "sudo devshop install" to run as root.');
      $output->writeln('<fg=red>Installation aborted.</>');
      $output->writeln('');
      return;
    }

    // Check for existing devshop install.
    // Look for aegir user
    $users = file_get_contents('/etc/passwd');
    if (strpos($users, 'aegir') !== FALSE) {
      $output->writeln('<error>WARNING:</error> aegir user already exists.');
      $question = new ConfirmationQuestion('Do you want to continue with the installation? ', false);
      if ($input->getOption('yes') == NULL && !$helper->ask($input, $output, $question)) {
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
    $version = $input->getArgument('devshop-version');
    if (empty($version)) {
      $output->writeln('Checking for latest releases...');
      $client = new \Github\Client();
      $release = $client->api('repo')->releases()->latest('opendevshop', 'devshop');
      $version = $release['tag_name'];

      // Confirm version
      $question = new Question("Target Version: (Default: $version) ", $version);
      $version = $helper->ask($input, $output, $question);
    }

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

    if ($input->getOption('yes') != 1) {
      $question = new ConfirmationQuestion("<comment>Run the install script as root?</comment> ", FALSE);

      if (!$helper->ask($input, $output, $question)){
        $output->writeln('<fg=red>Installation aborted.');
        $output->writeln('');
        return;
      }
    }
    else {
      $output->writeln("Running <comment>$script_url</comment> as root.");

    }

    // Get and Run the install script.
    $fs = new Filesystem();

    try {
      $fs->copy($script_url, '/tmp/devshop-install.sh');
    } catch (IOExceptionInterface $e) {
      $output->writeln("<error>An error occurred while retrieving the install script.</error>");
    }

    // Run devshop-install script.
    $process = new Process("bash /tmp/devshop-install.sh");
    $process->setTimeout(NULL);
    $process->run(function ($type, $buffer) {
      echo $buffer;
    });

    return TRUE;
  }
}