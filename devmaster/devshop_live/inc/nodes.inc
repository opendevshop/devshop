<?php

/**
 * @file devshop_live.node.inc
 *
 * DevShop Live Node related hooks and support functions.
 *
 */

/**
 * Implementation of hook_nodeapi().
 */
function devshop_live_nodeapi(&$node, $op, $a3 = NULL, $a4 = NULL) {
  if ($node->type == 'project') {
    if (in_array($op, array('insert', 'update'))) {
      //Load all information
      $node_p = node_load($node->nid);

      if ($node->live_domain && !isset($node_p->project_objects['site'])) {
        $live_nid = array_search('live', $node_p->project_objects['site']);

        if ($live_nid) {
          $live_node = node_load($live_nid);
          devshop_live_add_domain($live_node, $node->live_domain);
        }
      }
    }
    if ($op == 'view' && !empty($node->live_domain)) {
      $url = 'http://' . $node->live_domain;

      $node->content['info']['live_domain'] = array(
        '#type' => 'item',
        '#title' => t('Live Site'),
        '#value' => l($url, $url, array('attributes' => array('target' => '_blank'))),
        '#weight' => -9,
      );
    }
  }
}
