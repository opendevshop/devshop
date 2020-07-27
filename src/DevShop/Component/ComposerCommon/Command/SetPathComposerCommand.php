<?php

/*
 * This file is part of the DevShop package.
 *
 * (c) Jon Pugh <jon@thinkdrop.net>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

namespace DevShop\Component\ComposerCommon\Command;

use Composer\Command\BaseCommand;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Style\SymfonyStyle;

/**
 * SetPath prints out a new PATH variable that includes the composer bin path.
 *
 * @author Jon Pugh <jon@thinkdrop.net>
 */
class SetPathComposerCommand extends BaseCommand
{

  /**
   * @throws LogicException When the command name is empty
   */
  public function __construct()
  {
    parent::__construct('path:set');
  }

  /**
   * {@inheritdoc}
   */
  protected function execute(InputInterface $input, OutputInterface $output)
  {
    $this->command->execute($input, $output);
  }
}
