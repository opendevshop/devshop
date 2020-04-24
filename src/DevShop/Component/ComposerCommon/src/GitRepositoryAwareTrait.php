<?php

namespace DevShop\Component\Common;

use TQ\Git\Repository\Repository;
use TQ\Vcs\Cli\CallResult;

trait GitRepositoryAwareTrait
{
    /**
     * @var Repository
     */
    protected $repository = NULL;

    /**
     * @param Repository $repository If left empty, the current working directory will be used.
     *
     * @return $this
     */
    public function setRepository(Repository $repository = NULL)
    {
        if ($repository) {
            $this->repository = $repository;
        }
        else {
            $this->repository = Repository::open(getcwd());
        }

        return $this;
    }

    /**
     * @return Repository
     */
    public function getRepository()
    {
        if (!$this->repository){
            $this->setRepository();
        }
        return $this->repository;
    }

    /**
     * Run a git command in the repository directory.
     *
     * @param   string  $method             The VCS command, e.g. show, commit or add
     * @param   array   $arguments          The command arguments.
     * @return  CallResult
     * @example $this->callGit('status', array('--short'))
     */
    public function callGit($command, $arguments = array(), $message = "Git command failed.") {
      /** @var $result CallResult */
      $result = $this->getRepository()->getGit()->{$command}($this->getRepository()->getRepositoryPath(), $arguments);
      $result->assertSuccess($message);
      return $result;
    }
}
