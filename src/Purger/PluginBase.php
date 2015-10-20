<?php

/**
 * @file
 * Contains \Drupal\purge\Purger\PluginBase.
 */

namespace Drupal\purge\Purger;

use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\Core\Plugin\PluginBase as CorePluginBase;
use Drupal\purge\Purger\PluginInterface;

/**
 * Provides a base class for all purgers - the cache invalidation executors.
 */
abstract class PluginBase extends CorePluginBase implements PluginInterface {

  /**
   * Unique instance ID for this purger.
   *
   * @var string
   */
  protected $id;

  /**
   * Constructs a \Drupal\Component\Plugin\PluginBase derivative.
   *
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param mixed $plugin_definition
   *   The plugin implementation definition.
   *
   * @throws \LogicException
   *   Thrown if $configuration['id'] is missing, see Purger\Service::createId.
   */
  public function __construct(array $configuration, $plugin_id, $plugin_definition) {
    if (!is_string($configuration['id']) || empty($configuration['id'])) {
      throw new \LogicException('Purger cannot be constructed without ID.');
    }
    $this->id = $configuration['id'];
    unset($configuration['id']);
    parent::__construct($configuration, $plugin_id, $plugin_definition);
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration,
      $plugin_id,
      $plugin_definition
    );
  }

  /**
   * {@inheritdoc}
   */
  public function delete() {
    if ($this->pluginDefinition['multi_instance']) {
      throw new \LogicException('Plugin is multi-instantiable, ::delete() not implemented!');
    }
  }

  /**
   * {@inheritdoc}
   */
  public function getIdealConditionsLimit() {
    // We don't know how much invalidations our derivatives can process under
    // ideal circumstances, it can range from low numbers on inefficient CDNs to
    // literally thousands when connecting to efficient systems over a local
    // socket. Purger implementations are definitely encouraged to overload this
    // method with a value that is as accurately approached as possible.
    return 100;
  }

  /**
   * {@inheritdoc}
   */
  public function getId() {
    return $this->id;
  }

  /**
   * {@inheritdoc}
   */
  public function getLabel() {
    $label = $this->getPluginDefinition()['label'];
    if ($this->getPluginDefinition()['multi_instance']) {
      return $this->t('@label @id', ['@label' => $label, '@id' => $this->id]);
    }
    else {
      return $label;
    }
  }

  /**
   * {@inheritdoc}
   */
  public function getTypes() {
    return $this->getPluginDefinition()['types'];
  }

}
