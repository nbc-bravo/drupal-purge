<?php

/**
 * @file
 *
 * Contains all class and interface definitions for Purge.
 */


/**
 * Validates the item.
 *
 * The item can validates it's own settings.
 */
interface PurgeValidateable {
  /**
   * Validates the item.
   *
   * @return array $errors
   */
  function validate();
}

/**
 * Makes the item dependable on other items, modules or custom requirements.
 *
 * When the item fails its dependency check it cannot be enabled and will be
 * disabled on the next validation.
 */
interface PurgeDependable extends PurgeValidateable {

}

/**
 * Configures the item.
 */
interface PurgeConfigurable {
  function configure();
}

/**
 * Defines cacheable items.
 *
 * Cacheable items don't have initial values. This interface sets the item
 * properties with a value that is cached for reuse.
 */
interface PurgeCacheable {
  function cache();
}

/**
 * Defines processable items.
 *
 * Processable items take a Purgeable (URL, domain or tag) to set their
 * properties for each purgable it's used with.
 */
interface PurgeProcessable {
  /**
   * Processes a purgeable.
   *
   * @param object $purgeable
   */
  // function process($purgeable);

}

/**
 * Defines URL processable items.
 *
 * URL processable items take a Purgeable URL to set their properties.
 */
interface PurgeProcessableUrl extends PurgeProcessable {
  function process_url($purgeable_url);
}

/**
 * Defines domain processable items.
 *
 * Domain processable items take a Purgeable domain to set their properties.
 */
interface PurgeProcessableDomain extends PurgeProcessable {
  function process_domain($purgeable_domain);
}

/**
 * Defines tag processable items.
 *
 * Domain processable items take a Purgeable tag to set their properties.
 */
interface PurgeProcessableTag {
  function process_tag($purgeable_tag);
}

/**
 * Class for purge configuration management.
 *
 * Defines a basic configuration item.
 */
abstract class Purge {
  public $name = '';
  public $description = '';
  public $available = 1;
  public $enabled = 1;
  public $access = array(PURGE_ACCESS_FULL);
  public $item = array();
  public $depend = array();
  public $option = array();

  /**
   * Only serialize the static values.
   */
  public function __sleep() {
    $return_properties = array();
    $basic_properties = array('name', 'description', 'enabled', 'access');
    $return_properties = $basic_properties;

    // Build the static item array so we don't have to serialize all objects.
    if (count($this->item)) {
      // Make sure the item array is serialized once finished.
      $return_properties[] = 'item';
      //
      foreach ($this->item as $type_name => $items) {
        // Check we're parsing an option and this is a Purger.
        if ($this instanceof PurgePurger && $type_name = 'option') {
          // Store the option values in this purges item array.
          $this->item['option'] = array();
          foreach ($this->option as $option_name => $option) {
            $this->item['option'][$option_name] = array();
            // Set the current value of the option in the item array.
            $this->item['option'][$option_name] = $option->value;
          }
        }
        else {
          $this->item[$type_name] = array();
          // .
          foreach ($items as $item_name => $item) {
            $this->item[$type_name][] = $item_name;
          }
        }
      }
    }

    return $return_properties;
  }

  /**
   * Does basic dependency checks for builtin dependencies.
   *
   */
  public function validate() {
    $errors = array();
    // Check if we're a Dependable item or if we have dependencies in the items.
    if ($this instanceof PurgeDependable || count($this->item['depend']) > 0) {
      // Check if we have depend objects on board.
      if (count($this->depend) > 0) {
        foreach ($this->depend as $depend_name => $depend) {
          $depend_errors = array();
          $depend_errors = $depend->validate();
          foreach ($depend_errors as $depend_error) {
            $errors[] = $depend_error;
          }
        }
      }
    }
    return $errors;
  }
}

/**
 * Class definition for the Types object class.
 *
 * Type objects are used to define all the items managed by the Purge module.
 * These objects can be manipulated and extended for advanced uses.
 */
class PurgeItem extends Purge {

}

/**
 * Declares dependancies of the dependable items.
 */
class PurgeDepend extends Purge implements PurgeValidateable {

  /**
   * Validates the dependency.
   */
  public function validate() {
    $errors = array();
    return $errors;

  }
}

/**
 * Checks if a module is enabled.
 */
class PurgeDependModule extends PurgeDepend {
  public $modules = array();

  /**
   * Constructs a module dependency from an array of module.
   *
   * @param array $modulenames
   *
   */
  // public function __construct($module_names) {
  //  foreach($module_names as $module_name) {
      // @TODO: Check if things change and call reconfigure if needed.
  //    $this->modules[$module_name] = module_exists($module_name);
  //  }
 // }

  /**
   * Checks if the modules are installed and enabled.
   *
   * @param string $module_name
   */
  public function check_depend($module_name = NULL) {
    $pass = 0;
    if (!is_null($module_name)) {
      $modules = array($module_name => 0);
    }
    else {
      $modules = $this->modules;
    }
    foreach ($modules as $module_name => $module_status) {

      if (module_exists($module_name)) {
        $this->module_status[$module_name] = 1;
        $pass = 1;
      }
    }
    return $pass;
  }
}

/**
 * Checks if the curl library is installed.
 */
class PurgeDependCurl extends PurgeDepend {

  /**
   * Declares the dependency on the curl library.
   */
  public function depends() {
    $dependables = array(
      'custom' => array(
        'curl' => array(
          'name' => t('The Curl library for PHP.'),
          'description' => t('Install the curl library.'),
          'help_link' => 'http://php.net/manual/en/book.curl.php',
        ),
      ),
    );
    return $dependables;
  }


  /**
   * Checks if the curl library is installed.
   */
  public function check_depend() {
    $pass = (extension_loaded('curl'));
    return $pass;
  }
}

/**
 * Defines options for items.
 */
class PurgeOption extends Purge {
  public $value;
}


/**
 * Abstract Class definition for Purge Targets.
 *
 * Targets are the destination of the purge requests. Typically a reverse proxy
 * server or an API endpoint.
 */
abstract class PurgeTarget extends Purge implements PurgeValidateable {
  public $urls = array();

  /**
   * Validate
   *
   * @return $errors
   */
  public function validate() {
    $errors = array();
    // Validate the URLs
    foreach ($this->urls as $url) {
      if (!valid_url($url, TRUE)) {
        $url = check_plain($url);
        $errors[] = array(
          'name' => 'urls',
          'message' => t('Target URL @url in target @name is an invalid target URL.',
            array('@url' => $url, '@name' => $this->name))
        );
      }
    }
    return $errors;
  }
}

/**
 * Class definition for static targets.
 */
class PurgeTargetStatic extends PurgeTarget implements PurgeValidateable {

}


/**
 * Class definition for the Drupal Target.
 */
class PurgeTargetDrupal extends PurgeTarget implements PurgeCacheable {

  /**
   * Set api data to url.
   */
  public function cache() {
    $base_url = $GLOBALS['base_url'];
    $this->urls = array($base_url);
  }
}

/**
 * Class definition for domains.
 */
abstract class PurgeDomain extends Purge {
  public $domains = array();

  /**
   * Validate the domains.
   */
  public function validate() {
    $errors = array();
    foreach ($this->domains as $domain) {
      $pieces = explode(".",$domain);
      foreach($pieces as $piece) {
        if (!preg_match('/^[a-z\d][a-z\d-]{0,62}$/i', $piece) || preg_match('/-$/', $piece) ) {
          $errors[] = array(
            'name' => 'domains',
            'message' => t('Invalid domain'),
          );
        }
        return $errors;
      }
    }
  }
}

/**
 * Class definition for static domain items.
 */
class PurgeDomainStatic extends PurgeDomain {

}

/**
 * Used the domain supplied by the purgeable url.
 */
class PurgeDomainPurgeableUrl extends PurgeDomain implements PurgeProcessableUrl {

  /**
   * Processes the Purgeable URL.
   */
  public function process($purgeable) {

  }

  /**
   * Caches the domain untill the end of this processing.
   */
  public function cache() {

  }

  /**
   * Get the domain name from the url and set it as the current domains value.
   */
  public function process_url($url) {
    // Whipe the current value.
    $this->domains = array();
    // parse the url.
    $this->domain[] = parse_url($url, PHP_URL_HOST);
  }
}


/**
 * Class definition for the Drupal domain item.
 */
class PurgeDomainDrupal extends PurgeDomain implements PurgeCacheable {

  /**
   * Set the domain to the base urls domain.
   */
  public function cache() {
    $base_url = $GLOBALS['base_url'];
    $this->domains = array(parse_url($base_url, PHP_URL_HOST));
  }
}

/**
 * Abstract class definition for Purge Headers.
 */
abstract class PurgeHeader extends Purge {
  public $headers = array();
}

/**
 * Class definition for static header items.
 */
class PurgeHeaderStatic extends PurgeHeader {

}


/**
 * Class definition for Queues.
 */
abstract class PurgeQueue extends Purge {

}

/**
 * Class definition for the Fake Queue.
 *   This will issue all requests in the current process.
 */
class PurgeQueueFake extends PurgeQueue {
  public $queue = array();

  /**
   * Calls the BundleQueue object.
   */
  public function process() {
    $queue_bundle = new PurgeBundleQueue($this->queue);
    $queue_bundle->execute($this->queue);
  }
}

/**
 * Use the Queue Runner Queue.
 *
 * Uses the Queue Runner module to off load purges to a daemon, allowing
 * continues and un-delayed purging.
 */
class PurgeQueueRunner extends PurgeQueue implements PurgeDependable {
  public $queue = array();

  /**
   * Processes the queue.
   */
  public function process() {
    // Create a queuerunner task for each queue item.
    $task = array();
    $task['title'] = 'Purge Task';
    $task['data'] = $this->queue;

    queue_runner_add($task, 'purge_worker_callback');
  }

  /**
   * Validates all depedencies.
   */
  public function validate() {
    $errors = array();
    $pass = $this->check_depend();
    if (!$pass) {
      $dependables = $this->depends();
      foreach ($dependable as $dependable_name => $dependable) {
        if ($dependable['pass'] == 0) {
          $errors[] = array(
            'name' => $dependable_name,
            'message' => $dependable['description'],
          );
        }
      }
    }
    return $errors;
  }
}


/**
 * Class definition for Handlers.
 */
abstract class PurgeHandler extends Purge {

}

/**
 * Class definition for the Drupal HTTP request handler.
 */
class PurgeHandlerDrupal extends PurgeHandler {

  /**
   * Function to execute items from a queue.
   *
   * @param array $purges
   *
   * @param array $handler_options
   *
   * @return array $purges
   */
  public function execute($purges, $handler_options) {
    foreach ($purges as $purge_id => $purge) {
      $options = array(
        'headers' => $purge['headers'],
        'method' => $handler_options['method'],
      );
      $results = drupal_http_request($purge['purge_url'],$options);
      $purges[$purge_id]['http_code'] = 200; // $results['code'];
    }
    return $purges;
  }
}

/**
 * Class definition for the Curl HTTP request library.
 */
class PurgeHandlerCurl extends PurgeHandler implements PurgeDependable {

  /**
   * Funtion will determine what options are set and call the right execute
   * function.
   */
  public function execute($purges, $handler_options) {
    if ($handler_options['multi']) {
      //if ($handler_options['legacy']) {
      //  $purges = $this->execute_legacy($purges, $handler_options);
      //}
      //else {
        $purges = $this->execute_multi($purges, $handler_options);
      // }
    }
    else {
      $purges = $this->execute_single($purges, $handler_options);
    }
    return $purges;
  }

  /**
   * Function to procerss a single purge with curl.
   */
  private function execute_single($purges, $handler_options) {
    foreach ($purges as $purge_id => $purge) {
      $curl_request = $this->get_curl_request($purge, $handler_options);
      // Execute the request
      curl_exec($curl_request);
      // Get http status code.
      $info = curl_getinfo($curl_request);
      $purges[$purge_id]['http_code'] = $info['http_code'];
    }
    return $purges;
  }

  /**
   * Function to process multiple purges with curl.
   */
  private function execute_multi($purges, $handler_options) {
    $curl_purges = curl_multi_init();
    foreach ($purges as $purge_id => $purge) {
      $curl_requests[$purge_id] = $this->get_curl_request($purge, $handler_options);
      curl_multi_add_handle($curl_purges, $curl_requests[$purge_id]);
    }
    // Execute the purge requests
    ob_start();
    $active = 0;
    $multi_result = $this->multi_exec($curl_purges, $active);
    while ($active && $multi_result == CURLM_OK) {
      if (curl_multi_select($curl_purges) == -1) {
        usleep(100000);
      }
      $multi_result = $this->multi_exec($curl_purges, $active);
    }
    ob_end_clean();

    // Result collection. Collects the http code returned for each url purged
    foreach ($purges as $purge_id => $purge) {
      $info = curl_getinfo($curl_requests[$purge_id]);
      $purges[$purge_id]['http_code'] = $info['http_code'];
      curl_multi_remove_handle($curl_purges, $curl_requests[$purge_id]);
    }
    curl_multi_close($curl_purges);
    return $purges;
  }

  /**
    * Wrapper for curl_multi_exec().
    */
  private function multi_exec($curl_purges, &$active) {
    do {
      $multi_result = curl_multi_exec($curl_purges, $active);
    } while ($multi_result == CURLM_CALL_MULTI_PERFORM);
    return $multi_result;
  }



  /**
   * Function to construct a curl request object.
   */
  private function get_curl_request($purge, $handler_options) {
    $headers = array();
    foreach ($purge['headers'] as $header_key => $header_value) {
      $headers[] = $header_key . ": " . $header_value;
    }
    $curl_request = curl_init();
    curl_setopt($curl_request, CURLOPT_CUSTOMREQUEST, $handler_options['method']);
    curl_setopt($curl_request, CURLOPT_URL, $purge['purge_url']);
    curl_setopt($curl_request, CURLOPT_HEADER, 1);
    curl_setopt($curl_request, CURLOPT_HTTPHEADER, $purge['headers']);
    curl_setopt($curl_request, CURLOPT_RETURNTRANSFER, 0);

    return $curl_request;
  }

  /**
   * Checks if the curl library is installed.
   */
  public function check_depend() {
    $pass = (extension_loaded('curl'));
    return $pass;
  }
}

/**
 * Class definition for HTTPrl handler.
 */
class PurgeHandlerHttprl extends PurgeHandler {

  public function execute($purges, $handler_options) {
    foreach ($purges as $purge_id => $purge) {
      $headers_array = headers2array($purge['headers']);
      // Gather options
      $options = array(
        'method' => $handler_options['method'],
        'blocking' => FALSE,
        'headers' => $purge['headers'],
      );
      // Queue the request
      httprl_request($purge['purge_url'], $options);
      // Since we don't care about the results, let's fake 200
      $purges[$purge_id]['http_code'] = 200;
    }
    // Issue all  requests.
    $request = httprl_send_request();
    return $purges;
  }
}

/**
 * Class definition for Purgers.
 */
abstract class PurgePurger extends Purge {
  public $target = array();
  public $domain = array();
  public $header = array();
  public $queue = array();
  public $handler = array();

}

/**
 * Class definition for the Basic Url Purger.
 */
class PurgePurgerUrl extends PurgePurger implements PurgeProcessableUrl {
  public $purger = array();

  /**
   * Process purgeables.
   *
   * @param array $purgeablea_url
   *
   * @return array $queue_item
   */
  public function process_url($purgeable_url) {
    $purges = array();
    // First set all processable items with the purgeable data.
    foreach ($this->pointers as $pointer_type => $pointers) {
      foreach ($pointers as $pointer_name) {
        if ($pointer_type != 'handler') {
          if ($this->{$pointer_type}[$pointer_name] instanceof PurgeProcessableUrl) {
            $this->{$pointer_type}[$pointer_name]->process_url($purgeable_url);
          }
        }
      }
      // gather targets.
      $target_urls = array();
      foreach ($this->target as $target_name => $target) {
        foreach ($target->urls as $url) {
          $target_urls[] = $url;
        }
      }
      // Gather domains.
      $domains = array();
      foreach ($this->domain as $domain_name => $domain) {
        foreach ($domain->domains as $domain_domain) {
          $domains[] = $domain_domain;
        }
      }
      if ($this->options['domain']['purgeable_url_domain']) {
        $domains[] = parse_url($purgeable, PHP_URL_HOST);
      }
      // Gather headers.
      $headers = array();
      foreach ($this->header as $header_name => $header) {
        foreach ($header->headers as $header_key => $header_value) {
          $headers[$header_key] = $header_value;
        }
      }
      // Now lets fill the purges array
      $parsed_purgeable = parse_url($purgeable_url);
      foreach ($target_urls as $target_url) {
        foreach ($domains as $domain) {
          $purge_headers = $headers;
          // Add the host header.
          $purge_headers['Host'] = $domain;
          $purges[] = array(
            'purge_url' => $target_url . $parsed_purgeable['path'],
            'headers' => $purge_headers,
          );
        }
      }

    }
    // Get the hanlder and its options.
    $handler = $this->pointers['handler'][0];
    $handler_options = array();
    $handler_options = $this->options['handler'];

    $queue_item = array (
      'handler' => $handler,
      'handler_options' => $handler_options,
      'purges' => $purges,
    );
    $queue_name = $this->pointers['queue'][0];
    $this->queue[$queue_name]->queue[] = $queue_item;
  }
}

/**
 * Class definition for basic bundles.
 *
 * Bundles are objects containing purposed set of properties and methods,
 * depending on its subclass.
 *
 * This is a basic parent class.
 */
abstract class PurgePurgerBundle extends PurgePurger {
  public $type = array();

  /**
   * Override for parents sleep to make sure we include the bundled objects.
   */
  public function __sleep() {
    // First let call the parent function.
    $return_properties = parent::__sleep();
    // Add each type to the item array.
    foreach ($this->item as $type_name => $items) {
      if (!isset($return_properties[$type_name])) {
        $return_properties[] = $type_name;
      }
    }
    return $return_properties;
  }

  /**
   * Set the item objects linked in this bundle when waking up.
   */
  public function __wakeup() {
    // Each item in the item array.
    foreach ($this->item as $type_name => $items) {
      foreach ($this->{$type_name} as $bundle_item_name => $bundle_item) {
        // Check if the item contains items.
        if (isset($this->{$type_name}[$bundle_item_name]->item)) {
          foreach ($this->{$type_name}[$bundle_item_name]->item as $item_type => $item_names) {
            foreach ($item_names as $item_name) {
              // Set the item to the bundle item.
              // dprint_r($this->{$type_name}[$bundle_item_name]->{$item_type});
              // if$this->{$type_name}[$bundle_item_name]->{$item_type} = array();

              $this->{$type_name}[$bundle_item_name]->{$item_type}[$item_name] = $this->{$item_type}[$item_name];
            }
          }
        }
      }
    }
  }



  /**
   * Function to select data from the dataset.
   *
   * @return array $data
   */
  public function get_data() {
    // Get the data from the given dataset.
    $data = array();
    $data = variable_get('purge_config', NULL);

    return $data;
  }

  /**
   * Function to set the type objects.
   */
  public function set_types($data) {
    // Get the datastructure of the types up.
    foreach ($data['type'] as $type_name => $type_data) {
      // And create an object for each type
      $this->type[$type_name] = unserialize($type_data);
    }
  }
}

/**
 * Provides a full bundle.
 *
 * The full bundle contains the complete purge module configuration. Used for
 * management and operations that require all items, including disabled ones.
 */
class PurgePurgerBundleAPI extends PurgePurgerBundle implements PurgeValidateable {

  /**
   * Constructs the full bundle.
   */
  public function __construct() {
    // Get the data from the given dataset.
    $select_data = $this->get_data();
    // Then get the datastructure of the types up.
    foreach ($select_data['type'] as $type_name => $type_data) {
      $this->type[$type_name] = unserialize($type_data);
      // And create an object for each item
      foreach ($select_data[$type_name] as $item_name => $item_data) {
        $this->{$type_name}[$item_name] = unserialize($item_data);
      }
    }
    // Purgers need special handling
    foreach ($this->purger as $purger_name => $purger) {
      // Each type name that is a pointer.
      foreach ($purger->pointers as $pointer_type => $item_names ) {
        $this->purger[$purger_name]->{$pointer_type} = array();
      }
    }
  }

  /**
   * Merges another bundle into this one.
   *
   * @param object $merge_bundle
   *   A bundle, instance of PurgeBundleBasic.
   * @param bool $overwrite
   *   Overwrites with the merge bundle value should a duplicate be found.
   */
  public function merge($merge_bundle, $overwrite = 1) {
    // First check if this is actually another bundle.
    if (!$merge_bundle instanceof PurgePurgerBundle) {
      return;
    }
    // Loop through all types.
    foreach ($this->type as $type_name => $type) {
      // Walk the merge bundle item.
      foreach ($merge_bundle->{$type_name} as $item_name => $item) {
        // Check if the item exists in this bundle.
        if ($overwrite == 1 || !isset($this->{$type_name}[$item_name])) {
          // Set the merged item.
          $this->{$type_name}[$item_name] = clone $merge_bundle->{$type_name}[$item_name];
        }
      }
    }
  }


  /**
   * Validates the bundle.
   *
   * @return array $errors
   */
  public function validate() {
    $errors = array();
    foreach ($this->type as $type_name => $type) {
      foreach ($this->{$type_name} as $item) {
        if ($item instanceof PurgeValidateable) {
          $item_errors = $item->validate();
          if (count($item_errors) > 0) {
            foreach ($item_errors as $item_error) {
              $errors[] = $item_error;
            }
          }
        }
      }
    }
    return $errors;
  }

  /**
   * Saves the bundle to the variable.
   */
  public function save() {
    $data = array();
    // Serialize this bundle.
    $data = serialize($this);
    // Replace the object type in the serialized data with the default api
    // Strip off the first double quote (object name length)
    for ($i = 0; $i <= 1; $i++) {
      $data = ltrim($data, chr(34));
      $data = strstr($data, chr(34));
    }
    $data = 'O:20:"PurgePurgerBundleAPI' . $data;


    // Store the data in the variable
    variable_set('purge_config', $data);
  }
}

/**
 * Class definition for the runtime bundle.
 *   This class is used for processing the purge requests during runtine.
 */
class PurgePurgerBundleProcess extends PurgePurgerBundle {
  public $purgeables;

  /**
   * Constructor for the Configuration bundle.
   */
  public function __construct() {
    $runtime_data = array();
    // First check if the data is in cache.

    // If not, get from parent.
    $all_data = $this->get_data();

    // Create the types and keep the data for caching.
    $this->set_types($all_data);
    $runtime_data['type'] = $all_data['type'];

    // For loop through the set_data and collect active items.
    $data['purger'] = array();
    foreach ($all_data['purger'] as $purger_name => $purger_data) {
      // check if the purger is enabled.
      if (strpos($purger_data, "s:7:\"enabled\";i:1")) {
        $data['purger'][$purger_name] = $purger_data;
        // Create the purger object.
        $this->purger[$purger_name] = unserialize($purger_data);
        // Now loop through the pointers
        foreach ($this->purger[$purger_name]->pointers as $pointer_type => $item_names) {
          foreach ($item_names as $item_name) {
            $item_data = $all_data[$pointer_type][$item_name];
            // Check in the data if the item is enabled.
            if (strpos($item_data, "s:7:\"enabled\";i:1")) {
              // Check if the item object isn't allready there.
              if (isset($this->{$pointer_type}[$item_name])) {
                // If so just link the pointer to it.
                $this->purger[$purger_name]->{$pointer_type}[$item_name] = $this->{$pointer_type}[$item_name];
              }
              else {
                // Create the object from data
                $this->{$pointer_type}[$item_name] = unserialize($item_data);
                // Check if it item has cacheable data
                if ($this->{$pointer_type}[$item_name] instanceof PurgeCacheable) {
                  $this->{$pointer_type}[$item_name]->cache();
                }
                // And link the pointer
                $this->purger[$purger_name]->{$pointer_type}[$item_name] = $this->{$pointer_type}[$item_name];
                // And keep the item data
                $data[$pointer_type][$item_name] = $item_data;

              }
            }
          }
        }
      }
    }
  }

  /**
   * Process all requests.
   */
  public function process() {
    // Walk the purgeables.
    foreach ($this->purgeables as $purgeable_type_name => $purgeables) {
      foreach ($purgeables as $purgeable) {
        // Pass all purgers.
        foreach ($this->purger as $purger_name => $purger) {
          // Handle purgeable URLs.
          if ($purgeable_type_name == 'urls') {
            if ($this->purger[$purger_name] instanceof PurgeProcessableUrl) {
              // Process the Purgeable URL.
              $this->purger[$purger_name]->process_url($purgeable);
            }
          }
        }
      }
    }
    // Now process the queues.
    foreach ($this->queue as $queue) {
      //if (count($queue->queue) > 0) {
        $queue->process();
      // }
    }
  }
}

/**
 * Executes queued purges.
 *
 * This is a minimal bundle, containing just the handlers needed to
 * process all queued purges.
 */
class PurgePurgerBundleQueue extends PurgePurgerBundle {

  /**
   * Constructs the bundle from queue items and executes the purges.
   *
   * @param array $queue_items
   */
  public function __construct($queue_items) {
    // Get the raw data from the variable (or cache at a later stage).
    $all_data = $this->get_data();

    // Figure out which handlers we'll need for these purges.
    foreach ($queue_items as $queue_item) {
      $handler_name = $queue_item['handler'];
      if (!isset($this->handler[$handler_name])) {
        // Create the object from the serialized data.
        $this->handler[$handler_name] = unserialize($all_data['handler'][$handler_name]);
      }
    }
  }

  /**
   * Executes all queueditems with their handlers.
   */
  public function execute($queue_items) {
    // Execute all queue items to the handlers.
    foreach ($queue_items as $queue_item) {
      $handler_name = $queue_item['handler'];
      $this->handler[$handler_name]->execute($queue_item['purges'], $queue_item['handler_options']);
    }
  }
}

/**
 * Contains items offered by modules implementing the Purge API.
 *
 */
abstract class PurgePurgerBundleContrib extends PurgePurgerBundle {

}
