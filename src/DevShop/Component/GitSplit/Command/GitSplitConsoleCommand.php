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

use DevShop\Component\GitSplit\Splitter;
use Symfony\Component\Console\Exception\InvalidOptionException;
use Symfony\Component\Console\Input\InputInterface;
use Symfony\Component\Console\Input\InputOption;
use Symfony\Component\Console\Output\OutputInterface;
use Symfony\Component\Console\Style\SymfonyStyle;
use Symfony\Component\Console\Command\Command;

/**
 * GitSplitConsoleCommand runs splitsh-lite script.
 *
 * @author Jon Pugh <jon@thinkdrop.net>
 */
class GitSplitConsoleCommand extends Command
{

  private $command;

  /**
   * @var SymfonyStyle
   */
  protected $io;

  /**
   * @var array List of secondary repos, with path as key, repo URL as value.
   */
  protected $gitRepos = [];

  /**
   * {@inheritdoc}
   */
  protected function configure()
  {
    $this
      ->setName('git:split')
      ->setDefinition([
        new InputOption(
          'repo',
          'r',
          InputOption::VALUE_IS_ARRAY | InputOption::VALUE_REQUIRED,
          'The path and repo to split to. Use the format "$PATH=$REPO". Multiple "--repo" options may be used.'
        ),
        new InputOption(
          'progress',
          'p',
          InputOption::VALUE_NONE,
          'Print splitsh-lite progress.'
        ),
      ])
      ->setDescription('Split folders into separate git repos.')
      ->setHelp(<<<'EOF'
The <info>%command.name%</info> is a wrapper for the "splitsh-lite" command. 
It is used to push folders of a git repo into a separate second git repo. 

  <info>php %command.full_name% git:split --repo path/to/component=https://githost.com/split/component.git</info>

You can also output progress from splitsh-lite by using the <comment>--progress</comment> option:

  <info>php %command.full_name% --progress git:split --repo path/to/component=https://githost.com/split/component.git</info>

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

    // Console command only loads options from command line.
    if (!empty($input->getOption('repo'))) {
      foreach ($input->getOption('repo') as &$value) {
        try {
          [$path, $repo] = explode('=', $value);
          $this->gitRepos[$path] = $repo;
        }
        catch (\ErrorException $e) {
          $this->io->note("Invalid --repo option. Use the format PATH=REPO_URL. Unable to parse '$value'.");
        }
      }
    }

    if (!count($this->gitRepos)) {
      throw new InvalidOptionException('No valid repos found.');
    }
  }

  /**
   * {@inheritdoc}
   */
  protected function execute(InputInterface $input, OutputInterface $output)
  {
    $this->io->title('Git Split');
    $pwd = getcwd();
    $this->io->writeln(" Working Directory: $pwd");

    // Output a list of paths to split.
    foreach ($this->gitRepos as $path => &$gitRepo) {
      $rows[] = [$path, $gitRepo];

      // Rewrite git remote URL with GitHub Token so pushes will authenticate with the token.
      // @TODO: Is there a way to do this for SSH urls?
      // @TODO: Figure out why this is needed even though we are using the fregante/setup-git-token@v1 github action.
      if (!empty($_SERVER['GITSPLIT_GITHUB_TOKEN']) && strpos($gitRepo, 'https://') === 0) {
        $gitRepo = str_replace('https://', 'https://$GITSPLIT_GITHUB_TOKEN@', $gitRepo);
      }
    }
    $this->io->table(['Paths to Split', 'Git Repository'], $rows);

    Splitter::install();
    Splitter::splitRepos($this->gitRepos, $input->getOption('progress'));

    # Always return 0 (Success) if we got this far. Splitter::splitRepos() will throw exceptions or exit 1
    return 0;
  }
}
