<?php

namespace DevShop\Command;

use Herrera\Json\Exception\JsonException;
use Herrera\Phar\Update\Manager;
use Herrera\Phar\Update\Manifest;
use Herrera\Json\Exception\FileException;

use Github\Exception\RuntimeException;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Input\ArrayInput;
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
use Symfony\Component\Finder\Finder;

class SelfUpdate extends Command
{
  const MANIFEST_FILE = 'http://opendevshop.github.io/devshop/manifest.json';

  protected function configure()
  {
    $this
      ->setName('self-update')
      ->setDescription('Updates the DevShop CLI to the latest version.')
      ->setHelp(<<<EOT
The <info>self-update</info> command checks http://github.com/opendevshop/devshop for newer
versions of the DevShop CLI and if found, installs the latest.

The DevShop CLI uses git to configure releases. The current git reference is the version of the DevShop CLI.

<info>devshop self-update</info>

EOT
      )
    ;
  }

  protected function execute(InputInterface $input, OutputInterface $output)
  {

    $file = self::MANIFEST_FILE;
    $output->writeln("Loading <info>DevShop</info> release information from <comment>{$file}</comment>");

    try {
      $manager = new Manager(Manifest::loadFile(self::MANIFEST_FILE));
      $manager->update($this->getApplication()->getVersion(), true);
    }
    catch (JsonException $e) {
      $output->writeln('<error>' . $e->getMessage() . '</error>');
      $output->writeln('Contact the DevShop maintainers if the problem persists.');
    }
  }
}