<?php

/**
 *  @file
 *   Provides default configuration and handlers or the Purge module.
 */

/**
 * Class definition for the Defaults Bundle.
 */
class PurgePurgerBundleDefault extends PurgePurgerBundleAPI {

  /**
   * Construct the default bundle.
   */
  public function __construct() {
    // First declare an array with the static item declarations.
    $this->item = array(
      'type' => array('depend', 'option', 'target', 'domain', 'header', 'queue', 'handler', 'purger'),
      'depend' => array('module', 'curl'),
      'option' => array('method', 'parallel', 'non-blocking', 'batch'),
      'target' => array('drupal_url'),
      'domain' => array('drupal_domain'),
      'header' => array('gzip'),
      'queue' => array('fake', 'runner'),
      'handler' => array('drupal_http_request', 'curl', 'httprl'),
      'purger' => array(),
    );

    // Lets get the types defined.
    // First get dependecies up.
    $this->type['depend'] = new PurgeItem();
    $this->type['depend']->name = t('Dependency');
    $this->type['depend']->description = t('Declares conditions for the items availability.');

    // Option items.
    $this->type['option'] = new PurgeOption();
    $this->type['option']->name = t('Option');
    $this->type['option']->description = t('Adjust the configuration of items.');
    $this->type['option']->access = array(PURGE_ACCESS_LIST, PURGE_ACCESS_VIEW);

    // Target Type definition.
    $this->type['target'] = new PurgeItem();
    $this->type['target']->name = t('Target');
    $this->type['target']->description = t('Targets are the URLs where the Purge requests will be sent to.');

    // Domain type definition.
    $this->type['domain'] = new PurgeItem();
    $this->type['domain']->name = t('Domain');
    $this->type['domain']->description = t('Domains to generate Purge requests with.');

    // Header type definition.
    $this->type['header'] = new PurgeItem();
    $this->type['header']->name = t('Header');
    $this->type['header']->description = t('HTTP Headers added to the requests.');

    $this->type['queue'] = new PurgeItem();
    $this->type['queue']->name = t('Queue');
    $this->type['queue']->description = t('Queue Purge requests to handle large numbers of purges.');
    $this->type['queue']->access = array(PURGE_ACCESS_LIST,PURGE_ACCESS_ENABLE);

    // Handler type definition.
    $this->type['handler'] = new PurgeItem();
    $this->type['handler']->name = t('Handler');
    $this->type['handler']->description = t('The different http request handlers supported.');
    $this->type['handler']->access = array(PURGE_ACCESS_LIST,PURGE_ACCESS_ENABLE);

    // Purger type definition
    $this->type['purger'] = new PurgeItem();
    $this->type['purger']->name = t('Purger');
    $this->type['purger']->description = t('Purgers are the main configurable items in Purge and do the actual purging.');
    $this->type['purger']->access = array(PURGE_ACCESS_FULL);

    // Depend definitions.
    $this->depend['module'] = new PurgeDependModule();
    $this->depend['module']->name = t('Module Dependency');
    $this->depend['module']->description = t('Declares an items dependency on another Drupal module.');
    $this->depend['module']->access = array(PURGE_ACCESS_LIST);

    $this->depend['php_curl'] = new PurgeDependCurl();
    $this->depend['php_curl']->name = t('Curl Dependency');
    $this->depend['php_curl']->description = t('Checks if Curl is available as a PHP Extension.');

    // Options.

    $this->option['method'] = new PurgeOption();
    $this->option['method']->name = t('Method');
    $this->option['method']->description = t('The method used to perform the purge.');
    $this->option['method']->values = array(
      'PURGE' => 'HTTP request method PURGE',
      'BAN' => 'HTTP request method BAN.',
      'GET' => 'HTTP request method GET.',
    );
    $this->option['method']->default_value = 'PURGE';

    // Parallel Option
    $this->option['parallel'] = new PurgeOption();
    $this->option['parallel']->name = t('Parallel');
    $this->option['parallel']->description = t('Process purgeables in parallel');
    $this->option['parallel']->default_value = 1;


    // Non Blocking Option
    $this->option['non-blocking'] = new PurgeOption();
    $this->option['non-blocking']->name = t('Non Blocking');
    $this->option['non-blocking']->description = t(
      'Issue non-blocking requests. Enhances performance but disables error reporting and debugging.');
    $this->option['non-blocking']->default_value = 1;

    $this->option['batch'] = new PurgeOption();
    $this->option['batch']->name = t('Batch Size');
    $this->option['batch']->description = t('How many purge requests should be handled in one batch.');
    $this->option['batch']->default_value = 25;

    // $this->option[''] = new PurgeOption();
    // $this->option['']->name = t('');
    // $this->option['']->description = t('');
    // $this->option['']->values = array();
    // $this->option['']->default_value = ;

    // Target definitions.
    // The Drupal base url as target
    $this->target['drupal_url'] = new PurgeTargetDrupal();
    $this->target['drupal_url']->name = t('Drupal URL');
    $this->target['drupal_url']->description = t('Use the Drupal Base URL as a target.');
    $this->target['drupal_url']->access = array(PURGE_ACCESS_LIST,PURGE_ACCESS_ENABLE,PURGE_ACCESS_VIEW);
    $this->target['drupal_url']->urls = array();


    // Domain definition.
    // The domain from the Drupal Basd URL.
    $this->domain['drupal_domain'] = new PurgeDomainDrupal();
    $this->domain['drupal_domain']->name = t('Drupal Domain');
    $this->domain['drupal_domain']->description = t('Use the domain in the Drupal Base URL.');
    $this->domain['drupal_domain']->access = array(PURGE_ACCESS_LIST,PURGE_ACCESS_ENABLE,PURGE_ACCESS_VIEW);
    $this->domain['drupal_domain']->domains = array();

    $this->domain['purgeable_url_domain'] = new PurgeDomainPurgeableUrl();
    $this->domain['purgeable_url_domain']->name = t('Purgeable URL Domain');
    $this->domain['purgeable_url_domain']->description = t('Use the domain name provided by the Purgeable URL.');
    $this->domain['purgeable_url_domain']->access = array(PURGE_ACCESS_LIST,PURGE_ACCESS_ENABLE);


    // Header definitions.
    // The Gzip header.
    $this->header['gzip'] = new PurgeHeaderStatic();
    $this->header['gzip']->name = t('Gzip');
    $this->header['gzip']->description = t(
      'The Gzip header enables compression and is enabled by default on most http clients.'
    );
    $this->header['gzip']->access = array(PURGE_ACCESS_LIST,PURGE_ACCESS_CLONE,PURGE_ACCESS_VIEW);
    $this->header['gzip']->headers = array('Accept-Encoding' => 'gzip');


    // Queue definitions
    // The Fake Queue.
    $this->queue['fake'] = new PurgeQueueFake();
    $this->queue['fake']->name = t('No Queue');
    $this->queue['fake']->description = t('Do not queue the purges but process them right away.');
    $this->queue['fake']->access = array(PURGE_ACCESS_LIST,PURGE_ACCESS_ENABLE);

    // The QueueRunner Queue
    $this->queue['runner'] = new PurgeQueueRunner();
    $this->queue['runner']->name = 'Queue Runner';
    $this->queue['runner']->description = t('Use the Queue Runner Module to offload purges to a continues cli process.');
    $this->queue['runner']->available = 0;
    $this->queue['runner']->access = array(PURGE_ACCESS_LIST,PURGE_ACCESS_ENABLE);
    $this->queue['runner']->item = array(
      'depend' => array(
        'module' => array(
          'queue_runner' => 0,
        ),
      'option' => array(
        'batch' => '25',
        'parallel' => 1,
        ),
      ),
    );


    // Handler definitions.
    // Drupal http request.
    $this->handler['drupal_http_request'] = new PurgeHandlerDrupal();
    $this->handler['drupal_http_request']->name = t('Drupal HTTP Request');
    $this->handler['drupal_http_request']->description = t('The native http request function of Drupal.');
    $this->handler['drupal_http_request']->access = array(PURGE_ACCESS_LIST,PURGE_ACCESS_ENABLE);
    $this->handler['drupal_http_request']->item = array(
      'option' => array(
        'method' => 'PURGE',
      ),
    );

    // Curl Single Handler
    $this->handler['curl'] = new PurgeHandlerCurl();
    $this->handler['curl']->name = t('Curl');
    $this->handler['curl']->description = t('Use the Curl library to send out requests http requests.');
    $this->handler['curl']->access = array(PURGE_ACCESS_LIST,PURGE_ACCESS_ENABLE);
    $this->handler['curl']->item = array(
      'option' => array(
        'method' => 'PURGE',
        'parallel' => 1,
      ),
      'depend' => array(
        'php_curl' => 1,
      ),
    );

    // HTTPrl Handler.
    $this->handler['httprl'] = new PurgeHandlerHttprl();
    $this->handler['httprl']->name = t('HTTPrl');
    $this->handler['httprl']->description = t(
      'The HTTP Request Library module provides an advanced library that allows non-blocking requests.'
    );
    $this->handler['httprl']->enabled = 1;
    $this->handler['httprl']->access = array(PURGE_ACCESS_LIST,PURGE_ACCESS_ENABLE);
    $this->handler['httprl']->item = array(
      'option' => array(
        'method' => 'PURGE',
        'parallel' => 1,
        'non-blocking' => 1,
      ),
      'depend' => array(
        'module' => array(
          'httprl' => 1,
        ),
      ),
    );
  }
}


/**
 * Class definition for the Example Bundle.
 *   Provides a set of examples for various uses.
 */
class PurgePurgerBundleExample extends PurgePurgerBundle {

  /**
   * Constructor.
   */
  public function __construct() {
    // Targets
    // The localhost target
    $this->target['localhost'] = new PurgeTargetStatic();
    $this->target['localhost']->name = t('Localhost');
    $this->target['localhost']->description = t('The default localhost ip address.');
    $this->target['localhost']->access = array(PURGE_ACCESS_FULL);
    $this->target['localhost']->urls = array('http://127.0.0.1');

    // Domains
    // Get the current domain from the domain object.
    $item_domain_drupal = new PurgeDomainDrupal();
    $item_domain_drupal->cache();

    $this->domain['current_domain'] = new PurgeDomainStatic();
    $this->domain['current_domain']->name = t('Current Domain');
    $this->domain['current_domain']->description = t('Use the current domain as an example domain.');
    $this->domain['current_domain']->domains = $item_domain_drupal->domains[0];

    // Headers
    // Purgers

    // Now create the purger objects
    $this->purger['minimal_purger'] = new PurgePurgerUrl();
    $this->purger['minimal_purger']->name = t('Minimal Purger');
    $this->purger['minimal_purger']->description = t('Uses Drupal native methods to issue the requests.');
    $this->purger['minimal_purger']->item = array(
      'option' => array(
        'method' => 'PURGE',
      ),
      'depend' => array(),
      'target' => array('drupal_url'),
      'header' => array('gzip'),
      'queue' => array('fake'),
      'handler' => array('drupal_http_request'),
    );


    $this->purger['default_purger'] = new PurgePurgerUrl();
    $this->purger['default_purger']->name = t('Default Purger');
    $this->purger['default_purger']->description = t('Uses common settings used for a Varnish setup.');
    $this->purger['default_purger']->item = array(
      'option' => array(
        'method' => 'PURGE',
        'parallel' => 1,
      ),
      'depend' => array(),
      'target' => array('drupal_url', 'localhost'),
      'domain' => array('drupal_domain'),
      'header' => array('gzip'),
      'queue' => array('fake'),
      'handler' => array('curl'),
    );
  }
}
