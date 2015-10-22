<?php

/**
 * @file
 * Contains \Drupal\purge\Plugin\PurgeQueue\FileQueue.
 */

namespace Drupal\purge\Plugin\PurgeQueue;

use Drupal\Core\DestructableInterface;
use Drupal\Core\StreamWrapper\PublicStream;
use Drupal\purge\Plugin\PurgeQueue\MemoryQueue;
use Drupal\purge\Plugin\Purge\Queue\PluginInterface;

/**
 * A \Drupal\purge\Plugin\Purge\Queue\PluginInterface compliant file-based queue.
 *
 * @PurgeQueue(
 *   id = "file",
 *   label = @Translation("File"),
 *   description = @Translation("A file-based queue for fast I/O systems."),
 * )
 */
class FileQueue extends MemoryQueue implements PluginInterface, DestructableInterface {

  /**
   * The file path to which the queue buffer gets written to.
   */
  protected $file = 'public://purge-queue-file';

  /**
   * The separator string to split columns with.
   */
  const SEPARATOR = '|';

  /**
   * Constructs a \Drupal\purge\Plugin\PurgeQueue\File object.
   *
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param mixed $plugin_definition
   *   The plugin implementation definition.
   */
  public function __construct(array $configuration, $plugin_id, $plugin_definition) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->file = str_replace('public:/', PublicStream::basePath(), $this->file);
    $this->bufferInitialize();
  }

  /**
   * {@inheritdoc}
   */
  private function bufferInitialize() {
    if (!$this->bufferInitialized) {
      $this->bufferInitialized = TRUE;
      $this->buffer = [];

      // Open and parse the queue file, if it wasn't there during initialization
      // it will automatically get written at some point.
      if (file_exists($this->file)) {
        foreach (file($this->file) as $line) {
          $line = explode(self::SEPARATOR, str_replace("\n", '', $line));
          $item_id = (int)array_shift($line);
          $line[self::EXPIRE] = (int)$line[self::EXPIRE];
          $line[self::CREATED] = (int)$line[self::CREATED];
          $this->buffer[$item_id] = $line;
        }
      }
    }
  }

  /**
   * Commit the buffer to disk.
   */
  public function bufferCommit() {
    $ob = '';
    $fh = fopen($this->file, 'w');
    foreach($this->buffer as $item_id => $line) {
      $ob .= $item_id . SELF::SEPARATOR . $line[SELF::DATA] . SELF::SEPARATOR
        . $line[SELF::EXPIRE] . SELF::SEPARATOR . $line[SELF::CREATED] . "\n";
    }
    fwrite($fh, $ob);
    fclose($fh);
  }

  /**
   * {@inheritdoc}
   */
  public function deleteQueue() {
    if (file_exists($this->file)) {
      unlink($this->file);
    }
    parent::deleteQueue();
  }

  /**
   * {@inheritdoc}
   *
   * @see \Drupal\purge\Plugin\Purge\Queue\Service::reload()
   */
  public function destruct() {
    if ($this->bufferInitialized) {
      $this->bufferCommit();
    }
  }

  /**
   * Trigger a disk commit when the object is destructed.
   */
  function __destruct() {
    if ($this->bufferInitialized) {
      $this->bufferCommit();
    }
  }

}
