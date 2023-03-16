<div class="list-group environment <?php print $environment->class  ?>" id="<?php print $environment->project_name; ?>-<?php print $environment->name ?>">

    <?php if (!empty($environment->menu)): ?>
    <!-- Environment Settings & Task Links -->
    <div class="environment-dropdowns">

      <!-- Information Modal -->
      <a type="button" class="environment-meta-data environment-info btn btn-text btn-sm" title="<?php print t('Environment Information') ?>" data-toggle="modal" data-target="#infoModal<?php print $environment->site ?>">
          <i class="fa fa-info-circle fa-2x"></i>
          <span class="sr-only">
            <?php print t('Environment Information') ?>
          </span>
      </a>

      <!-- Modal -->
      <div class="modal modal-info fade" id="infoModal<?php print $environment->site ?>" tabindex="-1" role="dialog" aria-labelledby="infoModalLabel<?php print $environment->site ?>">
        <div class="modal-dialog" role="document">
          <div class="modal-content">
            <div class="modal-header">
              <button type="button" class="close btn btn-lg" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
              <h4 class="modal-title" id="infoModalLabel<?php print $environment->site ?>"><?php print t('Environment Information') ?> <small><?php print $environment->name; ?></small>
              </h4>
              <a href="<?php print url($environment->url); ?>" target="_blank" class='btn btn-default btn-sm'><?php print t('Visit Site') ?> <i class="fa fa-external-link-square"></i> </a>
            </div>
            <div class="modal-body">
              <h4>SSH Access</h4>
              <p class="text-muted small">
                <?php

                if (module_exists('aegir_ssh') && user_access('manage own SSH public keys')) {
                  print t('Upload your public SSH keys at !link so you can access the site via SSH and Drush.', array(
                    '!link' => l('My Account > SSH Keys', "user/$user->uid/ssh-keys"),
                  ));
                }
                else {
                  print t('In order to access the server with SSH, you must add your public SSH key to the file <code>/var/aegir/.ssh/authorized_keys</code>.');
                }
                ?>
              </p>
              <label>
                Command
              </label>
              <input class="form-control inline" onclick="this.select()" value="ssh aegir@<?php print $environment->web_server; ?>">
            </div>
            <div class="modal-body">
              <h4>Drush Access</h4>
              <p class="text-muted small">
                <?php print t('Drush is installed on the server, with the alias <code>@!alias</code>. You may also access the sites by running Drush locally and !download for this project.', array(
                  '!alias' => $environment->system_domain,
                  '!download' => l(t('downloading the Drush aliases'), "node/{$project->nid}/aliases"),
                ));
                ?>
              </p>
              <section>
                <label>
                  Database CLI
                </label>
                <input class="form-control" onclick="this.select()" value="drush <?php print $environment->drush_alias; ?> sqlc">
              </section>
              <section>
                <label>
                  List All Commands
                </label>
                <input class="form-control" onclick="this.select()" value="drush <?php print $environment->drush_alias; ?> help">
              </section>
            </div>
            <div class="modal-body">
              <h4>Other Information</h4>
              <section>
                <label>
                  Database
                </label>
                <?php print $environment->db_name; ?>
              </section>
              <section>
                <label>
                  Repository Path
                </label>
                <?php print $environment->repo_path; ?>
              </section>
              <section>
                <label>
                  Publish Path
                </label>
                <?php print $environment->publish_path; ?>
              </section>
            </div>
            <div class="modal-body">
              <h4><?php print t('Deploy Hooks'); ?></h4>
              <!-- Show Hooks -->
              <div class="btn-group btn-hooks" role="group">
                <ul class="list-unstyled" role="menu">
                  <li class="text"><?php print t('Hooks are run any time new code is deployed.  The following hooks are enabled for this environment:'); ?></li>
                  <?php if (isset($environment->settings->deploy)): ?>
                    <?php foreach ($environment->settings->deploy as $hook_type => $enabled): ?>
                      <?php if ($enabled): ?>
                        <?php if ($hook_type == 'cache'): ?>
                          <li class="text code">
                            <code>drush clear-cache all</code>
                          </li>
                        <?php elseif ($hook_type == 'update'): ?>
                          <li class="text code">
                            <code>drush update-db -y</code>
                          </li>
                        <?php elseif ($hook_type == 'revert'): ?>
                          <li class="text code">
                            <code>drush features-revert-all -y</code>
                          </li>
                        <?php elseif ($hook_type == 'composer'): ?>
                          <li class="text code">
                            <code>composer install</code>
                          </li>
                        <?php elseif ($hook_type == 'test'): ?>
                          <li class="text code">
                            <?php print t('Run Tests'); ?>
                          </li>
                        <?php elseif ($hook_type == 'dothooks'): ?>
                          <li><?php print t('File-based Hooks'); ?></li>
                          <?php if (!empty($environment->dothooks_file_name)): ?>
                            <li class="text"><p class=\"text-info\">
                                <i class=\"fa fa-question-circle\"></i>
                                <?php print t("This is your &filename file:", array(
                                  '&filename' => $environment->dothooks_file_name
                                )); ?></p></li>
                            <li>
                              <pre><?php if (isset($environment->dothooks_file_path)) { print file_get_contents($environment->dothooks_file_path); } ?></pre>
                            </li>
                          <?php else:  ?>
                            <li class="text text-danger">
                              <i class="fa fa-warning"></i> <?php print $hooks_yml_note; ?>
                            </li>
                          <?php endif; ?>
                        <?php elseif ($hook_type == 'acquia_hooks'): ?>
                          <li><label><?php print t('Acquia Cloud Hooks'); ?></label></li>
                          <li class="text"><p class="text-info">
                              <i class="fa fa-question-circle"></i>
                              <?php print t("When code or data is deployed, the appropriate Acquia Cloud Hook within the project will be triggered."); ?></p></li>
                          <li class="text"><p class="text-muted"><?php print t('See !link1 and !link2 for more information ', array(
                                '!link1' => l('Acquia Cloud Documentation', 'https://docs.acquia.com/cloud/manage/cloud-hooks'),
                                '!link2' => l('https://github.com/acquia/cloud-hooks', 'https://github.com/acquia/cloud-hooks'),
                              )); ?></p>
                          </li>
                          <li>
                            <label>Supported Cloud Hooks</label>
                          </li>
                          <li class="text">

                            <ul>
                              <li><strong>post-code-deploy:</strong> <?php print t('Triggered after a <em>manually</em> started "Deploy Code" task ends.'); ?></li>
                              <li><strong>post-code-update:</strong> <?php print t('Triggered after an <em>automatic</em> "Deploy Code" task ends. (When developers "git push")'); ?></li>
                              <li><strong>post-db-copy:</strong> <?php print t('Triggered after a "Deploy Data" task runs if "Database" was selected.'); ?></li>
                              <li><strong>post-files-copy:</strong> <?php print t('Triggered after a "Deploy Data" task runs if "Database" was selected.'); ?></li>
                            </ul>

                          </li>
                        <?php endif; ?>
                      <?php endif; ?>
                    <?php endforeach; ?>
                  <?php endif; ?>
                </ul>
              </div>
            </div>
            <div class="modal-footer">
              <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
            </div>
          </div>
        </div>
      </div>
    <?php endif; ?>
    <?php if (!empty($environment->menu) && !isset($page)): ?>

      <div class="environment-menu btn-group ">
            <button type="button" class="btn btn-link environment-menu-button dropdown-toggle" data-toggle="dropdown" title="<?php print t('Environment Menu') ;?>">
                <i class="fa fa-bars"></i>
            </button>
            <?php print $environment->menu_rendered; ?>
        </div>
    <?php endif; ?>
    </div>

    <div class="environment-header list-group-item list-group-item-<?php print $environment->list_item_class ?>">

      <?php  if (isset($environment->image)): ?>
        <img src="<?php print $environment->image ?>" width="32" height="32" class="environment-avatar">
      <?php endif; ?>

      <!-- Environment Name -->
        <a href="<?php print $environment->site? url("node/$environment->site"): url("node/$environment->platform"); ?>" class="environment-link" title="<?php print t('Environment: ') . $environment->name; ?>">

          <?php if ($environment->name == $project->settings->primary_environment): ?>
            <i class="fa fa-bolt" title="<?php print t('Primary Environment'); ?>"></i>
          <?php endif; ?>

          <span><?php print $environment->name; ?></span>
        </a>

      <div class="environment-status">

        <a href="<?php print $environment->git_ref_url; ?>" class="environment-meta-data environment-git-ref btn btn-text" target="_blank" title="<?php print t('Git !type: ', array('!type' => $environment->git_ref_type)) . $environment->git_ref; ?>">
          <i class='fa fa-<?php print $environment->git_ref_type == 'tag'? 'tag': 'code-fork'; ?>'></i><?php print $environment->git_ref; ?>
        </a>

        <?php if ($environment->version): ?>
            <a href="<?php print url("node/$environment->platform"); ?>"  title="Drupal version <?php print $environment->version; ?>" class="environment-meta-data environment-drupal-version btn btn-text">
                <i class="fa fa-drupal"></i><?php print $environment->version; ?>
            </a>
        <?php endif; ?>

        <?php if ($environment->site_status == HOSTING_SITE_DISABLED): ?>
            <a class="environment-meta-data btn btn-text">Disabled</a>
        <?php endif; ?>

        <?php if ($environment->site_status == HOSTING_SITE_DELETED): ?>
            <a class="environment-meta-data btn btn-text">Deleted</a>
        <?php endif; ?>

        <?php if (isset($environment->settings->locked) && $environment->settings->locked): ?>
            <a class="environment-meta-data btn btn-text" title="<?php print t('This database is locked.'); ?>">
          <i class="fa fa-lock"></i><?php print t('Locked') ?>
        </a>
        <?php endif; ?>

        <?php if (drupal_valid_path("node/{$environment->site}/errors")): ?>
          <!-- Browse Logs -->
          <a href="<?php print url("node/$environment->site/errors"); ?>" class="environment-meta-data btn btn-text btn-sm " title="<?php print t('View error logs.'); ?>">
            <i class="fa fa-exclamation-circle"></i><?php print t('Errors'); ?>
          </a>
        <?php endif; ?>
    </div>
    </div>
  <div class='environment-main'>
    <div class='environment-messages'>

    <!-- Environment Warnings -->
    <?php if (!empty($warnings)): ?>
        <?php foreach ($warnings as $warning):

            if ($warning['type'] == 'warning') {
                $icon = 'exclamation-triangle';
                $class = 'warning';
            }
            elseif ($warning['type'] == 'error') {
                $icon = 'exclamation-circle';
                $class = 'danger';
            }
            elseif ($warning['type'] == 'info') {
                $icon = 'info-circle fa-';
                $class = 'info';
            }
            else {
              $class = 'default';
            }

            if (isset($warning['icon'])) {
              $icon = $warning['icon'];
            }
            ?>
        <div class="list-group-item list-group-item-<?php print $class ?> text">
          <div class="text">
            <?php if ($icon): ?><i class="fa fa-<?php print $icon ?>"></i><?php endif; ?>
            <?php print $warning['text'] ?>
          </div>
          <?php if (!empty($warning['buttons'])): ?>
          <div class="buttons">
            <?php print $warning['buttons'] ?>
          </div>
          <?php endif; ?>
        </div>
        <?php endforeach; ?>
    <?php endif; ?>
    </div>

  <!-- URLs -->
  <div class="environment-domains list-group-item <?php if (isset($environment->login_text)) print 'login-available'; ?>">

    <div class="btn-toolbar" role="toolbar">

      <?php
      // If we have more than one domain, add the dropdown.
      if (count($environment->domains) > 1):
        ?>
        <div class="btn-group btn-group-smaller btn-urls" role="group">
          <a href="<?php print $environment->url ?>" target="_blank" class="environment-link">
            <?php if (!empty($environment->ssl_enabled)): ?>
              <i class="fa fa-lock text-success"></i>
            <?php else: ?>
              <i class="fa fa-globe"></i>
            <?php endif; ?>
            <span class="hidden">Visit Environment:</span>
            <?php print $environment->url ?>
          </a>
        </div>
        <div class="btn-group btn-group-smaller" role="group">
          <button type="button" class="btn btn-link dropdown-toggle" data-toggle="dropdown">
            <i class="fa fa-globe"></i>
            <?php print count($environment->domains); ?>
            <span class="caret"></span>
          </button>
          <ul class="dropdown-menu" role="menu">
            <?php foreach ($environment->domains as $domain): ?>
              <li><a href="<?php print 'http://' . $domain; ?>" target="_blank"><?php print 'http://' . $domain; ?></a></li>
            <?php endforeach; ?>
            <li class="divider">&nbsp;</li>
            <li><?php print l(t('Edit Domains'), 'node/' . $environment->site . '/edit', array('fragment' => 'edit-aliases-wrapper')); ?></li>
          </ul>
        </div>

        <?php
      // If site only has one domain (no aliases):
      else: ?>

        <?php if (!empty($environment->url)): ?>
          <div class="btn-group btn-group-smaller btn-urls-single" role="group">
            <a href="<?php print $environment->url ?>" target="_blank">
              <?php if (!empty($environment->ssl_enabled)): ?>
                <i class="fa fa-lock" alt="<?php print t('Encrypted'); ?>"></i>
              <?php else: ?>
                <i class="fa fa-globe"></i>
              <?php endif;?>
              <?php print $environment->url ?>
            </a>
          </div>
        <?php else: ?>
          <button class="btn btn-xs">
            <i class="fa fa-globe"></i>
            <em>&nbsp;</em>
          </button>
        <?php endif;?>

      <?php endif;?>

      <!-- Log In Link -->
      <?php if (isset($environment->login_text)): ?>
        <div class="btn-group btn-group-smaller pull-right login-link" role="group">

          <!-- Button trigger modal -->
          <button type="button" class="btn btn-link" data-toggle="modal" data-target="#loginModal-<?php print $environment->name ?>" data-remote="<?php print url('devshop/login/reset/' . $environment->site); ?>">
            <i class="fa fa-sign-in"></i>
            <?php print $environment->login_text; ?>
          </button>

          <!-- Modal -->
          <div class="modal fade" id="loginModal-<?php print $environment->name ?>" tabindex="-1" role="dialog" aria-labelledby="loginModalLabel-<?php print $environment->name ?>">
            <div class="modal-dialog" role="document">
              <div class="modal-content">
                <div class="modal-header">
                  <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
                  <h4 class="modal-title" id="loginModalLabel-<?php print $environment->name ?>">
                    <?php print $environment->login_text; ?>
                  </h4>
                </div>
                <div class="modal-body">
                  <i class="fa fa-gear fa-spin"></i>
                  <?php print t('Requesting new log in link. Please wait...') ?>
                </div>
                <div class="modal-footer">
                  <button type="button" class="btn btn-default" data-dismiss="modal"><?php print t('Cancel'); ?></button>
                </div>
              </div>
            </div>
          </div>
        </div>
      <?php endif;?>
    </div>
  </div>


  <!-- Environment Info -->
    <div class="list-group-item environment-info">
        <label>
          <?php print t('Created'); ?>
        </label>
        <span class="content">
            <time class="timeago" datetime="<?php print date('c', $environment->created) ?>"><?php print format_date($environment->created); ?></time>
            <?php print $environment->install_method_label; ?>
        </span>
    </div>
    <?php
      // SITUATION: Environment is Active!
      if (empty($environment->tasks['delete'])): ?>

        <?php
        // Only show this area if they have at least one of these permissions.
        if (
                user_access('create deploy task') ||
                user_access('create sync task') ||
                user_access('create migrate task')
        ): ?>
            <div class="environment-deploy list-group-item">

                <!-- Deploy -->
                <label><?php print t('Deploy'); ?></label>
                <div class="btn-group btn-toolbar" role="toolbar">

                    <?php if (user_access('create deploy task')): ?>
                        <!-- Deploy: Code -->
                        <div class="btn-group btn-deploy-code" role="group">
                            <button type="button" class="btn btn-default dropdown-toggle btn-git-ref" data-toggle="dropdown"><i class="fa fa-code"></i>
                                <?php print t('Code'); ?>
                                <span class="caret"></span>
                            </button>
                            <ul class="dropdown-menu btn-git-ref" role="menu">
                              <?php if (user_access('create platform git-checkout task') || user_access('create platform git-tag task')): ?>
                                <li class="create-git-ref"><label><?php print t('Create'); ?>
                              <?php if (user_access('create platform git-checkout task')): ?>
                                  <a href="/hosting_confirm/<?php print $environment->platform ?>/platform_git-checkout?create=1" class="btn btn-sm"><i class="fa fa-code-fork"></i> <?php print t('Branch'); ?></a>
                              <?php endif; ?>
                              <?php if (user_access('create platform git-tag task')): ?>
                                <a href="/hosting_confirm/<?php print $environment->platform ?>/platform_git-tag" class="btn btn-sm"><i class="fa fa-tag"></i> <?php print t('Tag'); ?></a>
                              <?php endif; ?>
                                  </label></li>
                              <?php endif; ?>

                              <li><label><?php print t('Deploy branch or tag'); ?></label></li>
                                <?php if (count($git_refs)): ?>
                                    <?php foreach ($git_refs as $ref => $item): ?>
                                        <li>
                                            <?php print str_replace('ENV_NID', $environment->site, $item); ?>
                                        </li>
                                    <?php endforeach; ?>
                                <?php endif; ?>
                            </ul>
                        </div>
                    <?php endif; ?>

                    <?php if (user_access('create sync task')): ?>
                        <!-- Deploy: Data -->
                        <div class="btn-group btn-deploy-database" role="group">

                            <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown"><i class="fa fa-database"></i>
                                <?php print t('Data'); ?>
                                <span class="caret"></span>
                            </button>
                            <ul class="dropdown-menu" role="menu">
                                <?php if (isset($environment->settings->locked) && $environment->settings->locked): ?>
                                    <li><label><?php print t('This environment is locked. You cannot deploy data here.'); ?></label></li>
                                <?php elseif (count($source_environments) == 0): ?>
                                    <li><label><?php print t('No other environments available.'); ?></label></li>
                                <?php else: ?>
                                    <li><label><?php print t('Deploy data from'); ?></label></li>
                                    <?php foreach ($source_environments as $source): ?>
                                        <?php if ($source->name == $environment->name) continue; ?>
                                        <li><a href="/hosting_confirm/<?php print $environment->site ?>/site_sync/?source=<?php print $source->site ?>">
                                                <?php if ($project->settings->primary_environment == $source->name): ?>
                                                    <i class="fa fa-bolt deploy-db-indicator"></i>
                                                <?php elseif (isset($source->settings->locked) && $source->settings->locked): ?>
                                                    <i class="fa fa-lock deploy-db-indicator"></i>
                                                <?php endif; ?>

                                                <strong class="btn-block"><?php print $source->name ?></strong>
                                                <small><?php print $source->url; ?></small>
                                            </a>
                                        </li>
                                    <?php endforeach; ?>
                                    <?php if (isset($project->settings->deploy['allow_deploy_data_from_alias'])): ?>
                                        <li class="divider"></li>
                                        <li><a href="/hosting_confirm/<?php print $environment->site ?>/site_sync/?source=other&dest=<?php print $source->name ?>">
                                                <strong class="btn-block"><?php print t('Other...'); ?></strong>
                                                <small><?php print t('Enter a drush alias to deploy from.'); ?></small>
                                            </a>
                                        </li>
                                    <?php endif; ?>
                                <?php endif; ?>
                            </ul>
                        </div>
                    <?php endif; ?>


                    <?php if (user_access('create migrate task')): ?>
                        <!-- Deploy: Stack -->
                        <div class="btn-group btn-deploy-servers" role="group">

                            <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown"><i class="glyphicon glyphicon-tasks"></i>
                                <?php print t('Stack'); ?>
                                <span class="caret"></span>
                            </button>
                            <ul class="dropdown-menu devshop-stack" role="menu">
                                <li><label><?php print t('IP Address'); ?></label></li>
                                <?php if (empty($environment->ip_addresses)): ?>
                              <li class="text">
                                <?php print l(t('Add IP Address'), "node/{$environment->servers['http']['nid']}/edit"); ?>
                              </li>

                              <?php endif; ?>
                                <?php foreach ($environment->ip_addresses as $ip): ?>
                                    <li class="text">
                                        <?php print $ip ?>
                                    </li>
                                <?php endforeach; ?>

                                <li><label><?php print t('Servers'); ?></label></li>
                                <?php foreach ($environment->servers as $type => $server):
                                    // DB: Migrate Task
                                    if ($type == 'db') {
                                        $icon = 'database';
                                        $url = "hosting_confirm/{$environment->site}/site_migrate";
                                    }
                                    // HTTP: Edit Platform
                                    elseif ($type == 'http') {
                                        $icon = 'cube';
                                        $url = "node/{$environment->platform}/edit";
                                    }
                                    // SOLR: Edit Site
                                    elseif ($type == 'solr') {
                                        $icon = 'sun-o';
                                        $url = "node/{$environment->project_nid}/edit/{$environment->name}";
                                    }

                                    // Build http query.
                                    $query = array();
                                    $query['destination'] = $_GET['q'];
                                    $query['deploy'] = 'stack';

                                    $full_url = url($url, array('query' => $query));

                                    // @TODO: Not sure why nid is localhost here.
                                    $server_url = $server['nid'] == 'localhost'?
                                            'server_localhost':
                                            url('node/' . $server['nid']);
                                    ?>
                                    <li class="inline">
                                        <a href="<?php print $server_url; ?>" title="<?php print $type .': ' . $server['name']; ?>">
                                            <strong class="btn-block"><i class="fa fa-<?php print $icon; ?>"></i> <?php print $type; ?></strong>
                                            <small><?php print $server['name']; ?></small>
                                        </a>
                                        <?php if ($full_url) :?>
                                            <a href="<?php print $full_url;?>" title="<?php print t('Change !type server...', array('!type' => $type)); ?>"><i class="fa fa-angle-right"></i></a>
                                        <?php endif; ?>
                                    </li>
                                <?php endforeach; ?>
                            </ul>
                        </div>
                    <?php endif; ?>
                </div>
            </div>
        <?php endif; ?>
    <?php endif; ?>

  <?php if ($environment->git_sha): ?>

    <?php
    // Figure out status
    $item_class = 'default';
    $icon = 'check';
    $label = t('Clean');
    $note = '';

    if (strpos($environment->git_status, 'Untracked files:') !== FALSE) {
      $icon = 'exclamation-circle';
      $label = t('Untracked Files');
      $item_class = 'warning';

      // Detect Aegir files we should be ignoring.
      if (strpos($environment->git_status, 'sites/sites.php') !== FALSE || strpos($environment->git_status, 'sites/' . $environment->uri) !== FALSE || strpos($environment->git_status, 'sites/all/drush') !== FALSE) {

        $note = t('Aegir files were detected by git. It is recommended to add the following to your <code>.gitignore</code> file: ');

        $note .= '<pre>
# Aegir files
sites/sites.php
sites/*/files
sites/*/private
sites/*/drushrc.php
sites/*/settings.php
sites/*/local.settings.php
sites/all/drush/drushrc.php
</pre>';

      }
    }
    if (strpos($environment->git_status, 'Your branch is ahead') !== FALSE) {
      $icon = 'arrow-right';
      $label = t('Ahead');
      $item_class = 'info';
    }

    if (strpos($environment->git_status, 'modified:') !== FALSE || strpos($environment->git_status, 'deleted:') !== FALSE) {
      $icon = 'warning';
      $label = t('Modified Files');
      $item_class = 'danger';
    }

    if (strpos($environment->git_status, 'Changes to be committed:') !== FALSE) {
      $icon = 'check-square-o';
      $label = t('Staged to Commit');
      $item_class = 'success';
    }

    if (strpos($environment->git_status, 'Your branch is behind') !== FALSE) {
      $icon = 'arrow-left';
      $label = t('Behind');
      $item_class = 'info';
    }

    if (strpos($environment->git_status, 'deleted:') !== FALSE || strpos($environment->git_status, 'deleted:') !== FALSE) {
      $icon = 'warning';
      $label = t('Deleted Files');
      $item_class = 'danger';
    }

    if (strpos($environment->git_status, 'have diverged') !== FALSE) {
      $icon = 'exchange fa-rotate-90';
      $label = t('Diverged');
      $item_class = 'danger';
    }

    ?>
    <div class="list-group-item list-group-item-git">
      <label><?php print t('Git Status') ?></label>

      <!-- Git Status -->
      <div class="btn-group btn-git-status" role="group">
        <button type="button" class="btn btn-<?php print $item_class; ?>" data-toggle="modal" data-target="#environment-git-status-<?php print $environment->name; ?>">
          <i class="fa fa-<?php print $icon; ?>"></i>
          <?php print $label ?>
        </button>
        <button type="button" class="btn btn-text" data-toggle="modal" data-target="#environment-git-status-<?php print $environment->name; ?>" title="<?php print t('Last Commit'); ?>">

          <time class='timeago' datetime="<?php if (isset($environment->git_last)) print $environment->git_last ?>"><?php if (isset($environment->git_last_ago)) print $environment->git_last_ago ?>
        </button>
        <div class="modal fade" id="environment-git-status-<?php print $environment->name; ?>" tabindex="-1" role="dialog" aria-labelledby="git-status-modal" aria-hidden="true">
          <div class="modal-dialog modal-lg">
            <div class="modal-content">
              <div class="modal-header">
                <button type="button" class="close" data-dismiss="modal"><span aria-hidden="true">×</span><span class="sr-only">Close</span></button>
                <h4 class="modal-title" id="drush-alias-modal">
                  <?php print $environment->name ?> <?php print t('environment'); ?>
                  <small>Git Information</small>
                </h4>
              </div>
              <div class="modal-body">

                <div class="well">
                  <div class="pull-right">
                    <?php if ($project->git_provider == 'github'): ?>
                      <a href="https://github.com/<?php print $project->github_owner ?>/<?php print $project->github_repo ?>/commit/<?php print $environment->git_sha ?>" class="btn btn-link">
                        <i class="fa fa-github"></i>
                        <?php print t('View Commit on GitHub'); ?>
                      </a>
                    <?php endif; ?>
                    <?php if (drupal_valid_path("hosting_confirm/{$environment->platform}/platform_git-commit")): ?>
                    <a href="<?php print url("hosting_confirm/{$environment->platform}/platform_git-commit", array('query' => array('token' => $token))); ?>" class="btn btn-primary">
                      <i class="fa fa-code"></i> <?php print t('Commit'); ?>
                    </a>
                    <?php endif; ?>
                    <?php if (drupal_valid_path("hosting_confirm/{$environment->platform}/platform_git-reset")): ?>
                    <a href="<?php print url("hosting_confirm/{$environment->platform}/platform_git-reset", array('query' => array('token' => $token))); ?>" class="btn btn-danger">
                      <i class="fa fa-close"></i> <?php print t('Reset'); ?>
                    </a>
                    <?php endif; ?>
                  </div>
                  <?php print t('Git status of the codebase at <code>@path</code> as of @ago ago.', array(
                          '@path' => $environment->repo_path,
                          '@ago' => format_interval(time() - $environment->verified),
                  )); ?>
                </div>

                <?php print theme('hosting_ascii', array('output' => $environment->git_status)); ?>

                <p>
                  <?php print $note; ?>
                </p>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  <?php endif; ?>
  <?php if (!empty($git_origin)): ?>
    <div class="list-group-item environment-info">
      <label><?php print t('Forked Repo') ?></label>
      <span class="content">
        <?php print $git_origin ?>
      </span>
    </div>
  <?php endif;?>
  </div>

    <div class="environment-task-logs <?php if (!isset($page)) print 'list-group-item' ?>">
        <?php if (isset($page)): ?>
            <?php print $environment->task_logs; ?>
       <?php else: ?>
        <!-- Tasks -->
          <label class="sr-only"><?php print t('Last Task') ?></label>

            <div class="btn-group btn-logs pull-right" role="group">
                <button type="button" class="btn btn-default btn-sm dropdown-toggle" data-toggle="dropdown">
                    <i class="fa fa-list-alt"></i>
                </button>
                <div class="dropdown-menu environment-task-logs">
                    <?php print $environment->task_logs; ?>
                </div>
            </div>

            <div class="last-task-alert">
              <?php print theme('devshop_task', array('task' => $environment->last_task_node)); ?>
            </div>
        <?php endif; ?>
    </div>
</div>
