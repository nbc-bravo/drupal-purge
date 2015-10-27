<?php

 /**
  * @file
  * Contains \Drupal\purge_queuer_test\Plugin\Purge\Queuer\AQueuer.
  */

namespace Drupal\purge_queuer_test\Plugin\Purge\Queuer;

use Drupal\purge\Plugin\Purge\Queuer\QueuerInterface;

 /**
  * Test queuer A.
  *
  * @PurgeQueuer(
  *   id = "a",
  *   label = @Translation("Queuer A"),
  *   description = @Translation("Test queuer A."),
  *   enable_by_default = true,
  *   configform = "",
  * )
  */
 class AQueuer implements QueuerInterface {

 }