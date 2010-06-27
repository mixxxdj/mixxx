<?php

function microtime_float()
{
	if (version_compare(phpversion(), '5.0.0', '>='))
	{
		return microtime(true);
	}
	else
	{
		list($usec, $sec) = explode(' ', microtime());
		return ((float) $usec + (float) $sec);
	}
}

$start = microtime_float();

include('../simplepie.inc');

// Parse it
$feed = new SimplePie();
if (!empty($_GET['feed']))
{
	if (get_magic_quotes_gpc())
	{
		$_GET['feed'] = stripslashes($_GET['feed']);
	}
	$feed->set_feed_url($_GET['feed']);
	$feed->init();
}
$feed->handle_content_type();

?><!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title><?php echo (empty($_GET['feed'])) ? 'SimplePie' : 'SimplePie: ' . $feed->get_title(); ?></title>

<!-- META HTTP-EQUIV -->
<meta http-equiv="content-type" content="text/html; charset=<?php echo ($feed->get_encoding()) ? $feed->get_encoding() : 'UTF-8'; ?>" />
<meta http-equiv="imagetoolbar" content="false" />

<style type="text/css">
html, body {
	height:100%;
	margin:0;
	padding:0;
}

h1 {
	background-color:#333;
	color:#fff;
	font-size:3em;
	margin:0;
	padding:5px 15px;
	text-align:center;
}

div#footer {
	padding:5px 0;
}

div#footer,
div#footer a {
	text-align:center;
	font-size:0.7em;
}

div#footer a {
	text-decoration:underline;
}

code {
	background-color:#f3f3ff;
	color:#000;
}

pre {
	background-color:#f3f3ff;
	color:#000080;
	border:1px dotted #000080;
	padding:3px 5px;
}

form {
	margin:0;
	padding:0;
}

div.chunk {
	border-bottom:1px solid #ccc;
}

form#sp_form {
	text-align:center;
	margin:0;
	padding:0;
}

form#sp_form input.text {
	width:85%;
}
</style>

</head>

<body>
	<h1><?php echo (empty($_GET['feed'])) ? 'SimplePie' : 'SimplePie: ' . $feed->get_title(); ?></h1>

	<form action="" method="get" name="sp_form" id="sp_form">
		<p><input type="text" name="feed" value="<?php echo ($feed->subscribe_url()) ? htmlspecialchars($feed->subscribe_url()) : 'http://'; ?>" class="text" id="feed_input" />&nbsp;<input type="submit" value="Read" class="button" /></p>
	</form>

	<div id="sp_results">
		<?php if ($feed->data): ?>
			<?php $items = $feed->get_items(); ?>
			<p align="center"><span style="background-color:#ffc;">Displaying <?php echo $feed->get_item_quantity(); ?> most recent entries.</span></p>
			<?php foreach($items as $item): ?>
				<div class="chunk" style="padding:0 5px;">
					<h4><a href="<?php echo $item->get_permalink(); ?>"><?php echo $item->get_title(); ?></a> <?php echo $item->get_date('j M Y'); ?></h4>
					<?php echo $item->get_content(); ?>
					<?php
					if ($enclosure = $item->get_enclosure(0))
						echo '<p><a href="' . $enclosure->get_link() . '" class="download"><img src="./for_the_demo/mini_podcast.png" alt="Podcast" title="Download the Podcast" border="0" /></a></p>';
					?>
				</div>
			<?php endforeach; ?>
			</div>
		<?php endif; ?>
	</div>

	<div id="footer">
		Powered by <?php echo SIMPLEPIE_LINKBACK; ?>, a product of <a href="http://www.skyzyx.com">Skyzyx Technologies</a>.<br />
		Page created in <?php echo round(microtime_float()-$start, 3); ?> seconds.
	</div>
</body>
</html>
