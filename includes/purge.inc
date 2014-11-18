<?php

/**
 * @file
 *
 * Contains the main purging functionality and error handling.
 */

// Load class files
module_load_include('inc', 'purge', 'includes/purge.class');


/**
 * Constant definitions.
 */
// Config permissions.
define('PURGE_ACCESS_SYSTEM', 0);
define('PURGE_ACCESS_LIST', 1);
define('PURGE_ACCESS_ENABLE', 2);
define('PURGE_ACCESS_RENAME', 3);
define('PURGE_ACCESS_CLONE', 4);
define('PURGE_ACCESS_VIEW', 5);
define('PURGE_ACCESS_FULL', 6);


/**
 * Purges urls from reverse proxy caches
 *
 * @param $urls
 *   Array of urls to remove from the proxy cache using the http purge method.
 */
function purge_urls($urls) {
  $bundle = new PurgePurgerBundleProcess();
  $bundle->purgeables['urls'] =  $urls;
  $bundle->process();
}

/**
 * Executes queued purges.
 *
 * @param array $queue_items
 */
function purge_queue_execute($queue_items) {
  $bundle = new PurgePurgerBundleQueue($queue_items);
  $results = $bundle->execute($queue_items);
}

/**
 * (Re)Configure.
 *   Loads all API provided bundles.
 */
function purge_configure($api = NULL) {
  // Start out by fetching the currently configured bundle.
  $bundle = new PurgePurgerBundleAPI();

  // Invoke hook_purge_configure to get the api bundle.
  $api_modules = module_implements('purge_configure');
  foreach ($api_modules as $api_module) {
    $api_bundle = module_invoke($api_module, 'purge_configure', $api);
    // Merge the API bundle into the main one.
    $bundle->merge($api_bundle);
  }
  // Save the bundle.
  $bundle->save();
}


/**
 * Logs successful purges and errors to the watchdog.
 *
 * @param $purge_request_results
 *   array of url with their http status code
 */
function purge_logging($purge_request_results) {
  $purge_success = 0;
  $purge_blocking = array();
  $purge_log = array();
  foreach ($purge_request_results as $purge_request_result) {
    switch ($purge_request_result['http_code']) {
      // First check if everything went ok.
      case 200:
        $purge_log[] = $purge_request_result['purge_url'] . ' on ' . $purge_request_result['proxy_name'] . ' Ok';
        $purge_success++;
        break;
      // Notice if the request was not found in the proxy cache
      case 404:
        $purge_log[] = $purge_request_result['purge_url'] . ' on ' . $purge_request_result['proxy_name'] . ' Not Found';
        $purge_success++;
        break;
      // Collect all proxy hosts that are blocking the url requests
      case 405:
        $purge_log[] = $purge_request_result['purge_url'] . ' on ' . $purge_request_result['proxy_name'] . ' Forbidden';
        $purge_blocking[] = parse_url($purge_request_result['proxy_url'], PHP_URL_HOST);
        break;
      // Collect all urls and their http error codes
      default:
        $purge_log[] = $purge_request_result['purge_url'] . ' on '
          . $purge_request_result['proxy_name'] . ' ' . $purge_request_result['http_code'];
        break;
    }

  }

  // Watchdog barking
  $purge_errors = count($purge_request_results) - $purge_success;
  // Just a notice when all is ok
  if ($purge_errors == 0) {
    if ($purge_success == 1) {
      watchdog(
        'purge',
        '1 URL has been successfully purged from the reverse proxy caches: !purge_log',
        array('!purge_log' => expire_print_r($purge_log))
      );
    }
    else {
      watchdog(
        'purge',
        '!purge_success_count URLs have been successfully purged from the reverse proxy caches: !purge_log',
        array('!purge_success_count' => $purge_success, '!purge_log' => expire_print_r($purge_log))
      );
    }
  }
  else {
    // Report all urls with errors
    if ($purge_errors == 1) {
      watchdog(
        'purge',
        '1 error has been encountered when purging URLs !purge_log',
        array('!purge_log' => expire_print_r($purge_log)),
        $severity = WATCHDOG_ERROR
      );
    }
    else {
      watchdog(
        'purge',
        '!purge_errors_count errors have been encountered when purging these URLs. !purge_log',
        array('!purge_errors_count' => count($purge_errors), '!purge_log' => expire_print_r($purge_log)),
        $severity = WATCHDOG_ERROR
      );
    }
    // Report on proxy servers that block the purge requests.
    if (!empty($purge_blocking)) {
      foreach ($purge_blocking as $purge_blocking_server) {
        watchdog(
          'purge',
          'The proxy server host %blocking_server is blocking purge requests. Please review the proxy configuration.',
          array('%blocking_server' => $purge_blocking_server),
          $severity = WATCHDOG_ERROR
        );
      }
    }
  }
}

/**
 * Helper function to explode headers to an array
 */
function headers2array($headers) {
  $headers_array = array();
  foreach ($headers as $header) {
    $header_parts = explode(":", $header);
    $headers_array[$header_parts[0]] = $header_parts[1];
  }
  return $headers_array;
}
