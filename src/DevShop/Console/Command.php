<?php

/*
 * This file is part of DevShop.

 * (c) Jon Pugh <jon@thinkdrop.net>
 *
 * Originally copied from Composer.
 *
 * Thanks to:
 *     Nils Adermann <naderman@naderman.de>
 *     Jordi Boggiano <j.boggiano@seld.be>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

namespace DevShop\Console;

use DevShop\DevShop;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Command\Command as BaseCommand;

/**
 * Base class for DevShop commands
 *
 * @author Jon Pugh <jon@thinkdrop.net>
 */
abstract class Command extends BaseCommand
{
  /**
   * @var DevShop
   */
  private $devshop;

  /**
   * @param  bool $required
   * @param  bool $disablePlugins
   * @throws \RuntimeException
   * @return DevShop
   */
  public function getDevShop($required = true, $disablePlugins = false)
  {
    return $this->devshop;
  }

  /**
   * Simple helper to output something inside an ANSI box.
   *
   * @param \Symfony\Component\Console\Output\OutputInterface $output
   * @param $title
   * @param int $width
   */
  public function announce(OutputInterface $output, $title, $width = 66)
  {
    $title_characters = strlen($title);

    if ($title_characters > $width) {
      $width = $title_characters;
    }

    $remainder = $width - $title_characters;
    $divider = floor($remainder / 2);

    $left_pad = str_repeat(" ", floor($divider));
    $right_pad = str_repeat(" ", ceil($divider));
    $bars = str_repeat("═", $width);

    if ($divider != $remainder / 2) {
      $right_pad .= ' ';
    }


    $output->writeln(
      "╔{$bars}╗"
    );
    $output->writeln("║{$left_pad}{$title}{$right_pad}║");
    $output->writeln(
      "╚{$bars}╝"
    );
  }
}
