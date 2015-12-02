<?php

/**
 * Implementation of hook_drush_command().
 * Provides provision commands for all devshop tasks.
 */
function aegir_download_drush_command()
{
    $items['provision-download'] = array(
        'description' => 'Downloads drupal modules and themes, and optionally commits them to git.',
        'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
        'options' => array(
            'packages' => 'This list of modules and themes to download.',
            'commit' => 'Commit the downloaded code to git.',
            'message' => 'The message to use when committing.',
            'update' => 'Run update.php after the download.',
            'test' => 'Queue a test run after the download.',
        ),
        'arguments' => array(
        ),
        'examples' => array(
            'drush @env.project.domain.com provision-download views ctools --update --commit --message="the usuals"' => 'Downloads views & ctools, commits them to the repo with a message, and run drush updb.',
        ),
        'aliases' => array('pdl'),
    );
    return $items;
}

/**
 * Implements the provision-git-pull command.
 */
function drush_aegir_download_provision_download() {
    $packages = drush_get_option('packages');

    if (is_array($packages)) {
        $packages = implode($packages);
    }

    // No modules, no more.
    if (empty($packages)) {
        return drush_set_error('No packages defined.');
    }

    // Run `drush dl $modules`
    $target = d()->name;
    $cmd = "drush $target dl $packages -y";
    drush_log("[AEGIR] Running $cmd", 'ok');
    drush_log($cmd, 'devshop_command');

    if (drush_shell_exec($cmd)) {
        $output = drush_shell_exec_output();
        foreach ($output as $line) {
            drush_log($line, 'devshop_ok');
        }
    }
    else {
        $output = drush_shell_exec_output();
        foreach ($output as $line) {
            drush_log("[DRUSH] $line", 'ok');
        }
        drush_log(dt('drush_shell_exec() returned FALSE, but We will keep going.'), 'ok');
    }
}
//
//function aegir_download_drush_pm_post_download($project, $release)
//{
//    $alias = d()->name;
//    $uri = d()->uri;
//    $project = d()->project;
//    $path_to_project = $project['full_project_path'];
//
//    // Bail if there is no URI or drush alias.
//    if (empty($uri) || empty($alias) || empty($project)) {
//        return;
//    }
//
//    // Bail if there is no repo
//    if (!provision_git_is_repo($path_to_project)) {
//        return;
//    }
//
//    // Invoke provision-git-add to add any new files to the index
//
//    // Generate commit message.
//    $message = array();
//    $message[] = "Downloaded '" . $project['name'] . "'" . " to http://$uri";
//    if (drush_get_option('message')){
//        $message[] = drush_get_option('message');
//    }
//    $message = implode("\n", $message);
//
//    $data = array(
//        'message' => $message,
//    );
//    provision_backend_invoke($alias, 'provision-git-add',
//        array($path_to_project));
//
//    // Invoke provision-git-commit
//    provision_backend_invoke($alias, 'provision-git-commit', array(), $data);
//
//    // Push, unless the user doesn't want to.
//    if (drush_get_option('push', TRUE)){
//        provision_backend_invoke($alias, 'provision-git-push');
//    }
//
//    // Revert, if the user wants.
//    if (drush_get_option('revert')){
//        provision_backend_invoke($alias, 'provision-git-reset');
//    }
//}