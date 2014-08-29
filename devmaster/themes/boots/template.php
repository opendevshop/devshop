<?php

function boots_preprocess_page(&$vars){

  if ($vars['node']) {

    // Removing conflicting scripts.
    // Not sure how this actually works.  Drupal 6 is fun!
    // Thanks, http://drupal.stackexchange.com/questions/5076/remove-every-javascript-except-own-theme-scripts
    drupal_add_js(path_to_theme(). '/js/bootstrap.min.js', 'theme');
    $js = drupal_add_js();
    unset($js['core']);
    unset($js['module']);
    $vars['scripts'] = $js;


    // Set subtitle
    if ($vars['node']->type == 'project'){
      $vars['subtitle'] = t('Project');
    }
  }
}


/**
 *
 * @param $vars
 */
function boots_preprocess_node(&$vars){

  if ($vars['node'] && $vars['node']->type == 'project') {

    // Easy Access
    $node = &$vars['node'];
    $project = $vars['project'] = $vars['node']->project;

    // Live Domain link.
    if ($project->settings->live['live_domain']) {
      $vars['live_domain_url'] =  'http://' . $project->settings->live['live_domain'];
      $vars['live_domain_text'] =  'http://' . $project->settings->live['live_domain'];
    }
    else {
      $vars['live_domain_url'] =  '';
    }

    // @TODO: Detect other web URLs for other git hosts.
    if (strpos($project->git_url, 'github.com') !== FALSE) {
      $url = str_replace('git@github.com:', 'http://github.com/', $project->git_url);
      $vars['github_url'] = $url;
    }

    // Generate branches/tags lists
    $vars['branches_count'] = count($project->settings->git['branches']);
    $vars['tags_count'] = count($project->settings->git['tags']);

    $vars['branches_label'] = format_plural($vars['branches_count'], t('1 Branch'), t('!count Branches', array('!count' => $vars['branches_count'])));

    if ($vars['tags_count']){
      $vars['branches_label'] .= ' &amp; ' . format_plural($vars['tags_count'], t('1 Tag'), t('!count Tags', array('!count' => $vars['tags_count'])));
    }

    // Get available servers
    $vars['web_servers'] = hosting_get_servers('http');
    $vars['db_servers'] = hosting_get_servers('db');


  }
}