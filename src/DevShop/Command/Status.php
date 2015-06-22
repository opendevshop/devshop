<?php

namespace DevShop\Command;

use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;

use Symfony\Component\Process\Process;

class Status extends Command
{
  protected function configure()
  {
    $this
      ->setName('status')
      ->setDescription('Check status')
    ;
  }

  protected function execute(InputInterface $input, OutputInterface $output)
  {
    $output->writeln('Hello, DevShop!');

    // Check for Drush
    $output->write("<comment>Checking for Drush...  </comment>");

    $process = new Process('drush --version');
    $process->run();
    if (!$process->isSuccessful()) {
      $output->writeln("<question>Drush not detected.</question>");
      $output->writeln($process->getErrorOutput());
    }
    else {
      $output->write("<info>Drush is installed.</info>");
      $output->write($process->getOutput());
    }

    // Check for provision
    $output->write("<comment>Checking for Provision...  </comment>");

    $process = new Process('drush help provision-save');
    $process->run();
    if (!$process->isSuccessful()) {
      $output->writeln("<error>Provision not detected.</error>");
      $output->writeln($process->getErrorOutput());
    }
    else {
      $output->writeln("<info>Provision is installed.</info>");
    }

    // Check for devmaster
    $output->write("<comment>Checking for DevMaster...  </comment>");

    $process = new Process('drush @hostmaster status');
    $process->run();
    if (!$process->isSuccessful()) {
      $output->writeln("<error>Devmaster not detected.</error>");
      $output->writeln($process->getErrorOutput());
    }
    else {
      $output->writeln("<info>Devmaster is installed.</info>");
    }

    // Check for devshop

    $output->write("<comment>Checking for devshop...  </comment>");

    if (file_exists('/var/aegir/.devshop-version')) {
      $version = file_get_contents('/var/aegir/.devshop-version');
      $output->write("<info>Devshop is installed.</info>  ");
      $output->write("DevShop Version $version");
    }
  }
}