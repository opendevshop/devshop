<?php

/*
 * This file is part of the DevShop package.
 *
 * (c) Jon Pugh <jon@thinkdrop.net>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

namespace DevShop\Component\GitTools\Command;

use Composer\Command\BaseCommand;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Style\SymfonyStyle;

/**
 * GitSplitCommand runs splitsh-lite script.
 *
 * @author Jon Pugh <jon@thinkdrop.net>
 */
class GitSplitComposerCommand extends BaseCommand
{

  /**
   * @var Symfony\Component\Console\Command\Command;
   */
  protected $command;

  /**
   * @var SymfonyStyle
   */
  protected $io;

  /**
   * @throws LogicException When the command name is empty
   */
  public function __construct()
  {
    parent::__construct('git:split');
  }

  /**
   * {@inheritdoc}
   */
  function isProxyCommand() {
    return TRUE;
  }

  /**
   * {@inheritdoc}
   */
  protected function configure()
  {
    $this->command = new GitSplitCommand();
    $this->command->configure();
    $this
       ->setName($this->command->getName())
       ->setDefinition($this->command->getDefinition())
       ->setDescription($this->command->getDescription())
       ->setHelp($this->command->getHelp())
     ;
  }

  /**
   * {@inheritdoc}
   */
  public function initialize(InputInterface $input, OutputInterface $output) {
    $this->io = new SymfonyStyle($input, $output);
    $this->command->initialize($input, $output);
  }

  /**
   * {@inheritdoc}
   */
  protected function execute(InputInterface $input, OutputInterface $output)
  {
    $this->command->execute($input, $output);
  }
}
