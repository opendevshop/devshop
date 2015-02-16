<?php

function devshop_cloud_settings_form() {
  $form = array();
  $form['note'] = array(
    '#value' => 'You must enable at least one cloud provider module.',
  );
  return system_settings_form($form);
}