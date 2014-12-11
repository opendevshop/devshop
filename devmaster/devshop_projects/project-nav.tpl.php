
<nav class="navbar navbar-default navbar-project" role="navigation">
  <div class="container-fluid">
    <!-- First Links -->
    <div class="nav navbar-text main-project-nav">
      <ul class="nav nav-pills">

        <!-- Dashboard -->
        <li><?php print $dashboard_link; ?></li>

        <!-- Settings -->
        <?php if ($settings_link): ?>
        <li><?php print $settings_link; ?></li>
        <?php endif; ?>

        <!-- Logs-->
        <?php if ($logs_link): ?>
          <li><?php print $logs_link; ?></li>
        <?php endif; ?>
        <!-- Drush Info -->
        <li>
          <button type="button" class="btn btn-link pull-right" data-toggle="modal" data-target="#drush-alias-modal" title="Drush Aliases">
          <i class="fa fa-drupal"></i>
         </button>
        </li>
      </ul>

      <!-- Modal -->
      <div class="modal fade" id="drush-alias-modal" tabindex="-1" role="dialog" aria-labelledby="drush-alias-modal" aria-hidden="true">
        <div class="modal-dialog">
          <div class="modal-content">
            <div class="modal-header">
              <button type="button" class="close" data-dismiss="modal"><span aria-hidden="true">&times;</span><span class="sr-only">Close</span></button>
              <h4 class="modal-title" id="drush-alias-modal">Project Drush Aliases</h4>
            </div>
            <div class="modal-body">

              <!-- Download button -->
              <p>
                <a href="<?php print $aliases_url; ?>" class="btn btn-primary"><?php print t('Download Alias File'); ?></a> or copy to <code>~/.drush/<?php print $project->name; ?>.aliases.drushrc.php</code>.
              </p>

              <textarea cols="40" rows="10" class='form-control' onlick="this.select()"><?php print $drush_aliases; ?></textarea>

              <p>
                <?php print $access_note; ?>
              </p>

            </div>
            <div class="modal-footer">
              <button type="button" class="btn btn-default" data-dismiss="modal">Close</button>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- Git Info -->
    <div class="navbar-form navbar-right form-group">


      <div class="input-group">

        <!-- Link to github or an icon -->
        <?php if (isset($github_url)): ?>
          <a class="input-group-addon" href="<?php print $github_url; ?>" title="<?php print t('View on GitHub'); ?>" target="_blank"><i class="fa fa-github-alt"></i></a>
        <?php else: ?>
          <div class="input-group-addon"><i class="fa fa-git"></i></div>
        <?php endif; ?>


        <!-- Git URL -->
        <input type="text" class="form-control" size="26" value="<?php print $node->project->git_url; ?>" onclick="this.select()">

        <!-- Branch & Tag List -->
        <div class="input-group-btn">
          <button type="button" class="btn btn-default dropdown-toggle <?php print $branches_class ?>" data-toggle="dropdown" title="<?php print $branches_label; ?>">

            <?php if ($branches_show_label): ?>
              <i class="fa fa-<?php print $branches_icon; ?>"></i>
              <?php print $branches_label; ?>
            <?php else: ?>
              <small>
                <i class="fa fa-code-fork"></i> <?php print $branches_count; ?>
              </small>

              &nbsp;
              <?php if ($tags_count): ?>
                <small>
                  <i class="fa fa-tag"></i> <?php print $tags_count; ?>
                </small>
              <?php endif; ?>

            <?php endif; // branches_show label ?>

            <span class="caret"></span></button>
          <ul class="dropdown-menu dropdown-menu-right" role="menu">
            <?php foreach ($branches_items as $item): ?>
              <li><?php print $item; ?></li>
            <?php endforeach; ?>
          </ul>
        </div>
      </div>

    </div>
  </div>
</nav>
