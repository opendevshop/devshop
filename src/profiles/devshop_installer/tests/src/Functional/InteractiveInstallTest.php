<?php

declare(strict_types=1);

namespace Drupal\Tests\drupal_cms_installer\Functional;

use Drupal\Core\Extension\ExtensionList;
use Drupal\Core\Extension\ModuleExtensionList;
use Drupal\Core\Extension\ModuleHandlerInterface;
use Drupal\Core\Extension\ThemeExtensionList;
use Drupal\Core\Extension\ThemeHandlerInterface;
use Drupal\FunctionalTests\Installer\InstallerTestBase;
use Drupal\user\Entity\User;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * @group drupal_cms_installer
 */
class InteractiveInstallTest extends InstallerTestBase {

  /**
   * {@inheritdoc}
   */
  protected $profile = 'drupal_cms_installer';

  /**
   * {@inheritdoc}
   */
  protected function setUpSettings(): void {
    $assert_session = $this->assertSession();
    $assert_session->buttonExists('Skip this step');

    // Choose all the add-ons!
    $page = $this->getSession()->getPage();
    $optional_recipes = $page->findAll('css', 'input[name^="add_ons["]');
    $this->assertNotEmpty($optional_recipes);
    array_walk($optional_recipes, fn ($checkbox) => $checkbox->check());
    $page->pressButton('Next');

    // Now we should be asked for the site name, with a default value in place
    // for the truly lazy.
    $assert_session->pageTextContains('Give your site a name');
    $site_name_field = $assert_session->fieldExists('Site name');
    $this->assertTrue($site_name_field->hasAttribute('required'));
    $this->assertNotEmpty($site_name_field->getValue());
    // We have to use submitForm() to ensure that batch operations, redirects,
    // and so forth in the remaining install tasks get done.
    $this->submitForm(['Site name' => 'Installer Test'], 'Next');

    // Proceed to the database settings form.
    parent::setUpSettings();
  }

  /**
   * {@inheritdoc}
   */
  protected function installDefaultThemeFromClassProperty(ContainerInterface $container): void {
    $this->assertNull($this->defaultTheme);
    // The Drupal CMS installer takes a specific step so that Stark will not
    // be installed, so assert that it is not, in fact, installed.
    $this->assertTrue($this->isInstalled);
    $this->assertArrayNotHasKey('stark', $container->get(ThemeHandlerInterface::class)->listInfo());
  }

  /**
   * {@inheritdoc}
   */
  protected function setUpProfile(): void {
    // Nothing to do here; Drupal CMS marks itself as a distribution so that the
    // installer will automatically select it.
  }

  /**
   * {@inheritdoc}
   */
  protected function visitInstaller(): void {
    parent::visitInstaller();
    // The task list should be hidden.
    $this->assertSession()->elementNotExists('css', '.task-list');
  }

  /**
   * {@inheritdoc}
   */
  protected function setUpLanguage() {
    // The Drupal CMS installer suppresses the language selection step, so
    // there's nothing to do here.
  }

  /**
   * {@inheritdoc}
   */
  protected function setUpSite(): void {
    $page = $this->getSession()->getPage();
    $page->fillField('Email', 'hello@good.bye');
    $page->fillField('Password', "kitty");
    $page->pressButton('Finish');

    $this->checkForMetaRefresh();
    $this->isInstalled = TRUE;
  }

  /**
   * Tests basic expectations of a successful Drupal CMS install.
   */
  public function testPostInstallState(): void {
    // The site name and site-wide email address should have been set.
    // @see \Drupal\RecipeKit\Installer\Form\SiteNameForm
    $site_config = $this->config('system.site');
    $this->assertSame('Installer Test', $site_config->get('name'));
    $this->assertSame("hello@good.bye", $site_config->get('mail'));

    // Update Status should be installed, and user 1 should be getting its
    // notifications.
    $this->assertTrue($this->container->get(ModuleHandlerInterface::class)->moduleExists('update'));
    $account = User::load(1);
    $this->assertContains($account->getEmail(), $this->config('update.settings')->get('notification.emails'));
    $this->assertContains('administrator', $account->getRoles());

    // The installer should have uninstalled itself.
    $this->assertFalse($this->container->getParameter('install_profile'));
    // The theme used in the installer, should not be installed.
    $this->assertArrayNotHasKey('drupal_cms_installer_theme', $this->config('core.extension')->get('theme'));

    // Ensure that there are non-core extensions installed, which proves that
    // recipes were applied during site installation.
    $this->assertContribInstalled($this->container->get(ModuleExtensionList::class));
    $this->assertContribInstalled($this->container->get(ThemeExtensionList::class));

    // Antibot prevents non-JS functional tests from logging in, so disable it.
    $this->config('antibot.settings')->set('form_ids', [])->save();
    // Log out so we can test that user 1's credentials were properly saved.
    $this->drupalLogout();

    // It should be possible to log in with the credentials we chose in the
    // installer.
    // @see ::setUpSite()
    $page = $this->getSession()->getPage();
    $page->fillField('name', "hello@good.bye");
    $page->fillField('pass', 'kitty');
    $page->pressButton('Log in');
    $assert_session = $this->assertSession();
    $assert_session->addressEquals('/admin/dashboard');
    $this->drupalLogout();

    // It should also be possible to log in with the username, which is
    // defaulted to `admin` by the installer.
    $page->fillField('name', 'admin');
    $page->fillField('pass', 'kitty');
    $page->pressButton('Log in');
    $assert_session->addressEquals('/admin/dashboard');
    $this->drupalLogout();
  }

  /**
   * Asserts that any number of contributed extensions are installed.
   *
   * @param \Drupal\Core\Extension\ExtensionList $list
   *   An extension list.
   */
  private function assertContribInstalled(ExtensionList $list): void {
    $core_dir = $this->container->getParameter('app.root') . '/core';

    foreach (array_keys($list->getAllInstalledInfo()) as $name) {
      // If the extension isn't part of core, great! We're done.
      if (!str_starts_with($list->getPath($name), $core_dir)) {
        return;
      }
    }
    $this->fail('No contributed extensions are installed.');
  }

}
