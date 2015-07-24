<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title><?php print $head_title ?></title>

  <!-- HTML5 Shim and Respond.js IE8 support of HTML5 elements and media queries -->
  <!-- WARNING: Respond.js doesn't work if you view the page via file:// -->
  <!--[if lt IE 9]>
    <script src="https://oss.maxcdn.com/html5shiv/3.7.2/html5shiv.min.js"></script>
    <script src="https://oss.maxcdn.com/respond/1.4.2/respond.min.js"></script>
  <![endif]-->

  <?php print $styles ?>

</head>
<body class='<?php print $body_classes ?>'>

<div id="navbar-main" class="navbar navbar-inverse navbar-static-top" role="navigation">
  <div class="container-fluid">
    <div class="navbar-header">
      <a class="navbar-brand" href="/"><b class="text-primary"><?php print $site_name ?></b></a>
    </div>
    <ul class="nav navbar-nav navbar-left">
      <?php foreach ($primary_links as $link):?>
        <li class="<?php if ($_GET['q'] == $link['href']) print 'active'; ?>"><?php print l($link['title'], $link['href']); ?> </li>
      <?php endforeach; ?>
        <li>
          <?php print $tasks; ?>
        </li>
    </ul>
    <ul class="nav navbar-nav navbar-right">
      <?php foreach ($secondary_links as $link):?>
        <li class="<?php if ($_GET['q'] == $link['href']) print 'active'; ?>"><?php print l($link['title'], $link['href']); ?> </li>
      <?php endforeach; ?>
    </ul>
  </div>
</div>


<div class="container-fluid">
  <div class="row">
    <div class="main col-md-6">
      <?php if ($title): ?>
      <h1>
        <?php print $title ?>
        <?php if ($subtitle): ?>
          <small><?php print $subtitle ?></small>
        <?php endif; ?>
      </h1>
      <?php endif; ?>
    </div>
  </div>
  <div class="row">
    <div class="pre-content col-md-12">

      <?php print $messages; ?>

      <?php print $pre_content ?>

      <?php if ($title2): ?>
        <h3>
          <?php print $title2 ?>
          <?php if ($subtitle2): ?>
            <small><?php print $subtitle2 ?></small>
          <?php endif; ?>
        </h3>
      <?php endif; ?>
    </div>
  </div>
  <div class="row">
    <div class="main col-md-12 container-fluid">

      <?php if ($tabs) print $tabs ?>
      <?php if (isset($tabs2)) print $tabs2 ?>

      <?php if ($help): print $help; endif; ?>

      <?php print $content ?>
    </div>
  </div>
</div>



  <div id="footer">
    <div class='footer-message'><?php print $footer_message ?></div>
  </div>

<?php print $scripts ?>
<?php print $closure ?>
</body>
</html>