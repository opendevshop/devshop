<script>
  (function ($) {
    $('#task-tabs a').click(function (e) {
      e.preventDefault()
      $(this).tab('show')
    })
  })(jQuery);
</script>

<?php

/**
 * @file node.tpl.php
 *
 * Theme implementation to display a node.
 *
 * Available variables:
 * - $title: the (sanitized) title of the node.
 * - $content: Node body or teaser depending on $teaser flag.
 * - $picture: The authors picture of the node output from
 *   theme_user_picture().
 * - $date: Formatted creation date (use $created to reformat with
 *   format_date()).
 * - $links: Themed links like "Read more", "Add new comment", etc. output
 *   from theme_links().
 * - $name: Themed username of node author output from theme_username().
 * - $node_url: Direct URL of the current node.
 * - $terms: the themed list of taxonomy term links output from theme_links().
 * - $submitted: themed submission information output from
 *   theme_node_submitted().
 *
 * Other variables:
 * - $node: Full node object. Contains data that may not be safe.
 * - $type: Node type, i.e. story, page, blog, etc.
 * - $comment_count: Number of comments attached to the node.
 * - $uid: User ID of the node author.
 * - $created: Time the node was published formatted in Unix timestamp.
 * - $zebra: Outputs either "even" or "odd". Useful for zebra striping in
 *   teaser listings.
 * - $id: Position of the node. Increments each time it's output.
 *
 * Node status variables:
 * - $teaser: Flag for the teaser state.
 * - $page: Flag for the full page state.
 * - $promote: Flag for front page promotion state.
 * - $sticky: Flags for sticky post setting.
 * - $status: Flag for published status.
 * - $comment: State of comment settings for the node.
 * - $readmore: Flags true if the teaser content of the node cannot hold the
 *   main body content.
 * - $is_front: Flags true when presented in the front page.
 * - $logged_in: Flags true when the current user is a logged-in member.
 * - $is_admin: Flags true when the current user is an administrator.
 *
 * @see template_preprocess()
 * @see template_preprocess_node()
 */
?>
<div id="node-<?php print $node->nid; ?>" class="node<?php if ($sticky) { print ' sticky'; } ?><?php if (!$status) { print ' node-unpublished'; } ?> clear-block">

  <?php print $picture ?>

  <div class="well well-sm">

    <h4>
      <span class="label label-<?php print $task_label_class ?>"><?php print $task_label ?></span>

      <a href="<?php print $node_url ?>" title="<?php print $title ?>"><?php print $title ?></a>

      <?php if ($retry): ?>
            <?php print $retry; ?>
      <?php endif; ?>
    </h4>

    <?php if ($submitted): ?>
      <p class="small">
        <?php print $submitted ?>
      </p>
    <?php endif; ?>

    <?php if ($site_url): ?>
      <?php print $site_url ?>
    <?php endif; ?>

    <?php if ($log_message): ?>
        <div class="alert alert-<?php print $log_class; ?>" role="alert">
            <?php print $log_message; ?>
        </div>
    <?php endif; ?>
  </div>

<?php  if ($node->test_results_formatted): ?>
  <div role="tabpanel">

    <!-- Nav tabs -->
    <ul class="nav nav-tabs" role="tablist" id="task-tabs">
      <li role="presentation" class="active"><a href="#task" aria-controls="task" role="tab" data-toggle="tab">
          <?php print t('Results'); ?>
        </a></li>
      <li role="presentation"><a href="#logs" aria-controls="logs" role="tab" data-toggle="tab">
          <?php print t('Details'); ?>
        </a></li>
    </ul>

    <!-- Tab panes -->
    <div class="tab-content">
      <div role="tabpanel" class="tab-pane active" id="task">
        <div class="padded-top">
          <div class="results-wrapper">
            <?php print $node->test_results_formatted; ?>
          </div>
          <label class="follow-checkbox btn btn-default"><input type="checkbox" id="follow"> Follow Logs</label>
        </div>
      </div>
      <div role="tabpanel" class="tab-pane" id="logs">
        <div class="padded-top">
          <?php print $content; ?>
        </div>
      </div>
    </div>
  </div>
<?php else: ?>
  <?php print $content; ?>

<?php endif; ?>

  <?php print $links; ?>
</div>