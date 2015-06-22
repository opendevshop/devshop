<?php

namespace DevShop\Command;

use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Question\ChoiceQuestion;
use Symfony\Component\Console\Question\ConfirmationQuestion;

use Symfony\Component\Process\Process;
use Github\Client;

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

    $output->writeln('Selected version ' . $version);

  }
}