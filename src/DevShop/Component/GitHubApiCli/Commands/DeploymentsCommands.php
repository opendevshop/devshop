<?php

namespace DevShop\Component\GitHubApiCli\Commands;

use DevShop\Component\GitHubApiCli\GitHubApiCli;

class DeploymentsCommands extends \Robo\Tasks
{

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
    }

    /**
     * Start a deployment
     * 1. Create a Deployment and a Deployment status.
     * github api deployment create opdendevshop devshop -p ref=component/github-cli -p description='COMMAND LINE DEPLOY!' -p environment=localhost -p required_contexts=
     */
    public function deploymentStart() {
        $this->cli->api('deployments');


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
