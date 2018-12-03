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
    <div class="row">
        <div class="col-sm-12 col-md-12 col-lg-12 p">

            <div id="auth_box" class="login row">
                <h2 class="title"><?php print t('Sign In'); ?>
                </h2>

              <?php print $messages; ?>

              <?php print render($page['content']); ?>

                <br />
              <?php print l(t('Forgot your password?'), 'user/password', array(
                'attributes' => array(
                  'class' => array('btn btn-link'),
                )
              )); ?>
            </div>
        </div>
    </div>
</div>