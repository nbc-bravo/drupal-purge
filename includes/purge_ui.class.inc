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
    // $form[$form_name] = $this->get_form_edit_{$this->item_type};
    // Get the buttons.
    // $form['purge_action'] = $this->get_form_action();

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
    if ($this->action == 'add' || $this->action == 'clone') {
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
        '#default_value' => $this->item_name,
      );
    }
    $form['description'] = array(
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


    return $form;
  }

}
