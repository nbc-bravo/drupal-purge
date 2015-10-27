<?php

/**
 * @file
 * Contains \Drupal\purge\Plugin\Purge\DiagnosticCheck\ProcessorsAvailableDiagnosticCheck.
 */

namespace Drupal\purge\Plugin\Purge\DiagnosticCheck;

use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\purge\Plugin\Purge\DiagnosticCheck\DiagnosticCheckInterface;
use Drupal\purge\Plugin\Purge\DiagnosticCheck\DiagnosticCheckBase;
use Drupal\purge\Plugin\Purge\Processor\ProcessorsServiceInterface;

/**
 * Checks if there's a service enabled that puts purgers actively to work
 *
 * Site configurators are urged to think and decide how (queued) cache
 * invalidations are processed. This can be using cron, a AJAX enabled UI or
 * for instance in-request in high-performance (localhost) scenarios.
 *
 * @PurgeDiagnosticCheck(
 *   id = "processorsavailable",
 *   title = @Translation("Processors"),
 *   description = @Translation("Checks if something is processing the queue."),
 *   dependent_queue_plugins = {},
 *   dependent_purger_plugins = {}
 * )
 */
class ProcessorsAvailableDiagnosticCheck extends DiagnosticCheckBase implements DiagnosticCheckInterface {

  /**
   * @var \Drupal\purge\Plugin\Purge\Processor\ProcessorsServiceInterface
   */
  protected $purgeProcessors;

  /**
   * Constructs a ProcessorsAvailableCheck object.
   *
   * @param \Drupal\purge\Plugin\Purge\Processor\ProcessorsServiceInterface $purge_processors
   *   The purge processors registry service.
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param mixed $plugin_definition
   *   The plugin implementation definition.
   */
  public function __construct(ProcessorsServiceInterface $purge_processors, array $configuration, $plugin_id, $plugin_definition) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->purgeProcessors = $purge_processors;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $container->get('purge.processors'),
      $configuration,
      $plugin_id,
      $plugin_definition
    );
  }

  /**
   * {@inheritdoc}
   */
  public function run() {
    $processors = $this->purgeProcessors->getEnabled();
    if (empty($processors)) {
      $this->value = '';
      $this->recommendation = $this->t("There are no processors enabled, which means that your queue builds up without being processed.");
      return SELF::SEVERITY_WARNING;
    }
    elseif (count($processors) === 1) {
      $id = key($processors);
      $this->value = $processors[$id]->getTitle();
      $this->recommendation = $processors[$id]->getDescription();
      return SELF::SEVERITY_OK;
    }
    else {
      $this->value = [];
      foreach ($processors as $processor) {
        $this->value[] = $processor->getTitle();
      }
      $this->value = implode(', ', $this->value);
      $this->recommendation = $this->t("You have multiple processing services configured.");
      return SELF::SEVERITY_OK;
    }
  }

}