<?php

/**
 * @file
 * Contains \Drupal\purge\Plugin\PurgePurgeable\Path.
 */

namespace Drupal\purge\Plugin\PurgePurgeable;

use Drupal\purge\Purgeable\PluginInterface as Purgeable;
use Drupal\purge\Purgeable\PluginBase;
use Drupal\purge\Purgeable\Exception\InvalidStringRepresentationException;

/**
 * Describes a path based cache wipe, e.g. "news/article-1".
 *
 * @PurgePurgeable(
 *   id = "path",
 *   label = @Translation("Path Purgeable")
 * )
 */
class Path extends PluginBase implements Purgeable {

  /**
   * {@inheritdoc}
   */
  public function __construct($representation) {
    parent::__construct($representation);
    if (empty($representation)) {
      throw new InvalidStringRepresentationException(
        'This does not look like a ordinary HTTP path element.');
    }
    if (strpos($representation, ' ') !== FALSE) {
      throw new InvalidStringRepresentationException(
        'A HTTP path element should not contain a space.');
    }
    if (strpos($representation, '*') !== FALSE) {
      throw new InvalidStringRepresentationException(
        'A HTTP path should not contain a *.');
    }
    if (strpos($representation, ':') !== FALSE) {
      throw new InvalidStringRepresentationException(
        'A HTTP path should not contain a : and look like a tag.');
    }
    if (preg_match('/[A-Za-z]/', $representation) === 0) {
      throw new InvalidStringRepresentationException(
        'A HTTP path should have alphabet characters in it.');
    }
  }
}
