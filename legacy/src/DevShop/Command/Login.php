<?php

namespace DevShop\Command;

use DevShop\Console\Command;

use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;

use Symfony\Component\Process\Process;

class Login extends Command
{
  protected function configure()
  {
    $this
      ->setName('login')
      ->setDescription('Get a login link to the front-end.')
    ;
  }

  protected function execute(InputInterface $input, OutputInterface $output)
  {
    // Attaches input and output to the Command class.
    parent::execute($input, $output);

    // Announce ourselves.
    $output->writeln($this->getApplication()->getLogo());
    $this->announce('Login');

    $output->writeln('');


    if ($_SERVER['USER'] != 'aegir') {
      $output->writeln('<error>ERROR: Not running as "aegir" user.  Use "sudo su - aegir" to switch to aegir user, then try again.</error>');
      return;
    }

    // Check for Drush
    $output->write("<comment>Getting a login URL...  </comment>");

    $process = new Process('drush @hostmaster uli');
    $process->run();
    if (!$process->isSuccessful()) {
      $output->writeln("<error>Something Failed:</error>");
      $output->writeln($process->getErrorOutput());
    }
    else {
      $output->write("<info>Success: </info>");
      $output->write($process->getOutput());
    }
  }
}