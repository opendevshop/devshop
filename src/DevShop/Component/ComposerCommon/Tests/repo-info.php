#!/usr/bin/env php
<?php


function includeIfExists(string $file): bool
{
    return file_exists($file) && include $file;
}

if (
  !includeIfExists(__DIR__ . '/../../../autoload.php') &&
  !includeIfExists(__DIR__ . '/../vendor/autoload.php') &&
  !includeIfExists(__DIR__ . '/../../../../../vendor/autoload.php')
) {
    fwrite(STDERR, 'Install dependencies using Composer.'.PHP_EOL);
    exit(1);
}

use DevShop\Component\Common\GitRepositoryAwareTrait;
use DevShop\Component\Common\GitHubRepositoryAwareTrait;
use Symfony\Component\Console\Output\ConsoleOutput;
use Robo\Common\OutputAwareTrait;
use Robo\Common\IO;

class repoInfo {

    use GitRepositoryAwareTrait;
    use GitHubRepositoryAwareTrait;
    use IO;

    function __construct()
    {
        // Set repository to the current git checkout.
        $this->setRepository();

        // Set the GitHub Repo from the git remote.
        $this->getRepository()->getCurrentRemote();
        $this->setGitHubRepo($this->getRepository()->getCurrentRemote()['origin']['fetch']);

        // Get a ConsoleOutput object so we can look nice.
        $this->setOutput(new ConsoleOutput());
    }

    function printBranches() {
        $this->io()->writeln([
          "<comment>Package:</comment> devshop/composer-common",
          "<comment>Class:</comment> GitRepositoryAwareTrait",
        ]);
        $this->io()->block("This is an Example Class, used for Testing the GitRepositoryAwareTrait.");

        $this->io()->table(["Repo Information"], [
          ['Current Branch', $this->getRepository()->getCurrentBranch()],
          ['Current Remote', "Fetch:" . current($this->getRepository()->getCurrentRemote())['fetch']],
          ['',               "Push:" . current($this->getRepository()->getCurrentRemote())['push']],
          ['Current Commit', $this->getRepository()->getCurrentCommit()],
        ]);

        // Show all branches
        foreach ($this->getRepository()->getBranches() as $branch) {
            $rows[] = [$branch];
        }
        $this->io()->table(["Repo Branches"], $rows);

        $this->io()->table(["GitHub Repo Information"], [
          ['GitHub Repo Owner', $this->getRepoOwner()],
          ['GitHub Repo Name', $this->getRepoName()],
        ]);

    }
}

$repoInfo = new repoInfo();
$repoInfo->printBranches();
