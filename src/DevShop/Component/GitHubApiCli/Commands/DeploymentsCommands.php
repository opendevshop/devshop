<?php

namespace DevShop\Component\GitHubApiCli\Commands;

use DevShop\Component\Common\GitHubRepositoryAwareTrait;
use DevShop\Component\GitHubApiCli\GitHubApiCli;
use DevShop\Component\Common\GitRepositoryAwareTrait;

class DeploymentsCommands extends \Robo\Tasks
{

    use GitHubRepositoryAwareTrait;

    /**
     * @var \DevShop\Component\GitHubApiCli\GitHubApiCli
     */
    protected $cli;

    /**
     * GitHubCommands constructor.
     */
    public function __construct()
    {
        $this->cli = new GitHubApiCli();

        // Make this class aware of it's repo.
        $this->setGitHubRepo();

    }

    /**
     * Start a deployment
     * 1. Create a Deployment and a Deployment status.
     * github api deployment create opdendevshop devshop -p ref=component/github-cli -p description='COMMAND LINE DEPLOY!' -p environment=localhost -p required_contexts=
     */
    public function deploymentStart($opts = [
      'description' => null,
      'environment' => null,
      'required_contexts' => [],
      'ref' => null,
    ]) {

        $this->io()->section('Start Deployment');
        $this->io()->table(["Repo Information"], [
          ['Current Branch', $this->getRepository()->getCurrentBranch()],
          ['Current Remote', "Fetch: " . current($this->getRepository()->getCurrentRemote())['fetch']],
          ['',               "Push: " . current($this->getRepository()->getCurrentRemote())['push']],
          ['Current Commit', $this->getRepository()->getCurrentCommit()],
        ]);

        $this->io()->table(["GitHub Repo Information"], [
          ['GitHub Repo Owner', $this->getRepoOwner()],
          ['GitHub Repo Name', $this->getRepoName()],
        ]);

        $params = [
          'ref' => $opts['ref']?: $this->getRepository()->getCurrentCommit(),
          'description' => $opts['description'],
          'environment' => $opts['environment'],
          'required_contexts' => $opts['required_contexts'],
        ];

        print_r($params);

        if ($this->confirm("Start deployment with the above params?")) {
            $this->cli->api('deployments')->create($this->getRepoOwner(), $this->getRepoName(), $params);
        }
        else {
            throw new \Exception('Deployment cancelled.');
        }
    }

    /**
     * Update deployment information.
     */
    public function deploymentUpdate() {
    }

    /**
     * End a deployment
     */
    public function deploymentEnd() {

    }

    /**
     * Delete a deployment
     */
    public function deploymentDelete() {

    }
}
