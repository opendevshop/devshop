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
        return;
      }
    }

    // Lookup versions
    $output->writeln('Checking for latest releases...');
    $client = new \Github\Client();
    $releases = $client->api('repo')->releases()->all('opendevshop', 'devshop');

    $options = array();
    foreach ($releases as $release) {
      $options[] =  $release['tag_name'];
    }

    // Ask what version
    $question = new ChoiceQuestion('<comment>What version of devshop would you like to install? (Default: Latest)</comment> ', $options, 0);
    $version = $helper->ask($input, $output, $question);

    $output->writeln("<info>Selected version $version</info>");


    // Check and confirm hostname
    $output->writeln('');
    $output->writeln('Checking hostname...');

    // See if hostname resolves to this PC.
    $hostname = gethostname();
    $ip = gethostbyname($hostname);

    $output->writeln("<comment>Current Hostname: $hostname</comment>");
    $output->writeln("<comment>IP: $ip</comment>");
    $output->writeln("The hostname is how you access the devshop front end through a web browser. The IP must resolve from the internet.");


    $question = new ConfirmationQuestion("<comment>Continue installing devshop with this hostname? (y/n) </comment> ", FALSE);

    if (!$helper->ask($input, $output, $question)){
      $output->writeln('<fg=red>Installation aborted.');
      return;
    }
    $output->writeln("<info>Selected hostname: $hostname</info>");
    $output->writeln('');

    // Confirm running on Install script as ROOT
    $script_url = "https://raw.githubusercontent.com/opendevshop/devshop/$version/install.sh";
    $output->writeln("Installation Options");
    $output->writeln("<info>Version:</info> $version");
    $output->writeln("<info>Hostname:</info> $hostname");
    $output->writeln("<info>Install Script URL:</info> $script_url");

    $question = new ConfirmationQuestion("<comment>Run the install script as root?</comment> ", FALSE);

    if (!$helper->ask($input, $output, $question)){
      $output->writeln('<fg=red>Installation aborted.');
      return;
    }

    // @TODO: Get and Run the install script.
  }
}