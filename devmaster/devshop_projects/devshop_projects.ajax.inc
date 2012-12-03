<?php

/**
 * Totally stolen from poll_choice, of course.
 */
function devshop_projects_ajax($action = '', $argument = ''){
  $form_state = array(
    'storage' => NULL,
    'submitted' => FALSE,
  );
  $form_build_id = $_POST['form_build_id'];
  
  // Get the form from the cache.
  $form = form_get_cache($form_build_id, $form_state);
  $args = $form['#parameters'];
  $form_id = array_shift($args);
  
  // We will run some of the submit handlers so we need to disable redirecting.
  $form['#redirect'] = FALSE;
  
  // We need to process the form, prepare for that by setting a few internals
  // variables.
  $form['#post'] = $_POST;
  $form['#programmed'] = FALSE;
  $form_state['post'] = $_POST;
  
  // Build, validate and if possible, submit the form.
  // drupal_process_form($form_id, $form, $form_state);
  
  // This call recreates the form relying solely on the form_state that the
  // drupal_process_form set up.
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);
  
  //// Render the new output.
  //$choice_form = $form['choice_wrapper']['choice'];
  //unset($choice_form['#prefix'], $choice_form['#suffix']); // Prevent duplicate wrappers.
  //$output = theme('status_messages') . drupal_render($choice_form);
  
  
  // Get refs
  $git_url =  $form_state['post']['git_url'];
  $command = "git-ls-remote $git_url";
  $exec_output = shell_exec($command);
  
  $lines = explode("\n", $exec_output);
  $output .= print_r($lines, 1);
  $branches = array();
  foreach ($lines as $line){
    list($commit, $ref_string) = explode("	", $line);
    list($i, $type, $name) = explode('/', $ref_string);
    
    if ($type == 'heads'){
      $branches[$name] = $name;
    }
  }
  
  $form['branch']['#options'] = $branches;
  
  //$output .= $command.$exec_output;
  $output .= theme('status_messages');
  $output .= drupal_render($form['branch']);
//  $output .= print_r($form_state, 1);
  
  drupal_json(array('status' => TRUE, 'data' => $output));
}