<?php
/********************************************************************
MULTIFEEDS TEST PAGE

Nothing too exciting here.  Just a sample page that demos integrated 
Multifeeds support as well as cached favicons and perhaps a few other 
things.

Lots of this code is commented to help explain some of the new stuff.  
Code was tested in PHP 5.2.2, but *should* also work with earlier 
versions of PHP, as supported by SimplePie (PHP 4.1).

********************************************************************/

// Include the SimplePie library, and the one that handles internationalized domain names.
require_once('../simplepie.inc');
require_once('../idn/idna_convert.class.php');

// Initialize some feeds for use.
$feed = new SimplePie();
$feed->set_feed_url(array(
	'http://rss.news.yahoo.com/rss/topstories',
	'http://news.google.com/?output=atom',
	'http://rss.cnn.com/rss/cnn_topstories.rss'
));

// When we set these, we need to make sure that the handler_image.php file is also trying to read from the same cache directory that we are.
$feed->set_favicon_handler('./handler_image.php');
$feed->set_image_handler('./handler_image.php');

// Initialize the feed.
$feed->init();

// Make sure the page is being served with the UTF-8 headers.
$feed->handle_content_type();

// Begin the (X)HTML page.
?><!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
        "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">	
<head>
	<title>Multifeeds Test page</title>
	<link rel="stylesheet" href="../demo/for_the_demo/simplepie.css" type="text/css" media="screen" title="SimplePie Styles" charset="utf-8" />
	<style type="text/css">
	div#site {
		width:600px;
	}
	span.footnote {
		white-space:nowrap;
	}
	h1 {
		line-height:1.4em;
	}
	h4 {
		padding-left:20px;
		background-color:transparent;
		background-repeat:no-repeat;
		background-position:0 1px;
	}
	.clearBoth {
		clear:both;
	}
	</style>
</head>
<body>
<div id="site">

	<?php if ($feed->error): ?>
		<p><?=$feed->error()?></p>
	<?php endif ?>

	<div class="chunk">
		<h1>Quick-n-Dirty Multifeeds Demo</a></h1>
	</div>

	<?php
	// Let's loop through each item in the feed.
	foreach($feed->get_items() as $item):

	// Let's give ourselves a reference to the parent $feed object for this particular item.
	$feed = $item->get_feed();
	?>

		<div class="chunk">
			<h4 style="background-image:url(<?php echo $feed->get_favicon(); ?>);"><a href="<?php echo $item->get_permalink(); ?>"><?php echo html_entity_decode($item->get_title(), ENT_QUOTES, 'UTF-8'); ?></a></h4>

			<!-- get_content() prefers full content over summaries -->
			<?php echo $item->get_content(); ?>

			<?php if ($enclosure = $item->get_enclosure()): ?>
				<div>
				<?php echo $enclosure->native_embed(array(
					// New 'mediaplayer' attribute shows off Flash-based MP3 and FLV playback.
					'mediaplayer' => '../demo/for_the_demo/mediaplayer.swf'
				)); ?>
				</div>
			<?php endif; ?>

			<p class="footnote">Source: <a href="<?php echo $feed->get_permalink(); ?>"><?php echo $feed->get_title(); ?></a> | <?php echo $item->get_date('j M Y | g:i a'); ?></p>
		</div>

	<?php endforeach ?>

	<p class="footnote">This is a test of the emergency broadcast system.  This is only a test&hellip; beeeeeeeeeeeeeeeeeeeeeeeeeep!</p>

</div>
</body>
</html>