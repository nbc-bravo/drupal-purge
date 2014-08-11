<?php

/**
 *  @file
 *   Provides administrative interface for the Purge module.
 */

// Load the purge.inc file
module_load_include('inc', 'purge', 'includes/purge');
// module_load_include('inc', 'purge', 'includes/examples');

/**
 * Generates UI elements for the Purge UI module.
 */
class PurgePurgerBundleUI extends PurgePurgerBundleAPI {
  public $action = 'view';
  public $item_type = NULL;
  public $item_name = NULL;
  public $form_item;


  /**
   * Sets the bundle with data from the form.
   */
  public function set_form_item($action = 'view', $item_type = NULL, $item_name = NULL) {
    $this->action = $action;
    $this->item_type = $item_type;
    $this->item_name = $item_name;
    // See if we are managing an existing item.
    if (!is_null($item_type) && !is_null($item_name)) {
      if ($action == 'view' || $action == 'edit') {
        $this->form_item = $this->{$item_type}[$item_name];
      }
      elseif ($action == 'clone') {
        $this->form_item = clone $this->{$item_type}[$item_name];
      }
      elseif ($action == 'example') {
        $example_bundle = new PurgePurgerBundleExample();
        $this->form_item = clone $example_bundle->{$item_type}[$item_name];
        // @TODO: also add the items of this example in the bundle.
      }
      elseif ($action == 'add' && $item_name = 'new') {
        if ($item_type == 'target') {
          $this->form_item = new PurgeTargetStatic();
        }
        elseif ($item_type == 'domain') {
          $this->form_item = new PurgeDomainStatic();
        }
        elseif ($item_type == 'header') {
          $this->form_item = new PurgeHeaderStatic();
        }
        elseif ($item_type == 'purger') {
          $this->form_item = new PurgePurgerBundleURL();
        }
      }
    }
  }


  /**
   * Generates a form form the current form settings.
   *
   * @return array $form
   */
  public function get_form() {
    // Check if this is a list or an item.
    if ($this->action == 'view' && is_null($this->item_name)) {
      $form = $this->get_form_list();
    }
    elseif ($this->action == 'view') {
      $form = $this->get_form_view();
    }
    else {
      $form = $this->get_form_edit();
    }
    return $form;
  }

  /**
   * Generates a list form.
   */
  public function get_form_list() {
    $form = array();
    // Check if we're just showing a single type.
    if (!is_null($this->item_type)) {
      $types = array($item_type);
    }
    else {
      $types = $this->item['type'];
      // Make sure purgers are displayed first.
      $types = array_diff($types, array('purger', 'type'));
      array_unshift($types, 'purger');
    }
    // Loop through all types.
    foreach ($types as $type) {
      $form[$type] = $this->get_form_list_type($type);
    }

    return $form;
  }

  /**
   * Generates a list of items of a specific type.
   *
   * @param string $item_type
   *
   * @return array $form
   */
  public function get_form_list_type($item_type) {
    // fieldset for each type
    $form = array(
      '#type' => 'fieldset',
      '#title' => $this->type[$item_type]->name,
    );
    $form['description'] = array(
      '#type' => 'item',
      '#markup' => $this->type[$item_type]->description,
    );
    // Add link
    if (in_array(PURGE_ACCESS_FULL, $this->type[$item_type]->access)) {
      $form['add'] = array(
        '#type' => 'item',
        '#title' => l(t('Add @type', array('@type' => $this->type[$item_type]->name)), "admin/config/system/purge/add/{$item_type}"),
      );
    }
    $table_options = array();

    // Generate the table form.
    foreach ($this->{$item_type} as $item_name => $item) {
      $actions = array();
      // Skip all system items.
      if (!in_array(PURGE_ACCESS_SYSTEM, $item->access)) {
        // Check if we'll only show an edit option.
        if (in_array(PURGE_ACCESS_FULL, $item->access) || in_array(PURGE_ACCESS_ENABLE, $item->access)) {
          $actions[] = l(t('Edit'), "admin/config/system/purge/edit/{$item_type}/{$item_name}");
        }
        // else check if a view action is needed.
        elseif (in_array(PURGE_ACCESS_VIEW, $item->access)) {
          $actions[] = l(t('View'), "admin/config/system/purge/view/{$item_type}/{$item_name}");
        }

        // Enable/disable cations.
        if (in_array(PURGE_ACCESS_FULL, $item->access) || in_array(PURGE_ACCESS_ENABLE, $item->access)) {
          if ($item->enabled == 1) {
            $actions[] = l(t('Disable'), "admin/config/system/purge/disable/{$item_type}/{$item_name}");
          }
          else {
            $actions[] = l(t('Enable'), "admin/config/system/purge/enable/{$item_type}/{$item_name}");
          }
        }
        // Clone
        if (in_array(PURGE_ACCESS_FULL, $item->access) || in_array(PURGE_ACCESS_CLONE, $item->access)) {
          $actions[] = l(t('Clone'), "admin/config/system/purge/clone/{$item_type}/{$item_name}");
        }
        if (in_array(PURGE_ACCESS_FULL, $item->access)) {
          $actions[] = l(t('Delete'), "admin/config/system/purge/delete/{$item_type}/{$item_name}");
        }
        // Get the curent status.
        if ($item->enabled == 1) {
          $status = t('Enabled');
        }
        else {
          $status = t('Disabled');
        }
        $table_options[$item_name] = array(
          'name' => $item->name,
          'description' => $item->description,
          'status' => $status,
          'actions' => implode(' | ', $actions),
        );
      }
    }
    $form['items'] = array(
      '#type' => 'tableselect',
      '#header' => array(
        'name' => t('Name'),
        'description' => t('Description'),
        'status' => t('Status'),
        'actions' => t('Actions'),
      ),
      '#options' => $table_options,
    );
    return $form;
  }

  /**
   * Generates a form to view an item.
   */
  private function get_form_view() {
    $form = array();
    // First get the form with basic information up.
    $form['purge_basic'] = $this->get_form_view_basic();
    // Get the status form field.
    $form['purge_status'] = $this->get_form_view_status();
    // Get the item specific form field.
    $form_name = 'purge_' . $this->item_type;
    $form[$form_name] = $this->get_form_view_{$this->item_type};
    // Get the buttons.
    $form['purge_action'] = $this->get_form_action();

    return $form;
  }


  /**
   * Generates a form to edit an item.
   *
   * @return array $form
   */
  private function get_form_edit() {
    $form = array();

    // First get the form with basic information up.
    $form['purge_basic'] = $this->get_form_edit_basic();
    // Get the status form field.
    $form['purge_status'] = $this->get_form_edit_status();
    // Get the item specific form field.
    $form_name = 'purge_' . $this->item_type;
    $form[$form_name] = $this->get_form_edit_{$this->item_type};
    // Get the buttons.
    $form['purge_action'] = $this->get_form_action();

    return $form;
  }

  /**
   * Generate the basic information view.
   */
  private function get_form_view_basic() {
    $form = array();
    $form = array(
      '#type' => 'fieldset',
      '#title' => t('Basic Item Configuration'),
    );
  }


  /**
   * Generate the basic information form.
   */
  private function get_form_edit_basic() {
    $form = array();
    $form['purge_item'] = array(
      '#type' => 'fieldset',
      '#title' => t('Basic Item Configuration'),
    );
    $form['name'] = array(
      '#type' => 'textfield',
      '#title' => t('Name'),
      '#required' => TRUE,
      '#description' => t('The user-friendy name for the item.'),
      '#default_value' => $this->form_item->name,
    );
    if ($action == 'add' || $action == 'clone') {
      $form['machine_name'] = array(
        '#type' => 'machine_name',
        '#maxlength' => 21,
        '#default_value' => '',
        '#machine_name' => array(
          'exists' => 'purge_item_name_exists',
          'source' => array('purge_item', 'name'),
        ),
      );
    }
    else {
      $form['machine_name'] = array(
        '#type' => 'hidden',
        '#default_value' => $item_name,
      );
    }
    $form['description'] = array(
      '#type' => 'textfield',
      '#title' => t('Description'),
      '#default_value' => $item->description,
    );


  }

}


/**
 * Load the api bundle from the variable.
 *
 * @return $bundle
 */
function purge_get_bundle_ui($form, &$form_state, $action = 'view', $item_type = NULL, $item_name = NULL) {
  $data = variable_get('purge_config');
  // Load the UI bundle from the API bundle data.
  $data = str_replace('O:20:"PurgePurgerBundleAPI"', 'O:19:"PurgePurgerBundleUI"', $data);
  $bundle = unserialize($data);

  $bundle->set_form_item($action, $item_type, $item_name);

  return $bundle;
}

/**
 * Menu callback for purge admin settings.
 */
function purge_configuration_form($form, &$form_state, $action = 'view', $item_type = NULL) {
  // Debugging
  // purge_expire_cache(array('http://purge72/node/1'));
  // purge_urls(array('http://purge72/node/1'));
  $bundle = purge_get_bundle_ui($form, $form_state, $action , $item_type, $item_name = NULL);
  $form = array();
  $form = $bundle->get_form();

  return $form;
}

/**
 * Menu callback for the add item form.
 */
function purge_item_add_form($form, &$form_state, $item_type) {
  // Load the bundle.
  $bundle = purge_get_bundle_ui($form, $form_state, 'add', $item_type);
  // And load the example bundle.
  module_load_include('inc', 'purge', 'includes/defaults');
  $example_bundle = new PurgePurgerBundleExample();
  $form = array();
  // Hidden field with item_type information for later processing.
  $form['item_type'] = array(
    '#type' => 'hidden',
    '#default_value' => $item_type,
  );

  // First check if this itme type is allowed to be added.
  if (!in_array(PURGE_ACCESS_FULL, $bundle->type[$item_type]->access)) {
    drupal_set_message(t('You are not permitted to add items of type @type.', array('@type' => $bundle->type[$item_type]->name)));
    drupal_goto('admin/config/system/purge');
    return;
  }
  // The header of the selecfield.
  $header = array(
    'name' => t('Name'),
    'description' => t('Description'),
    'provider' => t('Provided by'),
  );

  // The options.
  // First the option to create a new item.
  $options = array(
    'new' => array(
      'name' => t('New'),
      'description' => t('Start from scratch with an empty @type.', array('@type' => $bundle->type[$item_type]->name)),
      'provider' => t('You!'),
    ),
  );
  // Now include all built in examples.
  foreach ($example_bundle->{$item_type} as $item_name => $item) {
    $options[$item_name] = array(
      'name' => $item->name,
      'description' => $item->description,
      'provider' => t('Purge Examples'),
    );
  }

  $form['template_select'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#multiple' => FALSE,
    '#default_value' => 'new',
  );
  // Submit
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Select'),
  );

  return $form;
}

/**
  *  Callback for the select button on the template form.
  */
function purge_item_add_form_submit($form, &$form_state) {
  if (isset($form_state['values']['template_select'])) {
    // New item selected.
    if ($form_state['values']['template_select'] == 'new') {
      $form_state['redirect'] = 'admin/config/system/purge/add/' . $form_state['values']['item_type'] . '/new';
    }
    // Example selected.
    else {
      $form_state['redirect'] = 'admin/config/system/purge/example/' . $form_state['values']['item_type'] .
        '/' . $form_state['values']['template_select'];
    }
  }
}

/**
 * Menu callback for the purge item edit form.
 */
function purge_item_form($form, &$form_state, $action = 'view', $item_type = NULL, $item_name = NULL) {
  // Get the bundle.
  $bundle = purge_get_bundle_ui($form, $form_state, $action = 'view', $item_type, $item_name = NULL);
  // Get the form
  $form = array();
  $form = $bundle->get_form();

  return $form;
}


/**
 * Menu callback for purge item settins.
 * @TODO REMOVE THIS. Die old functional code, die!.
 */
function OLDpurge_item_form($form, &$form_state, $item_type, $item_name = NULL, $action = 'view') {
  // First lets load a bundle.
  $bundle = new PurgePurgerBundleUI();
  // Select the object we need for this form.
  if ($action == 'add') {
    switch ($item_type) {
      case 'target':
        $item = new PurgeTargetStatic();
        break;
      case 'domain':
        $item = new PurgeDomainStatic();
        break;
      case 'header':
        $item = new PurgeHeaderStatic();
        break;
      case 'purger':
        $item = new PurgePurgerBasicUrl();
        break;
    }
    $item_name = '';
  }
  elseif ($action == 'clone') {
    $item = clone $bundle->{$item_type}[$item_name];
    $item->name = '';
    $item->description = '';
  }
  elseif ($action == 'example') {
    // Get the example bundle
    module_load_include('inc', 'purge', 'includes/defaults');
    $examples = new PurgePurgerBundleExample();
    $item = clone $examples->{$item_type}[$item_name];
    // Set pointers for purgers.
    if ($item_type == 'purger') {
      foreach ($item->pointers as $pointer_type => $pointers) {
        foreach ($pointers as $pointer_name) {
          // Check if the bundle has the item.
          if (!isset($bundle->{$pointer_type}[$pointer_name])) {
            // Check look for it in the exampele bundle.
            if (isset($example->{$pointer_type}[$pointer_name])) {
              // add it to the bundle.
              $bundle->{$pointer_type}[$pointer_name] = clone $example{$pointer_type}[$pointer_name];
            }
          }
          // Link the item to the bundle object.
          $item->{$pointer_type}[$pointer_name] = $bundle->{$pointer_type}[$pointer_name];
        }
      }
    }
  }


  else {
    $item = $bundle->{$item_type}[$item_name];
  }

  // The Basic Form elements.
  $form = array();
  $form['purge_item'] = array(
    '#type' => 'fieldset',
    '#title' => t('Basic Item Configuration'),
  );
  // First some hidden fields for internal use.
  $form['purge_item']['item_action'] = array(
    '#type' => 'hidden',
    '#default_value' => $action,
  );
  $form['purge_tem']['item_type'] = array(
    '#type' => 'hidden',
    '#default_value' => $item_type,
  );
  $form['purge_item']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#required' => TRUE,
    '#description' => t('The user-friendy name for the item.'),
    '#default_value' => $item->name,
  );
  if ($action == 'add' || $action == 'clone') {
    $form['purge_item']['machine_name'] = array(
      '#type' => 'machine_name',
      '#maxlength' => 21,
      '#default_value' => '',
      '#machine_name' => array(
        'exists' => 'purge_item_name_exists',
        'source' => array('purge_item', 'name'),
      ),
    );
  }
  else {
    $form['purge_item']['machine_name'] = array(
      '#type' => 'hidden',
      '#default_value' => $item_name,
    );
  }

  $form['purge_item']['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#default_value' => $item->description,
  );
  $form['purge_item']['enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enabled'),
    '#description' => t('Disabled items will ignored during operations, even when included in purgers.'),
    '#default_value' => $item->enabled,
  );
  // Now switch between the item_types to generate their respecitve forms.
  switch($item_type) {
    // The target form.
    case 'target':
      $form['target'] = array(
        '#type' => 'fieldset',
        '#title' => 'Target Configuration',
      );
      $form['target']['urls'] = array(
        '#type' => 'textarea',
        '#title' => t('URLs'),
        '#description' => t('The URLs where the Purge requests will be sent to. One URL per line.'),
        '#required' => true,
        '#default_value' => implode("\n", $item->urls),
      );
    break;
    case 'domain':
      $form['domain'] = array(
        '#type' => 'fieldset',
        '#title' => 'Domain Configuration',
      );
      $form['domain']['domains'] = array(
        '#type' => 'textarea',
        '#title' => t('Domains'),
        '#description' => t('For each domain name Purge sends out a request. One domain per line.'),
        '#required' => true,
        '#default_value' => implode("\n", $item->domains),
      );
    break;
    case 'header':
      $form['header'] = array(
        '#type' => 'fieldset',
        '#title' => 'Header Configuration',
      );
      $headers = array();
      foreach ($item->headers as $header_key => $header_value) {
        $headers[] = implode(": ", array($header_key, $header_value));
      }
      $form['header']['headers'] = array(
        '#type' => 'textarea',
        '#title' => t('Header'),
        '#description' => t('Headers are added to each request. One header per line. Format: Header: value'),
        '#required' => true,
        '#default_value' => implode("\n", $headers),
      );
    break;
    case 'purger':
      $form['purger'] = array(
        '#type' => 'fieldset',
        '#title' => 'Purger configuration',
      );
      // Generate the set
      $table_header = array(
        'name' => t('Name'),
        'description' => t('Description'),
      );
      foreach ($item->pointers as $pointer_type => $pointer_names) {
        $rows = array();
        $default_value = array();
        $multiple = true;
        if ($pointer_type == 'handler' || $pointer_type == 'queue') {
          $multiple = false;
        }
        foreach ($bundle->{$pointer_type} as $pointer_name => $pointer) {
          $rows[$pointer_name] = array(
            'name' => $bundle->{$pointer_type}[$pointer_name]->name,
            'description' => $bundle->{$pointer_type}[$pointer_name]->description,
          );
          if (in_array($pointer_name, $item->pointers[$pointer_type])) {
            if ($multiple) {
              $default_value[$pointer_name] = $pointer_name;
            }
            else {
              $default_value = $pointer_name;
            }
          }
        }
        $form['purger'][$pointer_type] = array(
          '#type' => 'fieldset',
          '#title' => $bundle->type[$pointer_type]->name,
          '#description' => $bundle->type[$pointer_type]->description,
        );
        $form['purger'][$pointer_type][$pointer_type . '_settings'] = array(
          '#type' => 'tableselect',
          '#header' => $table_header,
          '#options' => $rows,
          '#default_value' => $default_value,
          '#multiple' => $multiple,

        );
        // Check if this type has options.
        $options = array();
        if (isset($bundle->type[$pointer_type]->options)) {
          // Now get the form items for the options.
          $form['purger'][$pointer_type][$pointer_type . '_options'] = array(
            '#type' => 'fieldset',
            '#title' => $bundle->type[$pointer_type]->name . ' ' . t('Options'),
          );
          // Display the type options
          $options = $bundle->type[$pointer_type]->options;
          $all_item_options = array();
          foreach ($options as $option_name => $option) {
            // First display a form for all type specific options
            if ($options[$option_name]['context'] == 'type') {
            // If the purger has a value for this option
              if (isset($item->options[$pointer_type][$option_name])) {
                $option['purger_value'] = $item->options[$pointer_type][$option_name];
              }
              // Now generate a form for these options.
              $form['purger'][$pointer_type][$pointer_type . '_options'][$item_name . '_' . $option_name] =
                purge_option_form($option);
            }
            // Then save the item specific options
            elseif ($options[$option_name]['context'] == 'item') {
              $all_item_options[$option_name] = $option;
            }
          }
          // First go to all items of this type
          foreach ($bundle->{$pointer_type} as $pointer_name => $pointer) {
            $this_item_options = array();
            // Now pass each item option
            foreach ($all_item_options as $item_option_name => $item_option) {
              // Check if the item has the option defined.
              if (isset($bundle->{$pointer_type}[$pointer_name]->options[$item_option_name])) {
                $this_item_options[$item_option_name] = $item_option;
                if (isset($item->options[$pointer_type][$item_option_name])) {
                  $this_item_options[$item_option_name]['purger_value'] = $item->options[$pointer_type][$item_option_name];
                }
              }
            }
            if (count($this_item_options) > 0) {
              $form['purger'][$pointer_type][$pointer_type . '_options'][$pointer_name] = array(
                '#type' => 'fieldset',
                '#title' => $bundle->{$pointer_type}[$pointer_name]->name . ' ' . t('Options'),
                '#states' => array(
                  'visible' => array(
                    ':input[name="' . $pointer_type . '_settings"]' => array('value' => $pointer_name),
                  ),
                ),
              );
              // Generate an option form.
              foreach ($this_item_options as $option_name => $option) {
                $form['purger'][$pointer_type][$pointer_type . '_options'][$pointer_name][$pointer_name . '_' . $option_name]
                  = purge_option_form($option);
              }
            }
          }
        }
      }
    break;
  }
  // Now the items type options.
  $options = array();
  $options = $bundle->type[$item_type]->options;
  if (count($options) > 0) {
    if ($item_type == 'purger') {
      $form[$item_type]['options'] = array(
        '#type' => 'fieldset',
        '#title' => t('Purger Options'),
        '#description' => t('These settings apply to this purger only.'),
      );
      foreach ($options as $option_name => $option) {
        if (isset($item->options['purger'][$option_name])) {
          $option['purger_value'] = $item->options['purger'][$option_name];
        }
        $form[$item_type]['options'][$item_type . '_' . $option_name] =  purge_option_form($option);
      }
    }
    else {
      $form[$item_type]['options'] = array(
        '#type' => 'fieldset',
        '#title' => t('Default Options'),
        '#description' => t('These defaults can be overruled per purger.'),
      );
    }
  }

  // Now display submit buttons.
  if ($action != 'view' && in_array(PURGE_ACCESS_FULL, $item->access)) {
    $form['submit']  = array(
      '#type' => 'submit',
      '#value' => t('Save'),
    );
    if ($item instanceof PurgeValidateable) {
      $form['validate'] = array(
        '#type' => 'button',
        '#value' => t('Validate'),
       // '#submit' => array('purge_item_form_validate'),
      );
    }
  }
  // Enable/disable buton.
  if (in_array(PURGE_ACCESS_ENABLE, $item->access)) {
    if ($item->enabled) {
      $form['disable'] = array(
        '#type' => 'submit',
        '#name' => 'disable',
        '#value' => t('Disable'),
        '#submit' => array('purge_form_button_callback'),
      );
    }
    else {
      $form['enable'] = array(
        '#type' => 'submit',
        '#name' => 'enable',
        '#value' => t('Enable'),
        '#submit' => array('purge_form_button_callback'),
      );
    }
  }
  if (in_array(PURGE_ACCESS_FULL, $item->access)) {
    $form['delete'] = array(
      '#type' => 'submit',
      '#name' => 'delete',
      '#value' => t('Delete'),
      '#submit' => array('purge_form_button_callback'),
    );
  }
  return $form;
}

/**
 * Form validation for the item form.
 */
function purge_item_form_validate($form, &$form_state) {
  $errors = array();
  // First get the item with the form state
  $bundle = purge_item_form_load($form, $form_state);
  // First validate the item.
  $item_type = $form_state['values']['item_type'];
  $item_name = $form_state['values']['machine_name'];
  $item = $bundle->{$item_type}[$item_name];
  // Validate if needed.
  if ($item instanceof PurgeValidateable) {
    $errors = $item->validate();
    if (count($errors) == 0) {
    // Validate the bundle.
     // $errors = $bundle->validate();
    }
    // Now set a form error.
    if (count($errors) > 0) {
      foreach ($errors as $error) {
        // Display the errors
        form_set_error($error['name'], $error['message']);
      }
    }
    else {
      // Check if we're being triggered by the validate butten.
      if ($form_state['clicked_button']['#type'] == 'button') {
        $form_state['rebuild'] == FALSE;
        drupal_set_message(t('@item_type @item_name validated correctly.', array(
          '@item_type' => $bundle->type[$item_type]->name,
          '@item_name' => $form_state['values']['name'],
        )));
      }
    }
  }
  return $form;
}

/**
 * Form submit for the item form.
 */
function purge_item_form_submit($form, &$form_state) {
  // Get the bundle from the current form state.
  $bundle = purge_item_form_load($form, $form_state);
  // Save it.
  $bundle->save();
  $item_type = $form_state['values']['item_type'];
  drupal_set_message(t('@item_type @item_name was saved.', array(
    '@item_type' => $bundle->type[$item_type]->name,
    '@item_name' => $form_state['values']['name'],
  )));

  // Redirect to main admin page.
  $form_state['redirect'] = 'admin/config/system/purge';
  return $form;
}

/**
 * Callback for enable/disable.
 *
 * Calls the confirmation form if the bundle doesn't validate after.
 */
function purge_item_action_callback($item_type, $item_name, $action) {
  // Get the bundle and the item.
  $bundle = new PurgePurgerBundleAPI();
  $item = $bundle->{$item_type}[$item_name];
  $access = FALSE;
  // Check if we can enable/disable this item.
  if ($action == 'enable' || $action == 'disable') {
    if (in_array(PURGE_ACCESS_ENABLE, $item->access) || in_array(PURGE_ACCESS_FULL, $item->access)) {
      $access = TRUE;
      if ($action == 'enable') {
        $actioned = t('enabled');
        // Check if the item is already enabled.
        if ($item->enabled == 1) {
          drupal_set_message(t('@item_name is already enabled.'), array(
            '@item_name' => $item->name,
          ));
          drupal_goto('admin/config/system/purge');
          return;
        }
        else {
          // Enable the tiem.
          $item->enabled = 1;
        }
      }
      if ($action == 'disable') {
        $actioned = t('disabled');
        if ($item->enabled == 0) {
          drupal_set_message(t('@item_type  @item_name is already disabled.', array(
            '@item_type' => $bundle->type[$item_type]->name,
            '@item_name' => $item->name,
          )));
          drupal_goto('admin/config/system/purge');
          return;
        }
        else {
          // Disable the item.
          $item->enabled = 0;
        }
      }
    }
  }
  // Unown action.
  else {
    return;
  }
  $bundle->save();
  drupal_set_message('Saved');
  drupal_goto('admin/config/system/purge');

}


/**
 * Form for simple yes/no operations on items.
 */
function purge_item_action_form($form, &$form_state, $item_type, $item_name, $action) {
  // Get the bundle and the item.
  $bundle = new PurgePurgerBundleUI();
  $item = $bundle->{$item_type}[$item_name];
  if ($action == 'delete') {
    $actioned = t('deleted');
    // Check if we have access to delete this item.
    if (in_array(PURGE_ACCESS_FULL, $item->access)) {
      $access = TRUE;
      // Check if something bad happens is this item is deleted.
      unset($bundle->{$item_type}[$item_name]);
    }
  }
  else {
    // Unkown action. Throw warming and redirect to main admin view.
    drupal_set_message(t('Unkown action'), 'error');
    $form_state['redirect'] = 'admin/config/system/purge';
    return;
  }
  // Now make sure we have access.
  if (!$access) {
    drupal_set_message(t('Action not allowed'), 'warning');
    $form_state['redirect'] = 'admin/config/system/purge';
    return;
  }
  else {
    // Check if there would be errors after this action.
    $errors = $bundle->validate();
    if (count($errors) > 0) {
      // Display this warming:
      $message = t(
        'If you confirm to @action @item_type @item_name your current configuration will no longer be valid.'
          . ' See these warmings:', array(
            '@action' => $action,
            '@item_type' => $bundle->type[$item_type]->name,
            '@item_name' => $item->name,
          )
        );
      // Dispaly each error message.
      foreach ($errors as $error) {
        $message = $message . '<br>' . $error;
      }
      drupal_set_message($message, 'warning');
    }
    // Display the question if there are errors or the action is delete.
    if (count($errors) > 0 || $action == 'delete') {
      $form = array();
      $form['action'] = array(
        // Hidden fields to save arguments in form state.
        'item_type' => array(
          '#type' => 'hidden',
          '#default_value' => $item_type,
        ),
        'item_name' => array(
          '#type' => 'hidden',
          '#default_value' => $item_name,
        ),
        'action' => array(
          '#type' => 'hidden',
          '#default_value' => $action,
        ),
        'question' => array(
          '#type' => 'item',
          '#markup' => t('Are you sure you want to @action the @item_type @item_name ?', array(
            '@action' => t($action),
            '@item_type' => $bundle->type[$item_type]->name,
            '@item_name' => $item->name,
          )),
        ),
        'confirm' => array(
          '#type' => 'submit',
          '#value' => t('Confirm'),
          '#submit' => array('purge_item_action_form_submit'),
        ),
        'cancel' => array(
          '#type' => 'submit',
          '#name' => 'cancel',
          '#value' => t('Cancel'),
          '#submit' => array('purge_form_button_callback'),
        ),
      );
    }
  }
  return $form;
}

/**
 * Form submition for item ation form.
 */
function purge_item_action_form_submit($form, &$form_state) {
  // load the bundle and the item.
  $bundle = new PurgePurgerBundleUI();
  $item_type = $form_state['values']['item_type'];
  $item_name = $form_state['values']['item_name'];
  $action = $form_state['values']['action'];
  $item = $bundle->{$item_type}[$item_name];

  // Perform the action.
  if ($action == 'delete') {
    // Delete the item from the bundle.
    $item_name_full = $item->name;
    unset($bundle->{$item_type}[$item_name]);
    // Save the bundle.
    $bundle->save();
    // Display a status message.
    drupal_set_message(t('@item_name is now deleted.', array('@item_name' => drupal_ucfirst($item_name_full))));
  }

  $form_state['redirect'] = 'admin/config/system/purge';
  return $form;
}

/**
 * Simple form callback to redirect after a form button is clicked.
 */
function purge_form_button_callback($form, &$form_state) {
  // Cancel button.
  if ($form_state['clicked_button']['#name'] == 'cancel') {
    // Redirect to main page.
    $form_state['redirect'] = 'admin/config/system/purge';
  }
  // Enable button.
  elseif ($form_state['clicked_button']['#name'] == 'enable') {
    $form_state['redirect'] = 'admin/config/system/purge/enable/'
      . $form_state['values']['item_type'] . '/'
      . $form_state['values']['machine_name']
    ;
  }
  // Disable button.
  elseif ($form_state['clicked_button']['#name'] == 'disable') {
    $form_state['redirect'] = 'admin/config/system/purge/disable/'
      . $form_state['values']['item_type'] . '/'
      . $form_state['values']['machine_name']
    ;
  }
  // Delete button.
  elseif ($form_state['clicked_button']['#name'] == 'delete') {
    $form_state['redirect'] = 'admin/config/system/purge/delete/'
      . $form_state['values']['item_type'] . '/'
      . $form_state['values']['machine_name']
    ;
  }

  return $form;
}


/**
 * Form generator for option form.
 */
function purge_option_form($option) {
  $form = array(
    '#title' => $option['name'],
    '#description' => $option['description'],
  );
  if (isset($option['values'])) {
    $form['#type'] = 'select';
    $form['#options'] = $option['values'];
  }
  else {
    $form['#type'] = 'checkbox';
  }
  if (isset($option['purger_value'])) {
    $form['#default_value'] = $option['purger_value'];
  }
  else {
    $form['#default_value'] = $option['default_value'];
  }
  return $form;
}


/**
 * Function to load an item from current form values.
 *
 * @param array $form
 * @param array $form_state
 *
 * @return object $bundle
 */
function purge_item_form_load($form, &$form_state) {
  $bundle = new PurgePurgerBundleUI;
  $item_action = $form_state['values']['item_action'];
  // See what action is to be taken.
  $item_name = $form_state['values']['machine_name'];
  $item_type = $form_state['values']['item_type'];
  if ($item_action == 'add' || $item_action == 'clone' || $item_action == 'example') {
    switch ($item_type) {
      case 'target':
        $bundle->target[$item_name] = new PurgeTargetStatic();
        $item = $bundle->target[$item_name];
        break;
      case 'domain':
        $bundle->domain[$item_name] = new PurgeDomainStatic();
        $item = $bundle->domain[$item_name];
        break;
      case 'header':
        $bundle->header[$item_name] = new PurgeHeaderStatic();
        $item = $bundle->header[$item_name];
        break;
      case 'purger':
        $bundle->purger[$item_name] = new PurgePurgerBasicUrl();
        $item = $bundle->purger[$item_name];
        break;
    }
  }
  elseif ($item_action == 'edit') {
    $item = $bundle->{$item_type}[$item_name];
  }
  // First process the basic item properties
  $item->name = check_plain($form_state['values']['name']);
  $item->description = check_plain($form_state['values']['description']);
  $item->enabled = $form_state['values']['enabled'];

  // Now for each item specific property
  switch ($item_type) {
    case 'target':
      // Start with an empty array.
      $item->urls = array();
      // Get the URLs from the form state.
      $urls = preg_split('/$\R?^/m', $form_state['values']['urls']);
      // trims each url.
      foreach ($urls as $url) {
        $item->urls[] = trim($url);
      }
      break;
    case 'domain':
      $item->domains = preg_split('/$\R?^/m', $form_state['values']['domains']);
      break;
    case 'header':
      $headers = preg_split('/$\R?^/m', $form_state['values']['headers']);
      $item->headers = array();
      foreach ($headers as $header) {
        $header_key = trim(strstr($header, ":", true));
        $header_value = trim(strstr($header, ":"), " :\t");
        $item->headers[$header_key] = $header_value;
      }
      break;
    case 'purger':
      // Loop through all item types
      foreach ($item->pointers as $pointer_type => $pointer) {
        // Get the item valuea from the form
        $items = $form_state['values'][$pointer_type . '_settings'];
        $items_enabled = array();
        // See if this was a single of multi field.
        if ($form['purger'][$pointer_type][$pointer_type . '_settings']['#multiple'] == 0) {
          $item_name = $items;
          $item->{$pointer_type}[$item_name] = $bundle->{$pointer_type}[$item_name];
          $items_enabled = array($items);
        }
        else {
          // Multiple options form
          foreach ($items as $item_name => $item_selected) {
            // Check each item if it's selected
            if ($item_selected != '0') {
              // Point to set property to the bundle object
              $item->{$pointer_type}[$item_name] = $bundle->{$pointer_type}[$item_name];
              $items_enabled[] = $item_name;
            }
          }
        }
        // Set the pointers
        $item->pointers[$pointer_type] = $items_enabled;
        // Set the options
        if (isset($bundle->type[$pointer_type]->options)) {
          foreach ($bundle->type[$pointer_type]->options as $option_name => $option) {
            // Check if context is type
            if ($option['context'] == 'type') {
              $item->options[$pointer_type][$option_name] = $form_state['values'][$pointer_type . '_' . $option_name];
            }
            // or process as item specific options.
            elseif ($option['context'] == 'item') {
              $item_name = $form_state['values'][$pointer_type . '_settings'];
              $item->options[$pointer_type][$option_name] = $form_state['values'][$item_name . '_' . $option_name];
            }
          }
        }
      }
      // Now process the purger options
      foreach ($bundle->type['purger']->options as $option_name => $option) {
        if ($option['context'] == 'type') {
          $item->options['purger'][$option_name] = $form_state['values']['purger_' . $option_name];
        }
      }
      break;
  }
  return $bundle;
}

/**
 * Helper function to check if a name exists in the current configuration.
 */
function purge_item_name_exists($item_name) {
  $error = 0;
  // Get a basic bundle.
  $bundle = new PurgeBundleAPI();
  foreach($bundle->type as $type_name => $type) {
    if ($type_name == $item_name) {
      $error = 1;
    }
    foreach($bundle->{$type_name} as $name => $item) {
      if ($item_name == $name) {
        $error = 1;
      }
    }
  }
  return $error;
}

