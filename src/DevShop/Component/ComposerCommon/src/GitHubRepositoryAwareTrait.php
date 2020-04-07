<?php

namespace DevShop\Component\Common;

trait GitHubRepositoryAwareTrait
{
    /**
     * @var string
     */
    public $githubRepoUrlNormalized = NULL;

    /**
     * @var string
     */
    public $githubRepoOwner = NULL;

    /**
     * @var string
     */
    public $githubRepoName = NULL;

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
        $parts = explode('/', parse_url($repo_url, PHP_URL_PATH));
        $this->githubRepoOwner = $parts[1];
        $this->githubRepoName = $parts[2];

        return $this;
    }

}
