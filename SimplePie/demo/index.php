<?php
// Start counting time for the page load
$starttime = explode(' ', microtime());
$starttime = $starttime[1] + $starttime[0];

// Include SimplePie
// Located in the parent directory
include_once('../simplepie.inc');
include_once('../idn/idna_convert.class.php');

// Create a new instance of the SimplePie object
$feed = new SimplePie();

//$feed->force_fsockopen(true);

// Make sure that page is getting passed a URL
if (isset($_GET['feed']) && $_GET['feed'] !== '')
{
	// Strip slashes if magic quotes is enabled (which automatically escapes certain characters)
	if (function_exists('get_magic_quotes_gpc') && get_magic_quotes_gpc())
	{
		$_GET['feed'] = stripslashes($_GET['feed']);
	}
	
	// Use the URL that was passed to the page in SimplePie
	$feed->set_feed_url($_GET['feed']);
	
	// XML dump
	$feed->enable_xml_dump(isset($_GET['xmldump']) ? true : false);
}

// Allow us to change the input encoding from the URL string if we want to. (optional)
if (!empty($_GET['input']))
{
	$feed->set_input_encoding($_GET['input']);
}

// Allow us to choose to not re-order the items by date. (optional)
if (!empty($_GET['orderbydate']) && $_GET['orderbydate'] == 'false')
{
	$feed->enable_order_by_date(false);
}

// Allow us to cache images in feeds.  This will also bypass any hotlink blocking put in place by the website.
if (!empty($_GET['image']) && $_GET['image'] == 'true')
{
	$feed->set_image_handler('./handler_image.php');
}

// We'll enable the discovering and caching of favicons.
$feed->set_favicon_handler('./handler_image.php');

// Initialize the whole SimplePie object.  Read the feed, process it, parse it, cache it, and 
// all that other good stuff.  The feed's information will not be available to SimplePie before 
// this is called.
$success = $feed->init();

// We'll make sure that the right content type and character encoding gets set automatically.
// This function will grab the proper character encoding, as well as set the content type to text/html.
$feed->handle_content_type();

// When we end our PHP block, we want to make sure our DOCTYPE is on the top line to make 
// sure that the browser snaps into Standards Mode.
?><!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en-US" lang="en-US">
<head>
<title>SimplePie: Demo</title>

<link rel="stylesheet" href="./for_the_demo/sIFR-screen.css" type="text/css" media="screen">
<link rel="stylesheet" href="./for_the_demo/sIFR-print.css" type="text/css" media="print">
<link rel="stylesheet" href="./for_the_demo/simplepie.css" type="text/css" media="screen, projector" />

<script type="text/javascript" src="./for_the_demo/sifr.js"></script>
<script type="text/javascript" src="./for_the_demo/sifr-config.js"></script>
<script type="text/javascript" src="./for_the_demo/sleight.js"></script>

</head>

<body id="bodydemo">

<div id="header">
	<div id="headerInner">
		<div id="logoContainer">
			<div id="logoContainerInner">
				<div align="center"><a href="http://simplepie.org"><img src="./for_the_demo/logo_simplepie_demo.png" alt="SimplePie Demo: PHP-based RSS and Atom feed handling" title="SimplePie Demo: PHP-based RSS and Atom feed handling" border="0" /></a></div>
				<div class="clearLeft"></div>
			</div>

		</div>
		<div id="menu">
		<!-- I know, I know, I know... tables for layout, I know.  If a web standards evangelist (like me) has to resort 
		to using tables for something, it's because no other possible solution could be found.  This issue?  No way to 
		do centered floats purely with CSS. The table box model allows for a dynamic width while centered, while the 
		CSS box model for DIVs doesn't allow for it. :( -->
		<table cellpadding="0" cellspacing="0" border="0"><tbody><tr><td>
<ul><li id="demo"><a href="./">SimplePie Demo</a></li><li><a href="http://simplepie.org/wiki/faq/start">FAQ/Troubleshooting</a></li><li><a href="http://simplepie.org/support/">Support Forums</a></li><li><a href="http://simplepie.org/wiki/reference/start">API Reference</a></li><li><a href="http://simplepie.org/blog/">Weblog</a></li><li><a href="../test/test.php">Unit Tests</a></li></ul>

			<div class="clearLeft"></div>
		</td></tr></tbody></table>
		</div>
	</div>
</div>

<div id="site">

	<div id="content">

		<div class="chunk">
			<form action="" method="get" name="sp_form" id="sp_form">
				<div id="sp_input">


					<!-- If a feed has already been passed through the form, then make sure that the URL remains in the form field. -->
					<p><input type="text" name="feed" value="<?php if ($feed->subscribe_url()) echo $feed->subscribe_url(); ?>" class="text" id="feed_input" />&nbsp;<input type="submit" value="Read" class="button" /></p>


				</div>
			</form>


			<?php
			// Check to see if there are more than zero errors (i.e. if there are any errors at all)
			if ($feed->error())
			{
				// If so, start a <div> element with a classname so we can style it.
				echo '<div class="sp_errors">' . "\r\n";

					// ... and display it.
					echo '<p>' . htmlspecialchars($feed->error()) . "</p>\r\n";

				// Close the <div> element we opened.
				echo '</div>' . "\r\n";
			}
			?>

			<!-- Here are some sample feeds. -->
			<p class="sample_feeds"><strong>Or try one of the following:</strong> 
			<a href="?feed=http://www.詹姆斯.com/atomtests/iri/everything.atom" title="Test: International Domain Name support">詹姆斯.com</a>, 
			<a href="?feed=http://www.adultswim.com/williams/podcast/tools/xml/video_rss.xml" title="Humor from the people who make [adult swim] cartoons.">adult swim</a>, 
			<a href="?feed=http://afterdawn.com/news/afterdawn_rss.xml" title="Ripping, Burning, DRM, and the Dark Side of Consumer Electronics Media">Afterdawn</a>, 
			<a href="?feed=http://feeds.feedburner.com/ajaxian" title="AJAX and Scripting News">Ajaxian</a>, 
			<a href="?feed=http://www.andybudd.com/index.rdf&amp;image=true" title="Test: Bypass Image Hotlink Blocking">Andy Budd</a>, 
			<a href="?feed=http://feeds.feedburner.com/AskANinja" title="Test: Embedded Enclosures">Ask a Ninja</a>, 
			<a href="?feed=http://www.atomenabled.org/atom.xml" title="Test: Atom 1.0 Support">AtomEnabled.org</a>, 
			<a href="?feed=http://newsrss.bbc.co.uk/rss/newsonline_world_edition/front_page/rss.xml" title="World News">BBC News</a>, 
			<a href="?feed=http://newsrss.bbc.co.uk/rss/arabic/news/rss.xml" title="Test: Windows-1256 Encoding">BBC Arabic</a>, 
			<a href="?feed=http://newsrss.bbc.co.uk/rss/chinese/simp/news/rss.xml" title="Test: GB2312 Encoding">BBC China</a>, 
			<a href="?feed=http://newsrss.bbc.co.uk/rss/russian/news/rss.xml" title="Test: Windows-1251 Encoding">BBC Russia</a>, 
			<a href="?feed=http://inessential.com/xml/rss.xml" title="Developer of NetNewsWire">Brent Simmons</a>, 
			<a href="?feed=http://www.channelfrederator.com/rss" title="Test: Embedded Enclosures">Channel Frederator</a>, 
			<a href="?feed=http://rss.cnn.com/rss/cnn_topstories.rss" title="World News">CNN</a>, 
			<a href="?feed=http://digg.com/rss/index.xml" title="Tech news. Better than Slashdot.">Digg</a>, 
			<a href="?feed=http://revision3.com/diggnation/feed/quicktime-large" title="Tech and industry videocast.">Diggnation</a>, 
			<a href="?feed=http://www.flickr.com/services/feeds/photos_public.gne?format=rss2" title="Flickr Photos">Flickr</a>, 
			<a href="?feed=http://news.google.com/?output=rss" title="World News">Google News</a>, 
			<a href="?feed=http://video.google.com/videofeed?type=top100new&num=20&output=rss" title="Test: Media RSS Support">Google Video</a>, 
			<a href="?feed=http://blogs.law.harvard.edu/home/feed/rdf/" title="Test: Tag Stripping">Harvard Law</a>, 
			<a href="?feed=http://hagada.org.il/hagada/html/backend.php" title="Test: Window-1255 Encoding">Hebrew Language</a>, 
			<a href="?feed=http://www.infoworld.com/rss/news.xml" title="Test: Ad Stripping">InfoWorld</a>, 
			<a href="?feed=http://phobos.apple.com/WebObjects/MZStore.woa/wpa/MRSS/topsongs/limit=10/rss.xml&orderbydate=false" title="Test: Tag Stripping">iTunes</a>, 
			<a href="?feed=http://blog.japan.cnet.com/lessig/index.rdf" title="Test: EUC-JP Encoding">Japanese Language</a>, 
			<a href="?feed=http://nurapt.kaist.ac.kr/~jamaica/htmls/blog/rss.php&amp;input=EUC-KR" title="Test: EUC-KR Encoding">Korean Language</a>, 
			<a href="?feed=http://mir.aculo.us/xml/rss/feed.xml" title="Weblog for the developer of Scriptaculous">mir.aculo.us</a>, 
			<a href="?feed=http://images.apple.com/trailers/rss/newtrailers.rss" title="Apple's QuickTime movie trailer site">Movie Trailers</a>, 
			<a href="?feed=http://www.newspond.com/rss/main.xml" title="Tech and Science News">Newspond</a>, 
			<a href="?feed=http://nick.typepad.com/blog/index.rss" title="Developer of TopStyle and FeedDemon">Nick Bradbury</a>, 
			<a href="?feed=http://feeds.feedburner.com/ok-cancel" title="Usability comics and commentary">OK/Cancel</a>, 
			<a href="?feed=http://osnews.com/files/recent.rdf" title="News about every OS ever">OS News</a>, 
			<a href="?feed=http://weblog.philringnalda.com/feed/" title="Test: Atom 1.0 Support">Phil Ringnalda</a>, 
			<a href="?feed=http://kabili.libsyn.com/rss" title="Test: Improved enclosure type sniffing">Photoshop Videocast</a>, 
			<a href="?feed=http://www.pariurisportive.com/blog/xmlsrv/rss2.php?blog=2" title="Test: ISO-8859-1 Encoding">Romanian Language</a>, 
			<a href="?feed=http://www.erased.info/rss2.php" title="Test: KOI8-R Encoding">Russian Language</a>, 
			<a href="?feed=http://www.upsaid.com/isis/index.rdf" title="Test: BIG5 Encoding">Traditional Chinese Language</a>, 
			<a href="?feed=http://technorati.com/watchlists/rss.html?wid=29290" title="Technorati watch for SimplePie">Technorati</a>, 
			<a href="?feed=http://www.tbray.org/ongoing/ongoing.atom" title="Test: Atom 1.0 Support">Tim Bray</a>, 
			<a href="?feed=http://tuaw.com/rss.xml" title="Apple News">TUAW</a>, 
			<a href="?feed=http://www.tvgasm.com/atom.xml&amp;image=true" title="Test: Bypass Image Hotlink Blocking">TVgasm</a>, 
			<a href="?feed=http://uneasysilence.com/feed/" title="Interesting tech randomness">UNEASYsilence</a>, 
			<a href="?feed=http://feeds.feedburner.com/web20Show" title="Test: Embedded Enclosures">Web 2.0 Show</a>, 
			<a href="?feed=http://windowsvistablog.com/blogs/MainFeed.aspx" title="Test: Tag Stripping">Windows Vista Blog</a>, 
			<a href="?feed=http://xkcd.com/rss.xml" title="Test: LightHTTPd and GZipping">XKCD</a>, 
			<a href="?feed=http://rss.news.yahoo.com/rss/topstories" title="World News">Yahoo! News</a>, 
			<a href="?feed=http://youtube.com/rss/global/top_favorites.rss" title="Funny user-submitted videos">You Tube</a>, 
			<a href="?feed=http://zeldman.com/rss/" title="The father of the web standards movement">Zeldman</a></p>

		</div>

		<div id="sp_results">

			<!-- As long as the feed has data to work with... -->
			<?php if ($success): ?>
				<div class="chunk focus" align="center">

					<!-- If the feed has a link back to the site that publishes it (which 99% of them do), link the feed's title to it. -->
					<h3 class="header"><?php if ($feed->get_link()) echo '<a href="' . $feed->get_link() . '">'; echo $feed->get_title(); if ($feed->get_link()) echo '</a>'; ?></h3>

					<!-- If the feed has a description, display it. -->
					<?php echo $feed->get_description(); ?>

				</div>

				<!-- Add subscribe links for several different aggregation services -->
				<p class="subscribe"><strong>Subscribe:</strong> <a href="<?php echo $feed->subscribe_bloglines(); ?>">Bloglines</a>, <a href="<?php echo $feed->subscribe_google(); ?>">Google Reader</a>, <a href="<?php echo $feed->subscribe_msn(); ?>">My MSN</a>, <a href="<?php echo $feed->subscribe_netvibes(); ?>">Netvibes</a>, <a href="<?php echo $feed->subscribe_newsburst(); ?>">Newsburst</a><br /><a href="<?php echo $feed->subscribe_newsgator(); ?>">Newsgator</a>, <a href="<?php echo $feed->subscribe_odeo(); ?>">Odeo</a>, <a href="<?php echo $feed->subscribe_podnova(); ?>">Podnova</a>, <a href="<?php echo $feed->subscribe_rojo(); ?>">Rojo</a>, <a href="<?php echo $feed->subscribe_yahoo(); ?>">My Yahoo!</a>, <a href="<?php echo $feed->subscribe_feed(); ?>">Desktop Reader</a></p>


				<!-- Let's begin looping through each individual news item in the feed. -->
				<?php foreach($feed->get_items() as $item): ?>
					<div class="chunk">

						<?php
						// Let's add a favicon for each item. If one doesn't exist, we'll use an alternate one.
						if (!$favicon = $feed->get_favicon())
						{
							$favicon = './for_the_demo/favicons/alternate.png';
						}
						?>

						<!-- If the item has a permalink back to the original post (which 99% of them do), link the item's title to it. -->
						<h4><img src="<?php echo $favicon; ?>" alt="Favicon" class="favicon" /><?php if ($item->get_permalink()) echo '<a href="' . $item->get_permalink() . '">'; echo $item->get_title(); if ($item->get_permalink()) echo '</a>'; ?>&nbsp;<span class="footnote"><?php echo $item->get_date('j M Y, g:i a'); ?></span></h4>

						<!-- Display the item's primary content. -->
						<?php echo $item->get_content(); ?>

						<?php
						// Check for enclosures.  If an item has any, set the first one to the $enclosure variable.
						if ($enclosure = $item->get_enclosure(0))
						{
							// Use the embed() method to embed the enclosure into the page inline.
							echo '<div align="center">';
							echo '<p>' . $enclosure->embed(array(
								'audio' => './for_the_demo/place_audio.png',
								'video' => './for_the_demo/place_video.png',
								'mediaplayer' => './for_the_demo/mediaplayer.swf',
								'altclass' => 'download'
							)) . '</p>';

							if ($enclosure->get_link() && $enclosure->get_type())
							{
								echo '<p class="footnote" align="center">(' . $enclosure->get_type();
								if ($enclosure->get_size())
								{
									echo '; ' . $enclosure->get_size() . ' MB';								
								}
								echo ')</p>';
							}
							if ($enclosure->get_thumbnail())
							{
								echo '<div><img src="' . $enclosure->get_thumbnail() . '" alt="" /></div>';
							}
							echo '</div>';
						}
						?>

						<!-- Add links to add this post to one of a handful of services. -->
						<p class="footnote favicons" align="center">
							<a href="<?php echo $item->add_to_blinklist(); ?>" title="Add post to Blinklist"><img src="./for_the_demo/favicons/blinklist.png" alt="Blinklist" /></a>
							<a href="<?php echo $item->add_to_blogmarks(); ?>" title="Add post to Blogmarks"><img src="./for_the_demo/favicons/blogmarks.png" alt="Blogmarks" /></a>
							<a href="<?php echo $item->add_to_delicious(); ?>" title="Add post to del.icio.us"><img src="./for_the_demo/favicons/delicious.png" alt="del.icio.us" /></a>
							<a href="<?php echo $item->add_to_digg(); ?>" title="Digg this!"><img src="./for_the_demo/favicons/digg.png" alt="Digg" /></a>
							<a href="<?php echo $item->add_to_magnolia(); ?>" title="Add post to Ma.gnolia"><img src="./for_the_demo/favicons/magnolia.png" alt="Ma.gnolia" /></a>
							<a href="<?php echo $item->add_to_myweb20(); ?>" title="Add post to My Web 2.0"><img src="./for_the_demo/favicons/myweb2.png" alt="My Web 2.0" /></a>
							<a href="<?php echo $item->add_to_newsvine(); ?>" title="Add post to Newsvine"><img src="./for_the_demo/favicons/newsvine.png" alt="Newsvine" /></a>
							<a href="<?php echo $item->add_to_reddit(); ?>" title="Add post to Reddit"><img src="./for_the_demo/favicons/reddit.png" alt="Reddit" /></a>
							<a href="<?php echo $item->add_to_segnalo(); ?>" title="Add post to Segnalo"><img src="./for_the_demo/favicons/segnalo.png" alt="Segnalo" /></a>
							<a href="<?php echo $item->add_to_simpy(); ?>" title="Add post to Simpy"><img src="./for_the_demo/favicons/simpy.png" alt="Simpy" /></a>
							<a href="<?php echo $item->add_to_spurl(); ?>" title="Add post to Spurl"><img src="./for_the_demo/favicons/spurl.png" alt="Spurl" /></a>
							<a href="<?php echo $item->add_to_wists(); ?>" title="Add post to Wists"><img src="./for_the_demo/favicons/wists.png" alt="Wists" /></a>
							<a href="<?php echo $item->search_technorati(); ?>" title="Who's linking to this post?"><img src="./for_the_demo/favicons/technorati.png" alt="Technorati" /></a>
						</p>

					</div>

				<!-- Stop looping through each item once we've gone through all of them. -->
				<?php endforeach; ?>

			<!-- From here on, we're no longer using data from the feed. -->
			<?php endif; ?>

		</div>

		<div>
			<!-- Display how fast the page was rendered. -->
			<p class="footnote">Page processed in <?php $mtime = explode(' ', microtime()); echo round($mtime[0] + $mtime[1] - $starttime, 3); ?> seconds.</p>

			<!-- Display the version of SimplePie being loaded. -->
			<p class="footnote">Powered by <a href="<?php echo SIMPLEPIE_URL; ?>"><?php echo SIMPLEPIE_NAME . ' ' . SIMPLEPIE_VERSION . ', Build ' . SIMPLEPIE_BUILD; ?></a>.  Run the <a href="../compatibility_test/sp_compatibility_test.php">SimplePie Compatibility Test</a>.  SimplePie is &copy; 2004&ndash;<?php echo date('Y'); ?>, Ryan Parman and Geoffrey Sneddon, and licensed under the <a href="http://www.opensource.org/licenses/bsd-license.php">BSD License</a>.</p>
		</div>

	</div>

</div>

</body>
</html>
