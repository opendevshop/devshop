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

use Symfony\Component\Process\Process;
use Symfony\Component\Process\Exception\ProcessFailedException;
use Github\Exception\RuntimeException;

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
   * @var InputInterface
   */
  public $input;

  /**
   * @var OutputInterface
   */
  public $output;

  /**
   * @var Process
   * Process
   */
  protected $process = NULL;

  /**
   * @param \Symfony\Component\Console\Input\InputInterface $input
   * @param \Symfony\Component\Console\Output\OutputInterface $output
   */
  protected function execute(InputInterface $input, OutputInterface $output)
  {
    $this->input = $input;
    $this->output = $output;
  }

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
   * Used instead of Symfony\Component\Process\Process so we can easily mock it.
   *
   * This returns either an instantiated Symfony\Component\Process\Process or a mock object.
   * @param $commandline
   * @param null $cwd
   * @param array $env
   * @param null $input
   * @param int $timeout
   * @param array $options
   * @return Process
   *
   * @see Symfony\Component\Process\Process
   *
   *  @author Eric Duran <eric@ericduran.io>
   */
  public function getProcess($commandline, $cwd = null, array $env = null, $input = null, $timeout = 60, array $options = array()) {
    if ($this->process === NULL) {
      // @codeCoverageIgnoreStart
      // We ignore this since we mock it.
      return new Process($commandline, $cwd, $env, $input, $timeout, $options);
      // @codeCoverageIgnoreEnd
    }
    return $this->process;
  }

  /**
   * Helper for running processes.
   *
   * @param \Symfony\Component\Process\Process $process
   */
  public function runProcess(Process $process) {

    try {
      $process->mustRun(function ($type, $buffer) {
        if (Process::ERR === $type) {
          echo $buffer;
        } else {
          echo $buffer;
        }
      });
      return TRUE;
    } catch (ProcessFailedException $e) {
      $this->output->writeln('<error>' . $e->getMessage() . '</error>');
      return FALSE;
    }
  }

  /**
   * Simple helper to output something inside an ANSI box.
   *
   * @param \Symfony\Component\Console\Output\OutputInterface $output
   * @param $title
   * @param int $width
   */
  public function announce($title, $width = 45)
  {
    $output = $this->output;
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

  public function checkForRoot() {
      // Check current user is root
    $pwu_data = posix_getpwuid(posix_geteuid());
    if ($pwu_data['name'] != 'root') {
      $this->output->writeln('<error>WARNING:</error> You must run this command as the root user.');
      $this->output->writeln('Run "sudo devshop upgrade" to run as root.');
      $this->output->writeln('<fg=red>Installation aborted.</>');
      $this->output->writeln('');
      return;
    }
  }

  /**
   * Validates version string against GitHub branches or tags.
   *
   * @param $version
   * @throws \Exception
   */
  public function checkVersion($version) {
    if (empty($version)) {
      return FALSE;
    }

    $client = new \Github\Client();

    try {
      $ref = $client->getHttpClient()->get('repos/opendevshop/devshop/git/refs/heads/' . $version);
      $branch_found = TRUE;
    }
    catch (RuntimeException $e) {
      $branch_found = FALSE;
    }

    try {
      $ref = $client->getHttpClient()->get('repos/opendevshop/devshop/git/refs/tags/' . $version);
      $tag_found = TRUE;
    }
    catch (RuntimeException $e) {
      $tag_found = FALSE;
    }

    // Detect GitHub limit issues and just pass.
    if (strpos($e->getMessage(), 'You have reached GitHub hour limit! Actual limit is:') === 0) {
      return TRUE;
    }

    // If we don't find a branch or tag, throw an exception
    if (!$branch_found && !$tag_found) {
      throw new \Exception("An exception was thrown when trying to find a branch or tag named {$version}:" . $e->getCode() . ' ' . $e->getMessage());
    }

    // If no exceptions were thrown, return TRUE.
    return TRUE;
  }
}
