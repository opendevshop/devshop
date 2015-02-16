<?php

function devshop_cloud_settings_form() {
  $form = array();
  $form['note'] = array(
    '#value' => t('You must enable at least one cloud provider module.'),
    '#prefix' => '<div>',
    '#suffix' => '</div>',
  );
  return system_settings_form($form);
}