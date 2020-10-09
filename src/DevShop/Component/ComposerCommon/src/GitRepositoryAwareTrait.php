<?php

namespace DevShop\Component\Common;

use TQ\Git\Repository\Repository;
use TQ\Vcs\Cli\CallResult;

trait GitRepositoryAwareTrait
{
    /**
     * @var GitRepository
     */
    protected $repository = NULL;

    /**
     * @param GitRepository $repository If left empty, the directory this package is installed in will be used.
     *
     * @return $this
     */
    public function setRepository(GitRepository $repository = NULL)
    {
        if ($repository) {
            $this->repository = $repository;
        }
        else {
            $this->repository = GitRepository::open(__DIR__);
        }

        return $this;
    }

    /**
     * @return GitRepository
     */
    public function getRepository()
    {
        if (!$this->repository){
            $this->setRepository();
        }
        return $this->repository;
    }
}
