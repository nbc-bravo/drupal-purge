<?php

/**
 * @file
 * Contains \Drupal\purge\Plugin\PurgeInvalidation\EverythingInvalidation.
 */

namespace Drupal\purge\Plugin\PurgeInvalidation;

use Drupal\purge\Invalidation\PluginInterface;
use Drupal\purge\Invalidation\PluginBase;

/**
 * Describes that everything is to be invalidated.
 *
 * @PurgeInvalidation(
 *   id = "everything",
 *   label = @Translation("Everything"),
 *   description = @Translation("Invalidates everything."),
 *   expression_required = FALSE
 * )
 */
class EverythingInvalidation extends PluginBase implements PluginInterface {}