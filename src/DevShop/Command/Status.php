<?php

namespace DevShop\Command;

use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;

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
  }
}