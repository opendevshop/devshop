<?php

namespace Drupal\site_hosting\Plugin\Command;

use Drupal\commands\CommandPluginBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\user\Entity\User;

/**
 * Clones and checks out code.
 *
 * @Command(
 *   id = "git_checkout",
 *   label = @Translation("Git Checkout"),
 *   description = @Translation("Clone a git repository and checkout the desired git reference."),
 *   command = {
 *     "git clone [git_remote] [git_root] || echo 'Clone already exists.'",
 *     "cd [git_root] && git fetch && git checkout [git_reference]",
 *   }
 * )
 */
class GitCheckout extends CommandPluginBase {

//  public function command($params = []) {
//    $command = <<<CMD
//git clone $git_remote $git_root
//cd $git_root
//git fetch
//git checkout $git_reference
//CMD;
//    return $command;
//
//  }

  /**
   * Show a form on the "Run command" form.
   * @return void
   */
  public function form($form, FormStateInterface $form_state) {
    $form['parameters']['git_remote'] = [
      '#type' => 'textfield',
      '#title' => t('Git Remote'),
      '#description'=> t('The remote URL of the repository to clone.'),
    ];
    $form['parameters']['git_root'] = [
      '#type' => 'textfield',
      '#title' => t('Git Root'),
      '#description'=> t('The absolute path to the root of the repository.'),
    ];
    $form['parameters']['git_reference'] = [
      '#type' => 'textfield',
      '#title' => t('Git Reference'),
      '#description'=> t('The git reference to checkout.'),
    ];

    // @TODO: Reset working_directory to detected composer root.
    return $form;
  }
}
