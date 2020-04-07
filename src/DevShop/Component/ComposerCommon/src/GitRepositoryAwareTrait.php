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
        $this->repository = $repository;

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
