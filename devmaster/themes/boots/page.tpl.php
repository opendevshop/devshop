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

<div class="navbar navbar-inverse navbar-static-top" role="navigation">
  <div class="container-fluid">
    <div class="navbar-header">
      <button type="button" class="navbar-toggle" data-toggle="collapse" data-target=".navbar-collapse">
        <span class="sr-only">Toggle navigation</span>
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
      </button>
      <a class="navbar-brand" href="/"><b class="text-primary"><?php print $site_name ?></b></a>
    </div>
    <div class="navbar-collapse collapse">
      <ul class="nav navbar-nav navbar-left">
        <?php foreach ($primary_links as $link): ?>
          <li><?php print l($link['title'], $link['href']); ?> </li>
        <?php endforeach; ?>
      </ul>
    </div>
  </div>
</div>
<div class="breadcrumb">
  <?php if ($breadcrumb) print $breadcrumb; ?>
</div>


<div class="container-fluid">
  <div class="row">
    <div class="main col-md-12">


<?php if ($help): print $help; endif; ?>
<?php print $content ?>
    </div>
  </div>
</div>



<div id="footer" class='reverse'><div class='limiter clear-block'>
    <?php print $footer; ?>
    <?php if ($secondary_links) print theme('links', $secondary_links, array('class' => 'links secondary-links')) ?>
    <div class='footer-message'><?php print $footer_message ?></div>
  </div></div>


<!-- jQuery (necessary for Bootstrap's JavaScript plugins) -->
<script src="https://ajax.googleapis.com/ajax/libs/jquery/1.11.1/jquery.min.js"></script>
<!-- Include all compiled plugins (below), or include individual files as needed -->
<script src="js/bootstrap.min.js"></script>
<?php print $scripts ?>
<?php print $closure ?>
</body>
</html>