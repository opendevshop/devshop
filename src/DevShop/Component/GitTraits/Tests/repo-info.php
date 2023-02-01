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

    use GitHubRepositoryAwareTrait;

    function __construct()
    {
        // Set repository to the current git checkout.
        $this->setRepository();

        // Set the GitHub Repo from the git remote.
        $this->getRepository()->getCurrentRemote();
        $this->setGitHubRepo($this->getRepository()->getCurrentRemote()['origin']['fetch']);

    }

    function printBranches() {

        $output = [
                "Repo Information",
                'Current Branch:' . $this->getRepository()->getCurrentBranch(),
                'Current Remote',
                "    Fetch:" . current($this->getRepository()->getCurrentRemote())['fetch'],
                "    Push:" . current($this->getRepository()->getCurrentRemote())['push'],
                'Current Commit: ' . $this->getRepository()->getCurrentCommit(),
        ];
        print_r($output);

        print_r($this->getRepository()->getBranches());

        print_r([
          "GitHub Repo Information",
          'GitHub Repo Owner: ' . $this->getRepoOwner(),
          'GitHub Repo Name: ' . $this->getRepoName(),
        ]);
    }
}

$repoInfo = new repoInfo();
$repoInfo->printBranches();
