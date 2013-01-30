<?php

/**
 * @file devshop_live.form.inc DevShop Live forms related hooks and
 * support function include file.
 */


/**
 * Implementation of hook_form_alter().
 */
function devshop_live_form_alter(&$form, &$form_state, $form_id) {
  //Add field live_domain to edit form
  if ($form_id == 'project_node_form') {
    $node = $form['#node'];

    $form['live_domain'] = array(
      '#type' => 'textfield',
      '#title' => t('Live domain'),
      '#description' => t('The live domain. If you still do not know you can input it later.'),
      '#size' => 40,
      '#default_value' => $node->live_domain,
      '#maxlenght' => 255,
      '#weight' => 3,
    ); 
  }

  if ($form_id == 'devshop_project_create_step_settings') {
   $project = $form_state['project'];

    $form['live_domain'] = array(
      '#type' => 'textfield',
      '#title' => t('Live domain'),
      '#description' => t('The live domain. If you still do not know you can input it later.'),
      '#size' => 40,
      '#default_value' => $project->live_domain,
      '#maxlenght' => 255,
    );

    $form['#submit'][] = 'devshop_live_create_form_submit';
  }
}

function devshop_live_create_form_submit($form, &$form_state) {
  $project = &$form_state['project'];

  $project->live_domain = $form_state['values']['live_domain'];
}
