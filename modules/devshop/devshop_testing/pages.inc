<?php

require_once __DIR__.'/ansi-to-html/vendor/autoload.php';
use SensioLabs\AnsiConverter\AnsiToHtmlConverter;
use SensioLabs\AnsiConverter\Theme\SolarizedTheme;
use SensioLabs\AnsiConverter\Theme\SolarizedXTermTheme;

/**
 * Callback for devshop_tests/NID/VID
 * @param $nid
 * @param null $vid
 * @return mixed
 */
function devshop_testing_results_status($nid, $vid = NULL) {

  $node = node_load($nid, $vid);

  if ($node->task_status == HOSTING_TASK_PROCESSING || $node->task_status == HOSTING_TASK_QUEUED ) {
    $return = 'TRUE';
  }
  else {
    $return = 'FALSE';
  }

  print $return;
  exit;

}

/**
 * Callback for devshop_tests/NID/VID
 * @param $nid
 * @param null $vid
 * @return mixed
 */
function devshop_testing_results_page($nid, $vid = NULL) {
  $node = node_load($nid, $vid);

  if (empty($vid)) {
    $revisions = node_revision_list($node);

    $header = array(
      t('Test Run'),
      t('Date'),
      t('Status'),
    );
    $rows = array();

    foreach ($revisions as $revision) {
      $node_revision = node_load($nid, $revision->vid);

      $row = array();
      $row[] = l(t('Test Result #!vid', array('!vid' => $revision->vid)), "devshop_tests/$nid/{$revision->vid}");
      $row[] = format_date($revision->timestamp) . '  <small>' . t('!time ago', array('!time' => format_interval(time() - $revision->timestamp, 1))) . '</small>';

      $row[] = _hosting_parse_error_code($node_revision->task_status) . ' ' . t('in !time', array('!time' => format_interval($node->delta, 1)));
      $rows[] = $row;
    }
    return theme('table', $header, $rows, array('class' => 'table'));
  }
  $filename = pathinfo($node->task_args['output_path'], PATHINFO_BASENAME);
  drupal_add_js(drupal_get_path('module', 'devshop_testing') . '/test-reload.js');
  drupal_add_js(array(
    'test_running' => TRUE,
    'test_url' => url('devshop_tests/ansi-to-html/' . $filename),
    'test_status_url' =>  url("devshop_tests/{$node->nid}/{$node->vid}/status"),
  ), 'setting');

  $output ='<div class="results-wrapper">' . $node->test_results_formatted . '</div>';


  if ($node->task_status == HOSTING_TASK_PROCESSING || $node->task_status == HOSTING_TASK_QUEUED ) {
    $output .= '<label class="follow-checkbox btn btn-default"><input type="checkbox" id="follow"> Follow Logs</div>';
    drupal_add_css(drupal_get_path('module', 'devshop_testing') . '/testing.css');
  }

  return $output;
}

/**
 * Page callback for ajax test results.
 * @param $filename
 */
function devshop_testing_results_page_raw($task_dir) {

  $path = $_SERVER["DOCUMENT_ROOT"] .'/'. variable_get('file_directory_path', '') . '/devshop_testing/' . $task_dir;
  $output = '';

  foreach (scandir($path) as $file) {
    // Don't load directories.
    if ($file == '.' || $file == '..') {
      continue;
    }

    // Load all files
    $file_path = $path . '/' . $file;
    if (strpos($file, 'results') === 0) {
      $file_contents = file_get_contents($file_path);
      $output .= $file_contents;
    }
  }
  $output = devshop_testing_results_to_html($output);
  print $output;
}

/**
 * Helper to format test results.
 * @param $ascii
 * @return string
 */
function devshop_testing_results_to_html($ascii) {

  $theme = new SolarizedXTermTheme();
  $converter = new AnsiToHtmlConverter($theme, false);
  $html = $converter->convert($ascii);
  $html = strtr($html, array(
    "\n" => "<br />",
    "  " => "&nbsp; ",
  ));

  $output = "<style> .results { overflow: auto; padding: 10px 15px; font-family: monospace; }" . $theme->asCss() . "</style>";
  $output .= "<div class='results ansi_color_bg_black'>$html</div>";
  return $output;
}