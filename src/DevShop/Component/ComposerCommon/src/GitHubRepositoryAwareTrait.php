<?php

namespace DevShop\Component\Common;

trait GitHubRepositoryAwareTrait
{
    /**
     * @var string
     */
    protected $githubRepoUrlNormalized = NULL;

    /**
     * @var string
     */
    protected $githubRepoOwner = NULL;

    /**
     * @var string
     */
    protected $githubRepoName = NULL;

    /**
     * @param string $repo_url The URL of the remote repository.
     *
     * @return $this
     */
    public function setGitHubRepo(string $repo_url)
    {
        // Normalize the possible git URLs to https.
        $this->githubRepoUrlNormalized = strtr($repo_url, array(
          'git@github.com:' => 'http://github.com/',
          'git://' => 'http://',
          'ssh://' => 'http://',
          '.git' => '',
        ));

        // Extract github repo owner and name
        $parts = explode('/', parse_url($this->githubRepoUrlNormalized, PHP_URL_PATH));
        $this->githubRepoOwner = $parts[1];
        $this->githubRepoName = $parts[2];

        return $this;
    }

    /**
     * @return string
     */
    public function getRepoOwner() {
        return $this->githubRepoOwner;
    }

    /**
     * @return string
     */
    public function getRepoName() {
        return $this->githubRepoName;
    }

}
