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
                <small><?php print t('Welcome to DevShop.Support') ?></small>
            </h1>
        </div>
    </div>
    <div class="row">
        <div class="col-sm-12 col-md-12 col-lg-12 ">

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