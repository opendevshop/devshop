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

use DevShop\Component\GitTools\Splitter;
use Robo\Common\TaskIO;
use Symfony\Component\Console\Helper\DescriptorHelper;
use Symfony\Component\Console\Input\InputArgument;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Style\SymfonyStyle;
use Composer\Composer\BaseCommand;

/**
 * GitSplitCommand runs splitsh-lite script.
 *
 * @author Jon Pugh <jon@thinkdrop.net>
 */
class GitSplitCommand extends BaseCommand
{

  private $command;

  /**
   * @var SymfonyStyle
   */
  protected $io;

  /**
   * {@inheritdoc}
   */
  protected function configure()
  {
    $this
      ->setName('git:split')
      ->setDefinition([
        new InputArgument('path', InputArgument::OPTIONAL, 'The path to sub repository. If not specified, all paths listed in composer.json config.git-split.repos will be split.'),
        new InputArgument('remote', InputArgument::OPTIONAL, 'The git remote to push to.'),
        new InputOption('progress', null, InputOption::VALUE_NONE, 'Print splitsh-lite progress.'),
      ])
      ->setDescription('Split a folder into a separate git repo.')
      ->setHelp(<<<'EOF'
The <info>%command.name%</info> is a wrapper for the "splitsh-lite" command. 
It is used to push folders of a git repo into a separate second git repo. 

  <info>php %command.full_name% git:split path/to/component https://githost.com/split/component.git</info>

You can also output progress from splitsh-lite by using the <comment>--progress</comment> option:

  <info>php %command.full_name% --progress git:split</info>

See https://github.com/splitsh/lite for more information.
EOF
      )
    ;
  }

  public function setCommand(Command $command)
  {
    $this->command = $command;
  }

  /**
   * {@inheritdoc}
   */
  public function initialize(InputInterface $input, OutputInterface $output) {
    $this->io = new SymfonyStyle($input, $output);

    $this->io->warning('IO Initialized.');
  }

  /**
   * {@inheritdoc}
   */
  protected function execute(InputInterface $input, OutputInterface $output)
  {
    $this->io->title('Git Split');

    $pwd = getcwd();
    $this->io->writeln(["Working Directory: $pwd"]);

    Splitter::installBins();
    // @TODO: Read this from composer config.
    Splitter::splitRepos(self::REPOS, $input->getOption('progress'));

    $this->io->warning('This code is not yet functional!');
    return 1;
  }

  /**
   * @var array The list of folders to split into sub repos.
   * @TODO: Read this from composer config.
   */
  const REPOS = array(
    'devmaster' => 'https://$INPUT_GITHUB_TOKEN@github.com/opendevshop/devmaster.git',
    'roles/opendevshop.apache' => 'https://$INPUT_GITHUB_TOKEN@github.com/opendevshop/ansible-role-apache.git',
    'roles/opendevshop.devmaster' => 'https://$INPUT_GITHUB_TOKEN@github.com/opendevshop/ansible-role-devmaster.git',
    'roles/opendevshop.users' => 'https://$INPUT_GITHUB_TOKEN@github.com/opendevshop/ansible-role-users.git'
  );
}
