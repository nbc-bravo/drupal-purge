<?php

/**
 * @file
 * Contains \Drupal\purge\Queue\QueueManager.
 */

namespace Drupal\purge\Queue;

use Drupal\Core\Plugin\DefaultPluginManager;
use Drupal\Core\Cache\CacheBackendInterface;
use Drupal\Core\Extension\ModuleHandlerInterface;

/**
 * The queue plugin manager.
 */
class QueueManager extends DefaultPluginManager {

  /**
   * Constructs the QueueManager object.
   *
   * @param \Traversable $namespaces
   *   An object that implements \Traversable which contains the root paths
   *   keyed by the corresponding namespace to look for plugin implementations.
   * @param \Drupal\Core\Cache\CacheBackendInterface $cache_backend
   *   Cache backend instance to use.
   * @param \Drupal\Core\Extension\ModuleHandlerInterface $module_handler
   *   The module handler to invoke the alter hook with.
   */
  public function __construct(\Traversable $namespaces, CacheBackendInterface $cache_backend, ModuleHandlerInterface $module_handler) {
    parent::__construct(
      'Plugin/PurgeQueue',
      $namespaces,
      $module_handler,
      'Drupal\purge\Annotation\PurgeQueue');
    $this->setCacheBackend($cache_backend, 'purge_queue_plugins');
  }
}