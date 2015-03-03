<?php

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

  return $node->test_results_formatted;
}