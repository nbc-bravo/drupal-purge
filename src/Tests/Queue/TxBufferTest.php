<?php

/**
 * @file
 * Contains \Drupal\purge\Tests\Queue\TxBufferTest.
 */

namespace Drupal\purge\Tests\Queue;

use Drupal\purge\Tests\KernelTestBase;
use Drupal\purge\Plugin\Purge\Invalidation\PluginInterface as Invalidation;
use Drupal\purge\Plugin\Purge\Queue\TxBuffer;
use Drupal\purge\Plugin\Purge\Queue\TxBufferInterface;

/**
 * Tests \Drupal\purge\Tests\Queue\TxBufferTest.
 *
 * @todo
 *   This really, really needs to be a unit test but the effort failed the last
 *   time. Anyone willing to convert it entirely - much appreciated!
 *
 * @group purge
 */
class TxBufferTest extends KernelTestBase {

  /**
   * The tested TxBuffer object.
   *
   * @var \Drupal\purge\Plugin\Purge\Queue\TxBuffer
   */
  protected $buffer;

  /**
   * {@inheritdoc}
   */
  public function setUp() {
    parent::setUp();
    $this->buffer = new TxBuffer();
  }

  /**
   * Test that the state constants are available.
   */
  public function testStates() {
    $this->assertEqual(TxBufferInterface::CLAIMED, 0);
    $this->assertEqual(TxBufferInterface::ADDING, 1);
    $this->assertEqual(TxBufferInterface::ADDED, 2);
    $this->assertEqual(TxBufferInterface::RELEASING, 3);
    $this->assertEqual(TxBufferInterface::RELEASED, 4);
    $this->assertEqual(TxBufferInterface::DELETING, 5);
  }

  /**
   * Tests \Drupal\purge\Plugin\Purge\Queue\TxBuffer::count
   */
  public function testCount() {
    $this->assertEqual(0, count($this->buffer));
    $this->buffer->set($this->getInvalidations(5), TxBufferInterface::CLAIMED);
    $this->assertEqual(5, count($this->buffer));
    $this->buffer->set($this->getInvalidations(1), TxBufferInterface::CLAIMED);
    $this->assertEqual(6, count($this->buffer));
  }

  /**
   * Tests \Drupal\purge\Plugin\Purge\Queue\TxBuffer::current
   */
  public function testCurrent() {
    $objects = $this->getInvalidations(5);
    $this->assertFalse($this->buffer->current());
    $this->buffer->set($objects, TxBufferInterface::CLAIMED);
    $c = $this->buffer->current();
    $this->assertTrue($c instanceof Invalidation);
    $this->assertEqual($objects[0]->getId(), $c->getId());
  }

  /**
   * Tests \Drupal\purge\Plugin\Purge\Queue\TxBuffer::delete
   */
  public function testDelete() {
    $objects = $this->getInvalidations(5);
    $this->buffer->set($objects, TxBufferInterface::CLAIMED);

    // Test that deleting foreign objects, doesn't affect the buffer at all.
    $this->buffer->delete($this->getInvalidations(1));
    $this->assertEqual(5, count($this->buffer));
    $this->buffer->delete($this->getInvalidations(2));
    $this->assertEqual(5, count($this->buffer));

    // Now assert that deleting those we added earlier, does affect the buffer.
    $this->buffer->delete(array_pop($objects));
    $this->assertEqual(4, count($this->buffer));
    $this->buffer->delete($objects);
    $this->assertEqual(0, count($this->buffer));
  }

  /**
   * Tests \Drupal\purge\Plugin\Purge\Queue\TxBuffer::deleteEverything
   */
  public function testDeleteEverything() {
    $this->buffer->set($this->getInvalidations(5), TxBufferInterface::CLAIMED);
    $this->buffer->deleteEverything();
    $this->assertEqual(0, count($this->buffer));
  }

  /**
   * Tests \Drupal\purge\Plugin\Purge\Queue\TxBuffer::getByProperty
   */
  public function testGetByProperty() {
    $i = $this->getInvalidations(1);
    $this->buffer->set($i, TxBufferInterface::CLAIMED);
    $this->buffer->setProperty($i, 'find', 'me');
    $this->assertFalse($this->buffer->getByProperty('find', 'you'));
    $this->assertFalse($this->buffer->getByProperty('find', 0));
    $match = $this->buffer->getByProperty('find', 'me');
    $this->assertTrue($match instanceof Invalidation);
    $this->assertEqual($i->getId(), $match->getId());
  }

  /**
   * Tests \Drupal\purge\Plugin\Purge\Queue\TxBuffer::getFiltered
   */
  public function testGetFiltered() {
    $this->assertEqual(0, count($this->buffer->getFiltered(TxBufferInterface::CLAIMED)));
    $this->buffer->set($this->getInvalidations(5), TxBufferInterface::CLAIMED);
    $this->assertEqual(5, count($this->buffer->getFiltered(TxBufferInterface::CLAIMED)));
    $this->buffer->set($this->getInvalidations(3), TxBufferInterface::ADDING);
    $this->assertEqual(3, count($this->buffer->getFiltered(TxBufferInterface::ADDING)));
    $this->buffer->set($this->getInvalidations(7), TxBufferInterface::DELETING);
    $this->assertEqual(7, count($this->buffer->getFiltered(TxBufferInterface::DELETING)));
    $this->assertEqual(10, count($this->buffer->getFiltered(
      [TxBufferInterface::ADDING, TxBufferInterface::DELETING])));
  }

  /**
   * Tests \Drupal\purge\Plugin\Purge\Queue\TxBuffer::getState
   */
  public function testGetState() {
    $i = $this->getInvalidations(1);
    $this->assertNull($this->buffer->getState($i));
    $this->buffer->set($i, TxBufferInterface::CLAIMED);
    $this->assertEqual(TxBufferInterface::CLAIMED, $this->buffer->getState($i));
    $this->buffer->set($i, TxBufferInterface::DELETING);
    $this->assertEqual(TxBufferInterface::DELETING, $this->buffer->getState($i));
    $this->buffer->delete($i);
    $this->assertNull($this->buffer->getState($i));
  }

  /**
   * Tests:
   *   - \Drupal\purge\Plugin\Purge\Queue\TxBuffer::setProperty
   *   - \Drupal\purge\Plugin\Purge\Queue\TxBuffer::getProperty
   */
  public function testSetAndGetProperty() {
    $i = $this->getInvalidations(1);

    // Assert that setting/getting properties on unbuffered objects won't work.
    $this->assertNull($this->buffer->getProperty($i, 'prop'));
    $this->assertFalse($this->buffer->getProperty($i, 'prop', FALSE));
    $this->buffer->setProperty($i, 'prop', 'value');
    $this->assertNull($this->buffer->getProperty($i, 'prop'));

    // Assert that once buffered, it all does work.
    $this->buffer->set($i, TxBufferInterface::CLAIMED);
    $this->assertNull($this->buffer->getProperty($i, 'prop'));
    $this->assertFalse($this->buffer->getProperty($i, 'prop', FALSE));
    $this->buffer->setProperty($i, 'prop', 'value');
    $this->assertEqual('value', $this->buffer->getProperty($i, 'prop'));
    $this->buffer->setProperty($i, 'prop', 5.5);
    $this->assertEqual(5.5, $this->buffer->getProperty($i, 'prop'));
    $this->buffer->setProperty($i, 'prop', [1]);
    $this->assertTrue(is_array($this->buffer->getProperty($i, 'prop')));
    $this->assertTrue(current($this->buffer->getProperty($i, 'prop')) === 1);
    $this->buffer->delete($i);
    $this->assertNull($this->buffer->getProperty($i, 'prop'));
  }

  /**
   * Tests \Drupal\purge\Plugin\Purge\Queue\TxBuffer::has
   */
  public function testHas() {
    $i = $this->getInvalidations(1);
    $this->assertFalse($this->buffer->has($i));
    $this->buffer->set($i, TxBufferInterface::CLAIMED);
    $this->assertTrue($this->buffer->has($i));
    $this->buffer->delete($i);
    $this->assertFalse($this->buffer->has($i));
  }

  /**
   * Tests \Drupal\purge\Plugin\Purge\Queue\TxBuffer::key, \Drupal\purge\Plugin\Purge\Queue\TxBuffer::next
   */
  public function testKeyAndNext() {
    $objects = $this->getInvalidations(5);
    $this->assertNull($this->buffer->key());
    $this->buffer->set($objects, TxBufferInterface::CLAIMED);

    // Test that objects got added to the buffer in equal order as offered.
    foreach ($objects as $i) {
      $this->assertEqual($i->getId(), $this->buffer->key());
      $this->buffer->next();
    }

    // Test that iterating the buffer works as expected.
    foreach ($this->buffer as $id => $i) {
      $this->assertTrue($i instanceof Invalidation);
      $found = FALSE;
      foreach ($objects as $i) {
        if ($i->getId() === $id) {
          $found = TRUE;
          break;
        }
      }
      $this->assertTrue($found);
    }
  }

  /**
   * Tests \Drupal\purge\Plugin\Purge\Queue\TxBuffer::rewind
   */
  public function testRewind() {
    $objects = $this->getInvalidations(5);
    $this->assertNull($this->buffer->key());
    $this->assertFalse($this->buffer->rewind());
    $this->assertNull($this->buffer->key());
    $this->buffer->set($objects, TxBufferInterface::CLAIMED);
    $this->assertEqual($objects[0]->getId(), $this->buffer->key());
    foreach ($this->buffer as $id => $i) {
      // Just iterate, to advance the internal pointer.
    }
    $this->buffer->rewind();
    $this->assertEqual($objects[0]->getId(), $this->buffer->key());
  }

  /**
   * Tests \Drupal\purge\Plugin\Purge\Queue\TxBuffer::set
   */
  public function testSet() {
    $objects = $this->getInvalidations(4);

    // Assert that objects get set and become iterable.
    $this->buffer->set($objects, TxBufferInterface::DELETING);
    foreach ($objects as $i) {
      $found = FALSE;
      foreach ($this->buffer as $id => $i) {
        if ($id == $i->getId()) {
          $found = TRUE;
          break;
        }
      }
      $this->assertTrue($found);
    }

    // Assert that object states are correct.
    $this->assertEqual(4, count($this->buffer->getFiltered(TxBufferInterface::DELETING)));
    $this->buffer->set($objects[0], TxBufferInterface::ADDING);
    $this->assertEqual(3, count($this->buffer->getFiltered(TxBufferInterface::DELETING)));
    $this->assertEqual(1, count($this->buffer->getFiltered(TxBufferInterface::ADDING)));
  }

  /**
   * Tests \Drupal\purge\Plugin\Purge\Queue\TxBuffer::valid
   */
  public function testValid() {
    $this->assertFalse($this->buffer->valid());
    $this->buffer->set($this->getInvalidations(2), TxBufferInterface::CLAIMED);
    $this->assertTrue($this->buffer->valid());
    $this->buffer->next();
    $this->assertTrue($this->buffer->valid());
    $this->buffer->next();
    $this->assertFalse($this->buffer->valid());
  }

}
