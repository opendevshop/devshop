<?php

/**
 * @file
 * The Drush integration for server data example.
 */

/**
 * Implements hook_hosting_server_context_options().
 */
function ansible_variables_hosting_server_context_options(&$task) {
  if (isset($task->ref->ansible_variables) && !empty($task->ref->ansible_variables)) {
    $task->context_options['ansible_variables'] = $task->ref->ansible_variables;
  }
  else {
    $task->context_options['ansible_variables'] = 'null';
  }
}

/**
 * Implements hook_drush_context_import().
 */
function ansible_variables_drush_context_import($context, &$node) {
  if ($context->type == 'server') {
    if (isset($context->ansible_variables) && !empty($context->ansible_variables)) {
      $node->ansible_variable_overrides = $context->ansible_variables;
    }
  }

}
