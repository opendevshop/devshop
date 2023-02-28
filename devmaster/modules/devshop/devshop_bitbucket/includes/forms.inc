<?php

/**
 * Implements hook_form_FORM_ID_alter() for site_node_form().
 *
 * Add the list of PRs to the Branch selector.
 */
function devshop_bitbucket_form_site_node_form_alter(&$form, &$form_state, $form_id) {

  $project = $form_state['project'];

  $client = devshop_bitbucket_client();
  
  // Don't try to load PRs if there isn't even a token.
  if (arg(1) == 'add' && $project->git_provider == 'bitbucket'){
    try {
      $prs_data = $client->repositories()
        ->workspaces($project->bitbucket_owner)
        ->pullRequests($project->bitbucket_repo)
        ->list()
      ;

      $prs = $prs_data['values'];
      
      $repo = devshop_bitbucket_client()->repositories()
        ->workspaces($project->bitbucket_owner)
        ->show($project->bitbucket_repo)
      ;
      dsm($repo);
        
    }
    catch (\Exception $e) {
      drupal_set_message(t('Unable to load pull request [Error: %error]', array(
        '%error' => $e->getMessage(),
      )), 'warning');
      return;
    }
    $default_branch = 'master';

    // @TODO: Use the new function devshop_github_pr_options().
    $pr_options = array();
    $existing_options = $form['git_ref']['#options'];

    if (count($prs)) {
      foreach ($prs as $pr) {
        $pr_description = implode(' - ', array(
          "PR {$pr['id']}",
          $pr['source']['branch']['name'],
          $pr['author']['nickname'],
          $pr['title'],
        ));
        $option_name = $pr['source']['branch']['name'];
        $pr_options['Pull Requests'][$option_name] = $pr_description;
        $form_state['pull_requests'][$option_name] = $pr;
      }

      $form['git_ref']['#title'] = t('Branch, Tag, or Pull Request');
    }

    $default_branch_option = array(
      t('Default Branch') => array(
        $default_branch => $default_branch . ' (' . t('Default Branch') . ')',
      ),
    );

    unset($existing_options[t('Branches')][$default_branch]);

    $form['git_ref']['#options'] = array_merge($default_branch_option, $pr_options, $existing_options);
    $form['git_ref']['#default_value'] = $default_branch;

    $form['git_ref']['#element_validate'] = array(
      'devshop_bitbucket_form_site_node_form_validate_pr'
    );
  }
}

/**
 * @param $element
 * @param $form_state
 */
function devshop_bitbucket_form_site_node_form_validate_pr($element, &$form_state, $form)
{
  
  if (!empty($form_state['pull_requests'][$element['#value']])) {
    $pr_values = $form_state['pull_requests'][$element['#value']];
    
    $form_state['values']['settings']['pull_request'] = $pr = convert_to_object($pr_values);
//    dsm($pr, 'pr found');
    

//    // If PR has different remote URL, set it.
//    if ($pr->head->repo->fork) {
//      // Try to match project URL scheme
//      if (strpos($form_state['project']->git_url, 'https://') === 0) {
//        $remote_url = $pr->head->repo->clone_url;
//      }
//      elseif (strpos($form_state['project']->git_url, 'git://') === 0){
//        $remote_url = $pr->head->repo->git_url;
//      }
//      elseif (strpos($form_state['project']->git_url, 'git@') === 0){
//        $remote_url = $pr->head->repo->ssh_url;
//      }
//      else {
//        $remote_url = $pr->head->repo->clone_url;
//      }
//
//      // Recreate the platform node, with the new git git_remote.
//      $platform_node = devshop_prepare_platform_node($form_state['project'], $form_state['environment_name'], $form_state['values']['git_ref'], $form_state['values']['web_server'], $form_state['values']['git']['git_remote']);
//      $platform_node->no_verify = TRUE;
//      form_set_value($form['platform_node'], $platform_node, $form_state);
//    }
  }
}
