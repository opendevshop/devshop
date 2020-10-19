<?php



/**
 * Form function for the digital_ocean options form.
 * @return array
 */
function aegir_digital_ocean_options_form() {
  $form = array();
  aegir_digitalocean_load_api();
  $token = variable_get('aegir_cloud_digital_ocean_api_token', array());

  if (!empty($token)) {
    $options = variable_get('aegir_cloud_digital_ocean_options', array());

    $form['info'] = array(
      '#type' => 'item',
      '#title' => t('Digital Ocean Options'),
      '#value' => empty($options)? t('No options available. Click "Refresh Digital Ocean Options".'): '',
    );

    if (!empty($options)) {

      $region_options = $options['regions'];
      ksort($region_options);
      $default_options = variable_get('aegir_digital_ocean_default_options', array());
      $form['region'] = array(
        '#type' => 'fieldset',
        '#title' => t('Region'),
        '#collapsible' => true,
        '#collapsed' => true,
      );
      $form['region']['aegir_digital_ocean_region_default'] = array(
        '#type' => 'select',
        '#title' => t('Default Region'),
        '#options' => $region_options,
        '#default_value' => $default_options['aegir_digital_ocean_region_default'],
      );

      $form['region']['aegir_digital_ocean_region_allowed'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Allowed Regions'),
        '#description' => t('Choose the regions that are shown on the !link page.', array(
          '!link' => l(t('Create Server'), 'node/add/server'),
        )),
        '#options' => $region_options,
        '#default_value' => $default_options['aegir_digital_ocean_region_allowed'],
      );

      $size_options = $options['sizes'];
      $form['aegir_digital_ocean_default_size'] = array(
        '#type' => 'select',
        '#title' => t('Size'),
        '#options' => $size_options,
        '#default_value' => $default_options['aegir_digital_ocean_default_size'],
      );

      $image_options = $options['images'];
      $form['aegir_digital_ocean_default_image'] = array(
        '#type' => 'select',
        '#title' => t('Image'),
        '#options' => $image_options,
        '#default_value' => $default_options['aegir_digital_ocean_default_image'],
      );
      $key_options = $options['keys'];
      $form['aegir_digital_ocean_default_keys'] = array(
        '#type' => 'checkboxes',
        '#title' => t('SSH Keys'),
        '#options' => $key_options,
        '#default_value' => $default_options['aegir_digital_ocean_default_keys'],
      );
      $form['aegir_digital_ocean_default_backups'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable Backups'),
        '#default_value' => $default_options['aegir_digital_ocean_default_backups'],
      );
      $form['aegir_digital_ocean_default_ipv6'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable IPv6'),
        '#default_value' => $default_options['aegir_digital_ocean_default_ipv6'],
      );
      $form['aegir_digital_ocean_default_private_networking'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable Private Networking'),
        '#default_value' => $default_options['aegir_digital_ocean_default_private_networking'],
      );
      $form['aegir_digital_ocean_default_user_data'] = array(
        '#type' => 'textarea',
        '#title' => t('Cloud Init'),
        '#default_value' => $default_options['aegir_digital_ocean_default_user_data'],
        '#description' => t('Cloud Init (aka User Data) is a standardized way to pre-configure your server, supported by most cloud server providers. If this field is Bash, it is run as soon as the server is ready. If this field has YML, it can do many things. See !link for Cloud Init examples', array(
          '!link' => l('cloudinit.readthedocs.io', 'https://cloudinit.readthedocs.io/en/latest/topics/examples.html')
        )),
      );
    }

    $form['note'] = array(
      '#prefix' => '<div>',
      '#suffix' => '</div>',
      '#value' => t('Use the button below to retrieve available options from Digital Ocean.'),
    );

    $form['submit'] = array(
      '#type' => 'submit',
      '#name' => 'save',
      '#value' => t('Save Default Options'),
    );

    $form['refresh'] = array(
      '#type' => 'submit',
      '#name' => 'refresh',
      '#value' => t('Refresh Digital Ocean Options'),
    );
  }

  return $form;
}

/**
 *
 */
function aegir_digital_ocean_options_form_submit($form, $form_state) {

  $button = $form_state['clicked_button']['#name'];

  if ($button == 'save') {
    $values = $form_state['values'];
    variable_set('aegir_digital_ocean_default_options', $values);
  }
  else {

    $options = array();

    $digitalocean = aegir_digitalocean_load_api();

    $image = $digitalocean->image();
    $images = $image->getAll();
    $image_options = array();
    foreach ($images as $image) {
      $image_options[$image->id] = $image->distribution . ' - ' . $image->name;
    }
    $options['images'] = $image_options;

    $region = $digitalocean->region();
    $regions = $region->getAll();
    $region_options = array();
    foreach ($regions as $region) {
      $region_options[$region->slug] = $region->name;
    }
    $options['regions'] = $region_options;

    $key = $digitalocean->key();
    $keys = $key->getAll();
    $key_options = array();
    foreach ($keys as $key) {
      $key_options[$key->id] = $key->name;
    }
    $options['keys'] = $key_options;

    $size = $digitalocean->size();
    $sizes = $size->getAll();
    foreach ($sizes as $size) {
      $size_options[$size->slug] = $size->slug . ' - ' . $size->disk . 'gb disk' . ' - ' . $size->vcpus . 'vcpus';
    }
    $options['sizes'] = $size_options;


    variable_set('aegir_cloud_digital_ocean_options', $options);
  }
}
