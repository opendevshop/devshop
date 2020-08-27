<?php

/*
 * This file is part of the DevShop package.
 *
 * (c) Jon Pugh <jon@thinkdrop.net>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

namespace DevShop\Component\Deploy;

use Composer\Composer;
use Composer\Config;
use DevShop\Component\Common\ComposerRepositoryAwareTrait;
use Robo\Common\OutputAwareTrait;
use Symfony\Component\Console\Command\Command as BaseCommand;
use DevShop\Component\Deploy\DeployStages;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Style\SymfonyStyle;


/**
 * GitSplitConsoleCommand runs splitsh-lite script.
 *
 * @author Jon Pugh <jon@thinkdrop.net>
 */
class DeployCommand extends BaseCommand
{
    use ComposerRepositoryAwareTrait;

    /**
     * @var array default DeployStages
     */
    private $defaultStages = [];

    /**
     * @var SymfonyStyle
     */
    private $io;


  /**
   * {@inheritdoc}
   */
  protected function configure()
  {
    $this
      ->setName('deploy')
      ->setDescription('Run the deploy stages for this application, contingent on environment configuration and .')
      ->setHelp(<<<'EOF'
The <info>%command.name%</info> runs the commands defined in a project's composer.json:extras.deploy.stages
EOF
      )
    ;
    // @TODO Add Build stage options
  }

  /**
   * {@inheritdoc}
   */
  public function initialize(InputInterface $input, OutputInterface $output) {
    // Prepare object.
    $this->io = new SymfonyStyle($input, $output);
    $this->setComposerConfig();
    $this->defaultStages = DeployStages::defaultStages();

    // Load the stages from the current composer project.
    if ($this->io->isVerbose()){
      $this->io->section('Initializing Deploy Command');
      $verbose_rows[] = ['PWD', getenv('PWD')];
      $verbose_rows[] = ['Repository Root', $this->getRepository()->getRepositoryPath()];
      $verbose_rows[] = ['Composer Project', $this->getComposerConfig()->name()];

      $this->io->table(['Debug Information'], $verbose_rows);
    }

  }

  /**
   * Load deploy stage config from composer.json
   */
  public function loadStages() {
      if (empty($this->getComposerConfig()->extra()->deploy)) {
        // @TODO: Allow Fail or Warn
        $this->io->warning("No deploy stages found in composer.json:extra.deploy");
      }
  }


  /**
   * {@inheritdoc}
   */
  protected function execute(InputInterface $input, OutputInterface $output)
  {
    $this->io->title('Deploy');

    // Validate Stages
    $this->loadStages();


    foreach ($this->defaultStages as $name => $description) {
      $rows[] = [
        $name, $description
      ];
    }
    $this->io->table(['Stage', 'Command'], $rows);
    return 0;
  }
}
