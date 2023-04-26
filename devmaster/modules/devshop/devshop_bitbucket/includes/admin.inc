<?php

/**
 * Page callback for "get all repos" link.
 */
function devshop_bitbucket_get_repositories_page() {

  devshop_bitbucket_refresh_repositories();
  drupal_goto('admin/hosting');
}

/**
 * Get a list of all repos a user can access.
 */
function devshop_bitbucket_refresh_repositories() {

  try {
    $client = devshop_bitbucket_client();
    $api = $client->repositories();
    $params = array(
      'role' => 'member',
      'pagelen' => 1000,
    );
    $repos = $api->list($params);
    foreach ($repos['values'] as $repo) {
      $available_repos[$repo['full_name']] = array(
        'url'  => $repo['links']['clone'][1]['href'],
        'org' => $repo['workspace']['slug'],
      );
    }
    
    $count = count($available_repos);
    $r = array(
      '@count' => $count,
    );

    variable_set('devshop_bitbucket_all_repositories', $available_repos);
    drupal_set_message(t('Found @count_string.', array(
      '@count_string' => format_plural($count, '1 repository', t('@count repositories', $r)),
    )));
  }
  catch (\Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
    drupal_set_message('Code ' . $e->getCode(), 'error');
  }
}
