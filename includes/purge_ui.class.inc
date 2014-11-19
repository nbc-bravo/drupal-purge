<?php

/**
 *  @file
 *   Provides administrative interface for the Purge module.
 */


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
        // Load the defaults.inc file
        module_load_include('inc', 'purge', 'includes/defaults');
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
          $this->form_item = new PurgePurgerURL();
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
    $function_name = 'get_form_edit_' . $this->item_type;
    $form[$form_name] = $this->{$function_name}();
   // $form[$form_name] = $this->get_form_edit_{$this->item_type}();
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
    $form['purge_item']['item_name'] = array(
      '#type' => 'textfield',
      '#title' => t('Name'),
      '#required' => TRUE,
      '#description' => t('The user-friendy name for the item.'),
      '#default_value' => $this->form_item->name,
    );
    if ($this->action == 'add' || $this->action == 'clone') {
      $form['purge_item']['machine_name'] = array(
        '#type' => 'machine_name',
        '#maxlength' => 21,
        '#default_value' => '',
        '#machine_name' => array(
          'exists' => 'purge_item_name_exists',
          'source' => array('purge_basic', 'purge_item', 'item_name'),
        ),
      );
    }
    else {
      $form['purge_item']['machine_name'] = array(
        '#type' => 'hidden',
        '#default_value' => $this->item_name,
      );
    }
    $form['purge_item']['description'] = array(
      '#type' => 'textfield',
      '#title' => t('Description'),
      '#default_value' => $this->form_item->description,
    );
    return $form;
  }

  /**
   * Generate the status form.
   */
  private function get_form_edit_status() {
    $form = array();
    $form['enabled'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enabled'),
      '#description' => t('Disabled items will ignored during operations, even when included in purgers.'),
      '#default_value' => $this->form_item->enabled,
    );
    return $form;
  }

  /**
   * Generate the target form elements.
   */
  private function get_form_edit_target() {
    $form = array();
    $form['target'] = array(
      '#type' => 'fieldset',
      '#title' => 'Target Configuration',
    );
    $form['target']['urls'] = array(
      '#type' => 'textarea',
      '#title' => t('URLs'),
      '#description' => t('The URLs where the Purge requests will be sent to. One URL per line.'),
      '#required' => true,
      '#default_value' => implode("\n", $this->form_item->urls),
    );
    return $form;
  }

  /**
   * Generate the domain form elements.
   */
  private function get_form_edit_domain() {
    $form = array();
    $form['domain'] = array(
      '#type' => 'fieldset',
      '#title' => 'Domain Configuration',
    );
    $form['domain']['domains'] = array(
      '#type' => 'textarea',
      '#title' => t('Domains'),
      '#description' => t('For each domain name Purge sends out a request. One domain per line.'),
      '#required' => true,
      '#default_value' => implode("\n", $this->form_item->domains),
    );
    return $form;
  }

  /**
   * Generate the header form elements.
   */
  private function get_form_edit_header() {
    $form = array();
    $form['header'] = array(
      '#type' => 'fieldset',
      '#title' => 'Header Configuration',
    );
    $headers = array();
    foreach ($this->form_item->headers as $header_key => $header_value) {
      $headers[] = implode(": ", array($header_key, $header_value));
    }
    $form['header']['headers'] = array(
      '#type' => 'textarea',
      '#title' => t('Header'),
      '#description' => t('Headers are added to each request. One header per line. Format: Header: value'),
      '#required' => true,
      '#default_value' => implode("\n", $headers),
    );
    return $form;
  }

  /**
   * Generate the purger form elements.
   */
  private function get_form_edit_purger() {
    $form = array();
    $form['purger'] = array(
      '#type' => 'fieldset',
      '#title' => 'Purger configuration',
    );
    $select_types = array('target', 'domain', 'header', 'queue', 'handler', 'depend');
    foreach ($select_types as $pointer_type) {
      $form['purger'][$pointer_type] = $this->get_select($pointer_type);
    }
    return $form;
  }

  /** Get an item selection form.
   *  @param $pointer_type string
   */
  private function get_select($pointer_type) {
    // Check if we select a single or multiple values.
    $multiple = true;
    if ($pointer_type == 'handler' || $pointer_type == 'queue') {
      $multiple = false;
    }
    $form = array();
    // Common header
    $table_header = array(
      'name' => t('Name'),
      'description' => t('Description'),
    );
    $default_value = array();
    foreach ($this->{$pointer_type} as $pointer_name => $pointer) {
      // Get name and description from the item object.
      $rows[$pointer_name] = array(
        'name' => $this->{$pointer_type}[$pointer_name]->name,
        'description' => $this->{$pointer_type}[$pointer_name]->description,
      );
      // Add to the default value if selected.
      if (in_array($pointer_name, $this->form_item->pointers[$pointer_type])) {
        if ($multiple) {
          $default_value[$pointer_name] = $pointer_name;
        }
        else {
          $default_value = $pointer_name;
        }
      }
    }
    // Now lets build the form.
    $form[$pointer_type] = array(
      '#type' => 'fieldset',
      '#title' => $this->type[$pointer_type]->name,
      '#description' => $this->type[$pointer_type]->description,
    );
    $form[$pointer_type][$pointer_type . '_selection'] = array(
      '#type' => 'tableselect',
      '#header' => $table_header,
      '#options' => $rows,
      '#default_value' => $default_value,
      '#multiple' => $multiple,
    );
    return $form;
  }








  /**
   * Generate the submit buttons.
   */
  private function get_form_action() {
    $form = array();
    // Save button.
    if ($this->action != 'view' && in_array(PURGE_ACCESS_FULL, $this->form_item->access)) {
      $form['submit']  = array(
        '#type' => 'submit',
        '#value' => t('Save'),
      );
      if ($this->form_item instanceof PurgeValidateable) {
        $form['validate'] = array(
          '#type' => 'button',
          '#value' => t('Validate'),
          // '#submit' => array('purge_item_form_validate'),
        );
      }
    }
    // Enable/disable button.
    if (in_array(PURGE_ACCESS_ENABLE, $this->form_item->access) || in_array(PURGE_ACCESS_FULL, $this->form_item->access)) {
      if ($this->form_item->enabled) {
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
    // Delete button.
    if (in_array(PURGE_ACCESS_FULL, $this->form_item->access) && ($this->action == 'edit')) {
      $form['delete'] = array(
        '#type' => 'submit',
        '#name' => 'delete',
        '#value' => t('Delete'),
        '#submit' => array('purge_form_button_callback'),
      );
    }
    return $form;
  }

}
