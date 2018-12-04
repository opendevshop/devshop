<?php

/**
 * @file
 * User login page - Template file.
 */


$page['content']['system_main']['hybridauth']['#weight'] = -1;
$page['content']['system_main']['hybridauth']['#title'] = 'Sign In With';

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
    <div class="row">
        <div class="col-sm-12 col-md-12 col-lg-12 p">

            <div id="auth_box" class="login row">
                <h2 class="title"><?php print t('Reset Password'); ?><small class="pull-right"><?php print l(t('Sign In'), 'user/login', array(
                      'attributes' => array(
                        'class' => array('btn btn-default')
                      )
                    )); ?></small></h2>

              <?php print $messages; ?>

              <?php print render($page['content']); ?>

            </div>
        </div>
    </div>
</div>
<footer class="anonymous-page">
  <?php print render($page['footer']); ?>
</footer>