<?php
/**
 * @file
 * tools.inc
 *
 * Helpful commands for executing on the command line.
 */

/**
 * Helpful Exec
 */
function devshop_exec($command, $output = 'array', $return = TRUE){
  $lines = array();
  drush_log("[DEVSHOP] Running: $command", "ok");
  exec($command, $lines);

  if ($return) {
    return $output == 'string' ? implode("\n", $lines) : $lines;
  }
  else {
    foreach ($lines as $line){
      drush_log("[DEVSHOP] $line", 'ok');
    }
  }
}

/**
 * Helpful Exec to JSon
 */
function devshop_exec_json($command, $return_string = FALSE){
  drush_log("[DEVSHOP] Running: $command");
  drush_shell_exec($command);
  $lines = drush_shell_exec_output();
  $data_string = implode($lines);
  $data_json = json_decode($data_string);

  // Force return string if json did not decode
  if (!is_array($data_json) && !is_object($data_json)){
    return $data_string;
  } else {

    // If asked to return a string, return a string.
    return $return_string? $data_string: $data_json;
  }
}