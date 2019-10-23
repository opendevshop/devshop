<?php


/**
 * Implements hook_post_hosting_TASKTYPE_task().
 *
 * After an install, runs user-create.
 */
function devshop_extra_users_post_hosting_install_task($task, $data) {

  drush_log('Checking for extra users...', 'ok');

  if ($task->ref->type == 'site' && isset($task->ref->environment->settings->manager_email)) {

    // Create an extra user.
    $email = $task->ref->environment->settings->manager_email;
    $password = provision_password();
    $arguments = array('name' => $email);
    $data = array(
      'mail' => $email,
      'password' => $password,
    );

    drush_log('Manager email is found! running user-create...', 'ok');
    provision_backend_invoke($task->ref->title, 'user-create', $arguments, $data);

    drush_log(dt('User !user has been created with password !password', array(
      '!user' => $email,
      '!password' => $password,
    )), 'ok');

    drush_log('No email has been sent! Please notify your new user.');
  }
}
