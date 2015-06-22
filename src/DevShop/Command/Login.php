<?php

namespace DevShop\Command;

use Symfony\Component\Console\Command\Command;
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
    $output->writeln('Hello, DevShop!');

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