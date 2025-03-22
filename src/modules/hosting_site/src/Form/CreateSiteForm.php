<?php

declare(strict_types=1);

namespace Drupal\hosting_site\Form;

use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\hosting_site\Entity\SiteEntity\HostedDrupalSite;
use Drupal\hosting_site\Entity\SiteEntity\HostedSite;
use Drupal\site\Entity\SiteEntity;

/**
 * Provides a Hosting Site form.
 */
final class CreateSiteForm extends FormBase {

  /**
   * {@inheritdoc}
   */
  public function getFormId(): string {
    return 'hosting_site_create_site';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state): array {

    $form['code'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Name'),
      '#description' => $this->t('Enter a unique name for your website.'),
      '#field_prefix' => 'https://',
      '#field_suffix' => '.' . $this->getBaseTld(),
      '#required' => TRUE,
      '#size' => 16,
      '#maxlength' => 16,
    ];

    $form['actions'] = [
      '#type' => 'actions',
      'submit' => [
        '#type' => 'submit',
        '#value' => $this->t('Create Site'),
      ],
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function validateForm(array &$form, FormStateInterface $form_state): void {
    $full_url = $this->getFullUrl($form_state->getValue('code'));
    $form_state->setValue('full_url', $full_url);

    if (SiteEntity::loadBySiteUrl($this->getFullUrl($form_state->getValue('code')))) {
      $form_state->setErrorByName('code', $this->t('The site name %name is not available.', ['%name' => $form_state->getValue('name')]));
    }

    $site = HostedDrupalSite::create([
      'type' => 'hosted_drupal_site',
      'site_uri' => $full_url,
    ]);

    $site->validate();
    $site->save();
    $form_state->setValue('site_entity', $site);

  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state): void {
    $this->messenger()->addStatus($this->t('The site has been created.'));
    $form_state->setRedirect('<front>');
  }

  /**
   * Return configured base TLD.
   * @return string
   */
  private function getBaseTld() {
    return $this->config('hosting_site')->get('multisite_tld');
  }

  /**
   * Return a full URL for a project code.
   * @return string
   */
  private function getFullUrl($code) {
    return 'http://' . $code . '.' . $this->getBaseTld();
  }

}
