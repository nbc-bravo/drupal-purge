<?php

/**
 * @file
 * Contains \Drupal\purge\Plugin\PurgeInvalidation\RegularExpressionInvalidation.
 */

namespace Drupal\purge\Plugin\PurgeInvalidation;

use Drupal\purge\Plugin\Purge\Invalidation\PluginInterface;
use Drupal\purge\Plugin\Purge\Invalidation\PluginBase;
use Drupal\purge\Plugin\Purge\Invalidation\Exception\InvalidExpressionException;

/**
 * Describes invalidation by regular expression, e.g.: '\.(jpg|jpeg|css|js)$'.
 *
 * @PurgeInvalidation(
 *   id = "regex",
 *   label = @Translation("Regular expression"),
 *   description = @Translation("Invalidates by regular expression."),
 *   examples = {"\.(jpg|jpeg|css|js)$"},
 *   expression_required = TRUE,
 *   expression_can_be_empty = FALSE,
 *   expression_must_be_string = TRUE
 * )
 */
class RegularExpressionInvalidation extends PluginBase implements PluginInterface {}
