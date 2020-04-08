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
     * @var string The variable name to use when saving the deployment ID to git config.
     */
    const GIT_CONFIG_DEPLOYMENT_ID_NAME = 'devshop.github.deployment.id';

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
     * Start a deployment.
     * @see https://developer.github.com/v3/repos/deployments/#create-a-deployment
     *
     * Custom command:
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

        if (!$this->input->isInteractive() || $this->confirm("Start deployment with the above params?")) {
            $deployment = $this->cli->api('deployments')->create($this->getRepoOwner(), $this->getRepoName(), $params);

            // @TODO: Create simple get and set methods for git config in a class or trait.
            $git_config = self::GIT_CONFIG_DEPLOYMENT_ID_NAME;

            // @TODO: Import ProcessAwareTrait
            shell_exec("git config --add {$git_config} {$deployment['id']}");

            $this->io()->success("Deployment created successfully: " . $deployment['url']);
            $this->io()->comment("Deployment ID saved to git config: Use 'git config --get {$git_config}' to retrieve it.");
        }
        else {
            throw new \Exception('Deployment cancelled.');
        }
    }

    /**
     * Update deployment state.
     *
     * @see https://developer.github.com/v3/repos/deployments/#create-a-deployment-status
     *
     * @option state The state of the status. Can be one of error, failure, inactive, in_progress, queued pending, or success.
     * @option log_url The target URL to associate with this status.
     * @option description A short description of the status.
     * @option environment Name for the target deployment environment.
     * @option environment_url Sets the URL for accessing your environment.
     * @option auto_inactive Adds a new inactive status to all prior non-transient, non-production environment deployments with the same repository and environment name as the created status's deployment.
     */
    public function deploymentUpdate($deployment_id = null, $opts = [
      'deployment_id' => null,
      'state' => 'queued',
      'log_url' => null,
      'description' => null,
      'environment' => null,
      'environment_url' => null,
      'auto_inactive' => true,
    ]) {

        $this->io()->section('Update Deployment');
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

        $this->say('Looking up latest deployment...');
        $git_config = self::GIT_CONFIG_DEPLOYMENT_ID_NAME;
        $deployment_id = $opts['deployment_id']?: trim(shell_exec("git config --get {$git_config}"));
        if ($deployment_id) {
            $this->io()->comment("Deployment ID found: {$deployment_id}");
        } else {
            throw new \Exception('Unable to find deployment ID in git config. Please specify using --deployment-id');
        }

        // Run updateStatus method to update the deployment.
        $params = $opts;
        $deployment_status = $this->cli->api('deployments')->updateStatus($this->getRepoOwner(), $this->getRepoName(), $deployment_id, $params);
        $this->io()->success("Deployment status created successfully." . $deployment_status['url']);
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
