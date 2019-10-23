<?php

/**
 * @file
 * User login page - Template file.
 */

?>
<div class="container">
    <div class="row">
        <div class="col-sm-12 col-md-12 col-lg-12 page-header login-header">
            <h1><?php print t('Hi!'); ?>
                <small><?php print t('Welcome to @hostname', array(
                        '@hostname' => $_SERVER['HTTP_HOST']
                  )) ?></small>
            </h1>
        </div>
    </div>
    <div class="row logos">
        <div class="col-xs-6 col-sm-6 col-md-6 col-lg-6">
            <img src="/<?php print path_to_theme(); ?>/devshop.support.png" class="img-responsive login-images">
        </div>
        <div class="col-xs-6 col-sm-6 col-md-6 col-lg-6">
            <a href="<?php print url('<front>'); ?>">
                <img src="<?php print $logo; ?>" alt="<?php print $site_name; ?>" class="img-responsive login-images">
            </a>
        </div>
    </div>
    <div class="row">
        <div class="col-sm-12 col-md-12 col-lg-12">

            <div id="auth_box" class="login row">
                <h2 class="title"><?php print t('Sign In'); ?>
                </h2>

              <?php print $messages; ?>

              <?php print render($page['content']); ?>

            </div>
        </div>
    </div>
</div>
<footer class="anonymous-page">
  <?php print render($page['footer']); ?>
</footer>