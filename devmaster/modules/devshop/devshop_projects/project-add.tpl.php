<?php if ($project->name): ?>
<div class="well" id="project-add-status">

    <h4><?php print $project->name; ?></h4>
    <p><?php print $project->git_url; ?></p>

    <?php if ($project->drupal_path): ?>
    <p><?php print t('Path to Drupal: '); ?><em><?php print $project->drupal_path; ?></em></p>
    <?php endif; ?>

    <?php if ($project->settings->live['live_domain']): ?>
    <p><?php print t('Live Domain: '); ?><em><?php print $project->settings->live['live_domain']; ?></em></p>
    <?php endif; ?>

    <?php if ($project->settings->default_environment['web_server']): ?>
        <p>Default Stack:</p>
        <ul>
            <li class="web-server-node"><a href="<?php print url('node/' . $web_server_node->nid); ?>"><i class="fa fa-cube"></i> <?php print $web_server_node->title ?></a></li>
            <li class="db-server-node"><a href="<?php print url('node/' . $db_server_node->nid); ?>"><i class="fa fa-database"></i> <?php print $db_server_node->title ?></a></li>
        </ul>
    <?php endif; ?>
</div>
<?php endif; ?>
