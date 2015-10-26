<?php

/**
 * @file
 * Contains \Drupal\purge_ui\Controller\QueuerFormController.
 */

namespace Drupal\purge_ui\Controller;

use Symfony\Component\HttpKernel\Exception\NotFoundHttpException;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\Core\Controller\ControllerBase;
use Drupal\purge\Plugin\Purge\Queuer\QueuersServiceInterface;

/**
 * Controller for:
 *  - \Drupal\purge_ui\Form\QueuerDisableForm.
 *  - \Drupal\purge_ui\Form\QueuerEnableForm.
 */
class QueuerFormController extends ControllerBase {

  /**
   * @var \Drupal\purge\Plugin\Purge\Queuer\QueuersServiceInterface
   */
  protected $purgeQueuers;

  /**
   * Construct the QueuerFormController.
   *
   * @param \Drupal\purge\Plugin\Purge\Queuer\QueuersServiceInterface $purge_queuers
   *   The purge queuers registry service.
   */
  function __construct(QueuersServiceInterface $purge_queuers) {
    $this->purgeQueuers = $purge_queuers;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static($container->get('purge.queuers'));
  }

  /**
   * Render the queuer disable form.
   *
   * @param string $id
   *   The container id of the queuer to retrieve.
   *
   * @return array
   */
  public function disableForm($id) {
    if ($queuer = $this->purgeQueuers->get($id)) {
      if ($this->purgeQueuers->get($id)->isEnabled()) {
        return $this->formBuilder()->getForm("\Drupal\purge_ui\Form\QueuerDisableForm", $id);
      }
    }
    throw new NotFoundHttpException();
  }

  /**
   * Route title callback.
   *
   * @param string $id
   *   The container id of the queuer to retrieve.
   *
   * @return \Drupal\Core\StringTranslation\TranslationWrapper
   *   The page title.
   */
  public function disableFormTitle($id) {
    if ($queuer = $this->purgeQueuers->get($id)) {
      return $this->t('Disable @label', ['@label' => $queuer->getTitle()]);
    }
    return $this->t('Disable');
  }

  /**
   * Render the queuer enable form.
   *
   * @return array
   */
  public function enableForm() {
    if ($this->purgeQueuers->getDisabled()) {
      return $this->formBuilder()->getForm("Drupal\purge_ui\Form\QueuerEnableForm");
    }
    throw new NotFoundHttpException();
  }

}
