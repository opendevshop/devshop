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
use Symfony\Component\Console\Question\Question;

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

  /**
   * Helper to ask a question only if a default argument is not present.
   *
   * @param InputInterface  $input
   * @param OutputInterface $output
   * @param Question        $question
   *                                  A Question object
   * @param $argument_name
   *   Name of the argument or option to default to.
   * @param string $type
   *                     Either "argument" (default) or "option"
   *
   * @return mixed
   *               The value derived from either the argument/option or the value.
   */
  public function getAnswer(InputInterface $input, OutputInterface $output, Question $question, $argument_name, $type = 'argument', $required = FALSE)
  {
    $helper = $this->getHelper('question');

    if ($type == 'argument') {
      $value = $input->getArgument($argument_name);
    } elseif ($type == 'option') {
      $value = $input->getOption($argument_name);
    }

    if (empty($value)) {

      if ($required) {
        while (empty($value)) {
          $value = $helper->ask($input, $output, $question);
        }
      }
      else {
        $value = $helper->ask($input, $output, $question);
      }
    }

    return $value;
  }

  /**
   * Helper to get the latest Version.
   * @return mixed
   * @throws \Exception
   */
  public function getLatestVersion() {

    // Lookup latest version.
    $client = new \Github\Client();
    $release = $client->repositories()->releases()->latest('opendevshop', 'devshop');

    // Make sure we got the release info
    if (empty($release)) {
      return NULL;
    }
    return $release['tag_name'];
  }
}
