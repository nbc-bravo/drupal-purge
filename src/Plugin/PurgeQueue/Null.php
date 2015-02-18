<?php

/**
 * @file
 * Contains \Drupal\purge\Plugin\PurgeQueue\Null.
 */

namespace Drupal\purge\Plugin\PurgeQueue;

use Drupal\purge\Queue\PluginInterface;
use Drupal\purge\Plugin\PurgeQueue\Memory;

/**
 * API-compliant null queue back-end.
 *
 * This plugin is not intended for usage but gets loaded during module
 * installation, when configuration rendered invalid or when no other plugins
 * are available. Because its API compliant, Drupal won't crash visibly.
 *
 * @PurgeQueue(
 *   id = "null",
 *   label = @Translation("Null"),
 *   description = @Translation("API-compliant null queue back-end."),
 * )
 */
class Null extends Memory implements PluginInterface {}