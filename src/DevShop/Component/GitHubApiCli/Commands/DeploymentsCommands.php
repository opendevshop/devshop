<?php

namespace DevShop\Component\GitHubApiCli\Commands;

use DevShop\Component\Common\GitHubRepositoryAwareTrait;
use DevShop\Component\GitHubApiCli\GitHubApiCli;
use DevShop\Component\Common\GitRepositoryAwareTrait;
use TQ\Vcs\Cli\CallResult;

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
     * Start (create) a deployment.
     * @see https://developer.github.com/v3/repos/deployments/#create-a-deployment
     *
     * @option ref The ref to deploy. This can be a branch, tag, or SHA. Set to the string "branch", "tag" or "sha" to automatically read the branch, tag, or SHA from the repository
     * @option task	Specifies a task to execute (e.g., deploy or deploy:migrations). Default: deploy
     * @option auto_merge	Attempts to automatically merge the default branch into the requested ref, if it's behind the default branch. Default: true
     * @option required_contexts	The status contexts to verify against commit status checks. If you omit this parameter, GitHub verifies all unique contexts before creating a deployment. To bypass checking entirely, pass an empty array. Defaults to all unique contexts.
     * @option payload	JSON payload with extra information about the deployment. Default: ""
     * @option environment	Name for the target deployment environment (e.g., production, staging, qa). Default: production
     * @option description	Short description of the deployment. Default: ""
     * @option transient_environment	Specifies if the given environment is specific to the deployment and will no longer exist at some point in the future. Default: false
     * @option production_environment	Specifies if the given environment is one that end-users directly interact with. Default: true when environment is production and false otherwise.
     */
    public function deploymentStart($opts = [
      'ref' => 'sha',
      'task' => 'deploy',
      'auto_merge' => true,
      'required_contexts' => [],
      'payload' => null,
      'environment' => null,
      'description' => null,
      'transient_environment' => false,
      'production_environment' => false,
    ]) {

        $this->io()->section('Start Deployment');
        $info =  [
          ['Branch', $this->getRepository()->getCurrentBranch()],
          ['Remote', "Fetch: " . current($this->getRepository()->getCurrentRemote())['fetch']],
          ['',       "Push: " . current($this->getRepository()->getCurrentRemote())['push']],
          ['Commit', $this->getRepository()->getCurrentCommit()],
        ];

        try {
          $tag = $this->getCurrentTag();
          $info[] = ['Tag', $tag];
        }
        catch (\Exception $e) {
          $this->io()->note('No tag for current SHA found.');
        }

        $info[] = ['GitHub Repo Owner', $this->getRepoOwner()];
        $info[] = ['GitHub Repo Name', $this->getRepoName()];

        $this->io()->table(["Repo Information"], $info);

        $params = [
          'ref' => $this->getRefFromOpt($opts['ref']),
          'description' => $opts['description'],
          'environment' => $opts['environment']?: $this->getEnvironmentName(),
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
     * @param $ref string If "branch", "tag", or "sha", ref will be read from
     *   the current git repository. Otherwise, the $ref will be returned.
     *
     * @return string The requested git reference.
     */
    private function getRefFromOpt($ref) {
      switch ($ref) {
        case 'sha': return $this->getRepository()->getCurrentCommit();
        case 'branch': return $this->getRepository()->getCurrentBranch();
        case 'tag': return $this->getCurrentTag();
        default: return $ref;
      }
    }

    /**
     * Returns the name of the current tag, if that is what is checked out.
     *
     * @TODO: Submit a PR to Repository.php
     *
     * @return  string
     */
    private function getCurrentTag()
    {
      /** @var $result CallResult */
      $result = $this->getRepository()->getGit()->{'describe'}($this->getRepository()->getRepositoryPath(), array(
        '--tags',
        '--exact-match'
      ));
      $result->assertSuccess(
        sprintf('Cannot retrieve current tag from "%s"', $this->getRepository()->getRepositoryPath())
      );

      return $result->getStdOut();
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
        $this->io()->success("Deployment status created successfully.");
        $this->io()->comment($deployment_status['url']);
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

    /**
     * Helper to dynamically generate environment names.
     */
    private function getEnvironmentName() {

        // In GitHub Actions:
        if (getenv('GITHUB_EVENT_PATH')) {
            $event = json_decode(file_get_contents(getenv('GITHUB_EVENT_PATH')));
            $pull_request_number = $event->pull_request->number;
            return "pr{$pull_request_number}";
        }
        else {
            return shell_exec('hostname -f');
        }
    }
}
