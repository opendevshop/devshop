<?php

/*
 * This file is part of the DevShop package.
 *
 * (c) Jon Pugh <jon@thinkdrop.net>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

namespace DevShop\Component\GitSplit\Command;

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
   * @var \Composer\Composer
   */
  protected $composer;

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
    $this->command = new GitSplitConsoleCommand();
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
    $this->composer = $this->getComposer();
    $this->io = new SymfonyStyle($input, $output);

    // If no repos found anywhere, throw an error.
    if (empty($input->getOption('repo')) && empty($this->getComposer()->getPackage()->getExtra()['git-split']['repos'])) {
      throw new \LogicException('No repos found in composer.json "extras.git-split" section and there was no --repo option. Nothing to do.');
    }
    // If CLI --repo option was not used, and there are repos in the composer.json file, use those.
    elseif (empty($input->getOption('repo')) && !empty($this->getComposer()->getPackage()->getExtra()['git-split']['repos'])) {
      // Set the repo option with the data from composer.json.
      // Reformat repo_options in a format $input->setOption() expects them.
      foreach ($this->getComposer()->getPackage()->getExtra()['git-split']['repos'] as $path => $repo) {
        $repo_options[] = "{$path}={$repo}";
      }
      $input->setOption('repo', $repo_options);
    }

    // Initialize the Console command with input output from this command.
    $this->command->initialize($input, $output);
  }

  /**
   * {@inheritdoc}
   */
  protected function execute(InputInterface $input, OutputInterface $output)
  {
    return $this->command->execute($input, $output);
  }
}
