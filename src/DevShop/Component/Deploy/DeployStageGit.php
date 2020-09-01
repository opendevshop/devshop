<?php

namespace DevShop\Component\Deploy;

use DevShop\Component\Common\GitRepository;
use DevShop\Component\Deploy\DeployStage;
use DevShop\Component\Deploy\DeployStageInterface;
use Symfony\Component\Console\Exception\RuntimeException;
use TQ\Git\Repository\Repository;

/**
 * Class DeployStageGit
 *
 * @package DevShop\Component\Deploy\Stage
 *
 */
class DeployStageGit extends DeployStage {

    /**
     * @inheritdoc
     */
    public $name = 'git';

    /**
     * @var string The desired git reference.
     */
    private $gitReference = null;

    /**
     * @inheritdoc
     */
    public function __construct($name, $command, GitRepository $repository, Deploy $deploy)
    {
        parent::__construct('git', $command, $repository, $deploy);

        // If git reference was specified, use it.
        if ($this->deploy->getOption('git_reference')) {
            $this->gitReference = $this->deploy->getOption('git_reference');
            $this->command = "git reset --hard origin/{$this->gitReference}";
        }
        // If git reference was not specified, infer it from the current git ref.
        // If repository is in a "detached" state, its checked out by tag or SHA. Nothing to do here.
        elseif ($repository->isDetached()) {
            $this->gitReference = $repository->getCurrentCommit();
            $this->command = "git log -1";
        }
        else {
            $this->gitReference = $repository->getCurrentBranch();
            $this->command = "git reset --hard origin/{$this->gitReference}";
        }
    }

    /**
     * @inheritdoc
     */
    public function runStage() {

        // Throw exception if git local is ahead, so remote reset does not orphan the commits.
        if ($this->getRepository()->isAhead()) {
          throw new RuntimeException('Local git repo has commits not pushed to the remote. Cancelling deploy to avoid losing commits. Run "git push" or use deploy command option "--option=git_reset=1" to force the repo back to the remote SHA or "--skip-git" to skip the git stage and run the rest of the deploy.');
        }

        // If repository working copy changes exist, abort unless git_reset was set.
        if ($this->getRepository()->isDirty() && !$this->deploy->getOption('git_reset')) {
            throw new RuntimeException('Git repository working directory is dirty. Commit or clean changes, or use deploy command option "--option=git_reset=1"');
        }

        // @TODO: continue and warn if git_reset is true.
        // @TODO: Run "git fetch --all" as a pre-deploy stage.

        parent::runStage();
    }
}