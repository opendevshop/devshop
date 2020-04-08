<?php

namespace DevShop\Component\Common;

use TQ\Git\Repository\Repository;

trait GitRepositoryAwareTrait
{
    /**
     * @var Repository
     */
    protected $repository = NULL;

    /**
     * @param Repository $repository
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
        return $this->repository;
    }
}
