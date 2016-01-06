<?php

namespace DevShop\Command;

use DevShop\Console\Command;

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

    // Attaches input and output to the Command class.
    parent::execute($input, $output);

    // Announce ourselves.
    $output->writeln($this->getApplication()->getLogo());
    $this->announce('Status');

    // Check for DevShop CLI
    $output->write("<comment>Checking for DevShop CLI...  </comment>");
    $version = trim($this->getApplication()->getVersion());
    $output->write("<info>DevShop CLI is installed.</info>  ");
    $output->writeln($version);

    // Check for Drush
    $output->write("<comment>Checking for Drush...  </comment>");

    $process = new Process('drush --version');
    $process->run();
    if (!$process->isSuccessful()) {
      $output->writeln("<question>Drush not detected.</question>");
      $output->writeln($process->getErrorOutput());
    }
    else {
      $output->write("<info>Drush is installed.  </info>");
      $output->writeln(trim($process->getOutput()));
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
      $output->write("<info>Devmaster is installed.  </info>");
      $output->writeln($this->getApplication()->getDevmasterVersion());
    }

    $output->writeln('');

    // Check for deprecated .devshop-version file.
    if (file_exists('/var/aegir/.devshop-version')) {
      $output->writeln("<fg=red>A deprecated file was found. You should delete '/var/aegir/.devshop-version' to reduce confusion.</>");
    }
  }
}