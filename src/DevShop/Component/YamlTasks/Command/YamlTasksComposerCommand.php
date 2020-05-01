<?php

/*
 * This file is part of the DevShop package.
 *
 * (c) Jon Pugh <jon@thinkdrop.net>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

namespace DevShop\Component\YamlTasks\Command;

use Composer\Command\BaseCommand;
use Symfony\Component\Console\Command\Command;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Style\SymfonyStyle;

/**
 * YamlTasksComposerCommand runs the commands in a tasks.yml file.
 *
 * @author Jon Pugh <jon@thinkdrop.net>
 */
class YamlTasksComposerCommand extends BaseCommand
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
   * @var \Composer\Composer
   */
    protected $composer;

  /**
   * @throws LogicException When the command name is empty
   */
    public function __construct()
    {
        parent::__construct();
    }

  /**
   * {@inheritdoc}
   */
    public function isProxyCommand()
    {
        return true;
    }

  /**
   * {@inheritdoc}
   */
    protected function configure()
    {
        $this->command = new YamlTasksConsoleCommand();
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
    public function initialize(InputInterface $input, OutputInterface $output)
    {
        $this->composer = $this->getComposer();
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
