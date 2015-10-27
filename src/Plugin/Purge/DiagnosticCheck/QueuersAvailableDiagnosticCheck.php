<?php

/**
 * @file
 * Contains \Drupal\purge\Plugin\Purge\DiagnosticCheck\QueuersAvailableDiagnosticCheck.
 */

namespace Drupal\purge\Plugin\Purge\DiagnosticCheck;

use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\purge\Plugin\Purge\DiagnosticCheck\DiagnosticCheckInterface;
use Drupal\purge\Plugin\Purge\DiagnosticCheck\DiagnosticCheckBase;
use Drupal\purge\Plugin\Purge\Queuer\QueuersServiceInterface;

/**
 * Checks if there's a service actively adding items to the queue.
 *
 * This test exists because it is possible to disable the cache tags queuer for
 * pure API use cases, but, doing so does 'break' functionality for users. So
 * by flagging this up, users are at least made aware.
 *
 * @PurgeDiagnosticCheck(
 *   id = "queuersavailable",
 *   title = @Translation("Queuers"),
 *   description = @Translation("Checks if there are active queuing services."),
 *   dependent_queue_plugins = {},
 *   dependent_purger_plugins = {}
 * )
 */
class QueuersAvailableDiagnosticCheck extends DiagnosticCheckBase implements DiagnosticCheckInterface {

  /**
   * @var \Drupal\purge\Plugin\Purge\Queuer\QueuersServiceInterface
   */
  protected $purgeQueuers;

  /**
   * Constructs a \Drupal\purge\Plugin\Purge\DiagnosticCheck\PurgerAvailableCheck object.
   *
   * @param \Drupal\purge\Plugin\Purge\Queuer\QueuersServiceInterface $purge_queuers
   *   The purge queuers registry service.
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param mixed $plugin_definition
   *   The plugin implementation definition.
   */
  public function __construct(QueuersServiceInterface $purge_queuers, array $configuration, $plugin_id, $plugin_definition) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->purgeQueuers = $purge_queuers;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $container->get('purge.queuers'),
      $configuration,
      $plugin_id,
      $plugin_definition
    );
  }

  /**
   * {@inheritdoc}
   */
  public function run() {
    $queuers = $this->purgeQueuers->getEnabled();
    if (empty($queuers)) {
      $this->value = '';
      $this->recommendation = $this->t("There are no queuing services enabled, this means that you can only invalidate external caches manually or programmatically.");
      return SELF::SEVERITY_WARNING;
    }
    elseif (count($queuers) === 1) {
      $id = key($queuers);
      $this->value = $queuers[$id]->getTitle();
      $this->recommendation = $queuers[$id]->getDescription();
      return SELF::SEVERITY_OK;
    }
    else {
      $this->value = [];
      foreach ($queuers as $queuer) {
        $this->value[] = $queuer->getTitle();
      }
      $this->value = implode(', ', $this->value);
      $this->recommendation = $this->t("You have multiple queueing services configured.");
      return SELF::SEVERITY_OK;
    }
  }

}