<?php

namespace ProvisionOps\YamlTests;

use Composer\Plugin\Capability\CommandProvider as CommandProviderCapability;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;
use Composer\Command\BaseCommand;

class YamlTestsCommandProvider implements CommandProviderCapability
{
  public function getCommands()
  {
    return array(new Command);
  }
}

class Command extends BaseCommand
{
  protected function configure()
  {
    $this->setName('yaml-tests');
  }

  protected function execute(InputInterface $input, OutputInterface $output)
  {
    $output->writeln('Executing');
  }
}