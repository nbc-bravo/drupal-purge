<?php

/**
 *  @file
 *   Provides default configuration and handlers or the Purge module.
 */

// Load the purge.inc file
// module_load_include('inc', 'purge');

/**
 * Class definition for the Defaults Bundle.
 */
class PurgeBundleDefault extends PurgeBundleBasic {

  /**
   * Construct the default bundle.
   */
  public function __construct() {
    // First lets get the types defined.
    // Target Type definition.
    $this->type['target'] = new PurgeItemType();
    $this->type['target']->name = t('Target');
    $this->type['target']->description = t('Targets are the URLs where the Purge requests will be sent to.');
    $this->type['target']->enabled = 1;
    $this->type['target']->access = PURGE_ACCESS_SYSTEM;

    // Domain type definition.
    $this->type['domain'] = new PurgeItemType();
    $this->type['domain']->name = t('Domain');
    $this->type['domain']->description = t('Domains to generate Purge requests with.');
    $this->type['domain']->enabled = 1;
    $this->type['domain']->access = PURGE_ACCESS_SYSTEM;

    // Header type definition.
    $this->type['header'] = new PurgeItemType();
    $this->type['header']->name = t('Header');
    $this->type['header']->description = t('HTTP Headers added to the requests.');
    $this->type['header']->enabled = 1;
    $this->type['header']->access = PURGE_ACCESS_SYSTEM;

    // Handler type definition.
    $this->type['handler'] = new PurgeItemType();
    $this->type['handler']->name = t('Handler');
    $this->type['handler']->description = t('The different http request handlers supported.');
    $this->type['handler']->enabled = 0;
    $this->type['handler']->access = PURGE_ACCESS_SYSTEM;

    // Batch type definition.
//    $this->type['batch'] = new PurgeItemType();
//    $this->type['batch']->name = t('Batch');
//    $this->type['batch']->description = t('Batch processing handlers to process Purge requests in bulk.');
//    $this->type['batch']->enabled = 0;
//    $this->type['batch']->access = PURGE_ACCESS_SYSTEM;

    // Option type definition.
    $this->type['option'] = new PurgeItemType();
    $this->type['option']->name = t('Option');
    $this->type['option']->description = t('Additional configuration options.');
    $this->type['option']->enabled = 1;
    $this->type['option']->access = PURGE_ACCESS_SYSTEM;

    // Set type definition
    $this->type['set'] = new PurgeItemType();
    $this->type['set']->name = t('Set');
    $this->type['set']->description = t('Sets are combination of all item types.');
    $this->type['set']->enabled = 1;
    $this->type['set']->access = PURGE_ACCESS_SYSTEM;

    // Target definitions.
    // The Drupal base url as target
    $this->target['drupal_url'] = new PurgeItemTarget();
    $this->target['drupal_url']->name = t('Drupal URL');
    $this->target['drupal_url']->description = t('Use the Drupal Base URL as a target.');
    $this->target['drupal_url']->enabled = 1;
    $this->target['drupal_url']->access = PURGE_ACCESS_ENABLE;
    $this->target['drupal_url']->api = array(
      'callback' => 'purge_target_drupal_url',
      'provider' => 'purge',
      'context' => PURGE_API_RUNTIME,
    );
    $this->target['drupal_url']->urls = array();

    // The localhost target
    $this->target['localhost'] = new PurgeItemTarget();
    $this->target['localhost']->name = t('Localhost');
    $this->target['localhost']->description = t('The default localhost ip address.');
    $this->target['localhost']->enabled = 1;
    $this->target['localhost']->access = PURGE_ACCESS_FULL;
    $this->target['localhost']->urls = array('http://127.0.0.1/');


    // Domain definition.
    // The domain from the Drupal Basd URL.
    $this->domain['drupal_domain'] = new PurgeItemDomain();
    $this->domain['drupal_domain']->name = t('Drupal Domain');
    $this->domain['drupal_domain']->description = t('Use the domain in the Drupal Base URL.');
    $this->domain['drupal_domain']->enabled = 1;
    $this->domain['drupal_domain']->access = PURGE_ACCESS_ENABLE;
    $this->domain['drupal_domain']->api = array(
      'callback' => 'purge_domain_drupal_domain',
      'provider' => 'purge',
      'context' => PURGE_API_RUNTIME,
    );
    $this->domain['drupal_domain']->domains = array();

    // The domains provided by expire.
    $this->domain['expire_domain'] = new PurgeItemSet();
    $this->domain['expire_domain']->name = t('Expire Domain');
    $this->domain['expire_domain']->description = t('Use the domain provided by the Expire integration hook.');
    $this->domain['expire_domain']->enabled = 1;
    $this->domain['expire_domain']->access = PURGE_ACCESS_ENABLE;
    $this->domain['expire_domain']->api = array(
      'callback' => 'purge_domain_expire_domain',
      'provider' => 'purge',
      'context' => PURGE_API_RUNTIME,
    );
    $this->domain['drupal_domain']->domains = array();


    // Header definitions.
    // The Gzip header.
    $this->header['gzip'] = new PurgeItemHeader();
    $this->header['gzip']->name = t('Gzip');
    $this->header['gzip']->description = t(
      'The Gzip header enables compression and is enabled by default on most http clients.'
    );
    $this->header['gzip']->enabled = 1;
    $this->header['gzip']->access = PURGE_ACCESS_ENABLE;
    $this->header['gzip']->api = array(
      'callback' => 'purge_domain_expire_domain',
      'provider' => 'purge',
      'context' => PURGE_API_STATIC,
    );
    $this->header['gzip']->headers = array('Accept-Encoding' => 'gzip');


    // Handler definitions.
    // Drupal http request.
    $this->handler['drupal_http_request'] = new PurgeItemHandler();
    $this->handler['drupal_http_request']->name = t('Drupal HTTP Request');
    $this->handler['drupal_http_request']->description = t('The reuest handler builtin to Drupal.');
    $this->handler['drupal_http_request']->enabled = 1;
    $this->handler['drupal_http_request']->access = PURGE_ACCESS_ENABLE;
    $this->handler['drupal_http_request']->api = array(
      'callback' => 'purge_hanler_drupal_http_request',
      'provider' => 'purge',
      'context' => PURGE_API_RUNTIME,
    );
    $this->handler['drupal_http_request']->features = array(PURGE_HANDLER_SINGLE);

    // Curl Single Handler
    $this->handler['curl_single'] = new PurgeItemHandler();
    $this->handler['curl_single']->name = t('Curl Single');
    $this->handler['curl_single']->description = t('Use the Curl library to send out requests one at a time.');
    $this->handler['curl_single']->enabled = 1;
    $this->handler['curl_single']->access = PURGE_ACCESS_ENABLE;
    $this->handler['curl_single']->api = array(
      'callback' => 'purge_handler_curl_single',
      'provider' => 'purge',
      'context' => PURGE_API_RUNTIME,
    );
    $this->handler['curl_single']->features = array(PURGE_HANDLER_SINGLE);

    // Curl Multi Legacy Handler
    $this->handler['curl_multi_legacy'] = new PurgeItemHandler();
    $this->handler['curl_multi_legacy']->name = t('Curl Multi (Legacy)');
    $this->handler['curl_multi_legacy']->description = t(
      'Providers legacy support for older versions of PHP and Curl.'
    );
    $this->handler['curl_multi_legacy']->enabled = 1;
    $this->handler['curl_multi_legacy']->access = PURGE_ACCESS_ENABLE;
    $this->handler['curl_multi_legacy']->api = array(
      'callback' => 'purge_handler_curl_multi_legacy',
      'provider' => 'purge',
      'context' => PURGE_API_RUNTIME,
    );
    $this->handler['curl_multi_legacy']->features = array(PURGE_HANDLER_MULTI);

    // Curl Multi Handler.
    $this->handler['curl_multi'] = new PurgeItemHandler();
    $this->handler['curl_multi']->name = t('Curl Multi');
    $this->handler['curl_multi']->description = t(
      'Curl is an advanced HTTP request library and available on most hosting environments.'
    );
    $this->handler['curl_multi']->enabled = 1;
    $this->handler['curl_multi']->access = PURGE_ACCESS_ENABLE;
    $this->handler['curl_multi']->api = array(
      'callback' => 'purge_handler_curl_multi',
      'provider' => 'purge',
      'context' => PURGE_API_RUNTIME,
    );
    $this->handler['curl_multi']->features = array(PURGE_HANDLER_MULTI);

    // HTTPrl Handler.
    $this->handler['httprl'] = new PurgeItemHandler();
    $this->handler['httprl']->name = t('HTTPrl');
    $this->handler['httprl']->description = t(
      'The HTTP Request Library module provides an advanced library that allows non-blocking requests.'
    );
    $this->handler['httprl']->enabled = 1;
    $this->handler['httprl']->access = PURGE_ACCESS_ENABLE;
    $this->handler['httprl']->api = array(
      'callback' => 'purge_handler_curl_multi',
      'provider' => 'purge',
      'context' => array(PURGE_API_RUNTIME),
    );
    $this->handler['httprl']->features = array(PURGE_HANDLER_MULTI, PURGE_HANDLER_NON_BLOCKING);


    // Batch definition.

    // Options.
    $this->option['default_options'] = new PurgeItemOption();
    $this->option['default_options']->name = t('Default Options');
    $this->option['default_options']->description = t('Defaults for all options.');
    $this->option['default_options']->enabled = 1;
    $this->option['default_options']->access = PURGE_ACCESS_CLONE;
    $this->option['default_options']->api = array(
      'callback' => 'purge_option_defaults',
      'provider' => 'purge',
      'context' => array(PURGE_API_STATIC),
    );
    $this->option['default_options']->options = array(
      'request_method' => 'PURGE',
      'watchdog' => 1,
    );

    // Now create the set objects
    $this->set['minimal_set'] = new PurgeItemSet();
    $this->set['minimal_set']->name = t('Minimal Set');
    $this->set['minimal_set']->description = t('Uses Drupal native methods to issue the requests.');
    $this->set['minimal_set']->enabled = 1;
    $this->set['minimal_set']->access = PURGE_ACCESS_CLONE;
    $this->set['minimal_set']->api = array(
      'callback' => 'purge_minimal_set',
      'provider' => 'purge',
      'context' => array(PURGE_API_RUNTIME),
    );
    $this->set['minimal_set']->pointers = array(
      'target' => array('drupal_url'),
      'domain' => array('drupal_domain'),
      'header' => array('gzip'),
      'handler' => array('drupal_http_request'),
      'option' => array('default_options'),
    );
    $this->set['minimal_set']->target = array('drupal_url' => $this->target['drupal_url']);
    $this->set['minimal_set']->domain = array('drupal_domain' => $this->domain['drupal_domain']);
    $this->set['minimal_set']->header = array('gzip' => $this->header['gzip']);
    $this->set['minimal_set']->handler = array('drupal_http_request' => $this->handler['drupal_http_request']);
    $this->set['minimal_set']->option = array('default_options' => $this->option['default_options']);

    // Now create the set objects
    $this->set['default_set'] = new PurgeItemSet();
    $this->set['default_set']->name = t('Default Set');
    $this->set['default_set']->description = t('Uses Drupal native methods to issue the requests.');
    $this->set['default_set']->enabled = 1;
    $this->set['default_set']->access = PURGE_ACCESS_CLONE;
    $this->set['default_set']->api = array(
      'callback' => 'purge_defaults',
      'provider' => 'purge',
      'context' => array(PURGE_API_RUNTIME),
    );
    $this->set['default_set']->pointers = array(
      'target' => array('drupal_url', 'localhost'),
      'domain' => array('drupal_domain'),
      'header' => array('gzip'),
      'handler' => array('drupal_http_request'),
      'option' => array('default_options'),
    );
    $this->set['default_set']->target = array(
      'drupal_url' => $this->target['drupal_url'],
      'localhost' => $this->target['localhost'],
    );
    $this->set['default_set']->domain = array('drupal_domain' => $this->domain['drupal_domain']);
    $this->set['default_set']->header = array('gzip' => $this->header['gzip']);
    $this->set['default_set']->handler = array('drupal_http_request' => $this->handler['drupal_http_request']);
    $this->set['default_set']->option = array('default_options' => $this->option['default_options']);

  }

  /**
   * Save the Defaults.
   *   Will overwrite all existing configurations!
   *   Use reconfigure to restore the defaults but keep the other items.
   */
  public function save() {
    parent::save();
  }
}