<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en-AU">
  <head>
    <meta http-equiv="content-type" content="application/xhtml+xml; charset=UTF-8" />
    <meta name="author" content="haran" />
    <meta name="generator" content="haran" />
	<META NAME="description" CONTENT="Mixxx is free cross-platform digital DJ software with professional features like
									  parallel visual displays, beat estimation, beat matching, skins, EQs..." />
	<META NAME="keywords" CONTENT="dj, dj software, free dj software, mixxx, mp3 dj, crossfader, digital dj, beatmix, beatmixing, mp3, open source, mixing, mixer" />
    <link rel="SHORTCUT ICON" href="/favicon.ico"><!-- TODO: put a real favicon.ico on server, this one is blank, just to prevent IE 404 apache log spam -->
    <link rel="stylesheet" type="text/css" href="css/prosimii-screen.css" media="screen, tv, projection" title="Default" />
    <link rel="stylesheet alternative" type="text/css" href="css/prosimii-print.css" media="screen" title="Print Preview" />
    <link rel="stylesheet" type="text/css" href="css/prosimii-print.css" media="print" />
	<link rel="alternate" type="application/rss+xml" title="Mixxx RSS Feed" href="http://feeds.feedburner.com/MixxxNews" />
    <!--[if lt IE 7.]>
    <script defer type="text/javascript" src="js/pngfix.js"></script>
    <![endif]-->
    <script type="text/javascript" src="js/jquery.js"></script>  
    <script type="text/javascript" src="js/jquery.cycle.pack.js"></script>
    <title>Mixxx | Free Digital DJ Software</title>    
    <script type="text/javascript">
$(function() {


    $(".splash_link").click(function() {
    	var value = $(this).attr("splash_idx");
    	//alert("value =" + value);
    	$(".splash:visible").fadeOut(500);        //Wax on
		showdiv = $("div#splash" + value);
    	setTimeout('showdiv.fadeIn(500);', 500);  //Wax off
    });
   
});
    </script>
  </head>
  <body>
    <!-- For non-visual user agents: --><div id="top"><a href="#main-copy" class="doNotDisplay doNotPrint">Skip to main content.</a></div>
    <?php  include 'header.php';?>
    <!-- ##### Main Copy ##### -->
    <div id="main-copy">
      <div class="rowOfBoxes">
        <div class="half noBorderOnLeft" style="text-align: center;">
			<div id="main_fade11" style="height: 400px;">
				<div class="splash" id="splash1" style="width: 375px"> <!-- Splash page -->
                    <a href="features.php"><img border="0px" src="images/tagline.png" alt="Beatmix, better." /></a>
                    <br><br>
                    <a href="screenshots.php"><img border="0px" src="images/mixxx_screen2_small.png" alt="Mixxx Screenshot" /></a>	  
				</div><br/>
				<div class="splash" id="splash2" style="display: none;">
					<h1>Vinyl Control</h1>
					<img src="images/splash_vinyl1.png" alt="Vinyl Control"/>
					<br/>
					Use your turntables to control Mixxx without buying expensive hardware.
				</div>
				<div class="splash" id="splash3" style="display: none;">
					<h1>MIDI Control</h1>
					<img src="images/splash_midi1.png" alt="MIDI Control" />
					<br/>
					Mixxx supports a wide range of MIDI controllers, including the Hercules MK2 and RMX.
				</div>                            
				<div class="splash" id="splash4" style="display: none;">
				<h1>Multiple Soundcards</h1>
				<br/>
				<img src="images/splash_multiplesoundcard1.png" alt="Multiple Soundcards"/>
				<br/>
				Stuck with a single audio jack on your laptop? Plug in an external USB or Firewire soundcard and
				take advantage of headphone cueing.</div>
				<div class="splash" id="splash5" style="display: none;">
				<h1>Low Latency</h1>
				<br/>
				<img src="images/splash_lowlatency1.png" alt="Low Latency"/>
				<br/>
				Mixxx's adjustable latency setting allows you to get the best
				performance from your soundcard. Sub-10 ms latencies can be
				reached with modern PCs.</div>
			</div>
			<a class="splash_link" splash_idx="1" href="javascript:void(0);" style="clear:both">&lt;&lt;</a> | 
			<a class="splash_link" splash_idx="2" href="javascript:void(0);" style="clear:both">Vinyl Control</a> |
			<a class="splash_link" splash_idx="3" href="javascript:void(0);" style="clear:both">MIDI Control</a> | 
			<a class="splash_link" splash_idx="4" href="javascript:void(0);" style="clear:both">Multiple Soundcards</a> | 
			<a class="splash_link" splash_idx="5" href="javascript:void(0);" style="clear:both">Low Latency</a>
	  	</div>    
         
	  	<!--
	  	<div class="oneThird noBorderOnLeft" style="padding-right: 50px;"> 
	  	   <img src="images/Mixxx_screen_floor.png" align=center style="padding-left: 5px; padding-top: 35px;" border=0px>
        </div>
		-->
        <div class="half noBorderOnLeft">
            <p style="font-size: 1.5em; text-align: center;">
                <br><br><br>
                <b>Mixxx</b> is free, open source DJ software that gives
			    you everything you need to perform live mixes.
		    </p>

  	 		<script type="text/javascript">    
					var OSName="your OS";
					if (navigator.appVersion.indexOf("Win")!=-1) OSName="Windows";
					
					// OS X why do you come in so many binary-incompatible flavours?
					if (navigator.appVersion.indexOf("Mac")!=-1) 
					{
						OSName="Mac OS X";
						if ((navigator.userAgent.indexOf("10.5") != -1) //Leopard, Firefox
 						    || (navigator.userAgent.indexOf("10_5") != -1)) //Leopard, Webkit
						{
							OSName += " 10.5";
						}
						else if ((navigator.userAgent.indexOf("10.4") != -1) //Tiger, Firefox
 							 || (navigator.userAgent.indexOf("10_4") != -1)) //Tiger, Webkit
						{
							OSName += " 10.4";
						}
						
						if (navigator.userAgent.indexOf("Intel Mac")!=-1) //Intel machines
						{
							//OSName += " (Intel)";
							OSName += " (Universal)";
						}
					}

					if (navigator.appVersion.indexOf("X11")!=-1) OSName="Linux";
					if (navigator.appVersion.indexOf("Linux")!=-1) OSName="Linux";
					
			</script>     
					
  	 		<!-- <div class="downloadFloat" onclick="location.href='downloadlatest.php?os=' + OSName;" style="cursor:pointer;">
			</div>-->
				<center><img src="images/download_now.png" onclick="location.href='downloadlatest.php?os=' + OSName;" border="0px" style="vertical-align: middle; text-align:center; cursor:pointer;" /></center>
			<br/>
			<p class="downloadSmall" style="text-align: right;">
				Mixxx 1.7.2 for 
  	 		<script type="text/javascript">
					document.write(OSName);
			</script><br/>
			<a href="download.php">Other platforms...</a>
			  	 		
			<br/>
			<br/>
			
			<!-- PayPal generated donate stuff -->
			<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
			<input type="hidden" name="cmd" value="_s-xclick"> <p style="text-align: right;">
			<input type="image" src="https://www.paypal.com/en_GB/i/btn/btn_donate_LG.gif" border="0" name="submit" alt="PayPal - The safer, easier way to pay online.">
			<img alt="" border="0" src="https://www.paypal.com/en_GB/i/scr/pixel.gif" width="1" height="1">
			</p>
			<input type="hidden" name="encrypted" value="-----BEGIN PKCS7-----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-----END PKCS7-----
			">
			</form>
			</p> <!-- downloadSmall class -->
			  	 
         </div>
      </div>

      <div class="rowOfBoxes dividingBorderAbove">
        <div class="oneThird noBorderOnLeft">
          <h1>Latest News <a href="http://feeds.feedburner.com/MixxxNews" rel="alternate" type="application/rss+xml"><img src="http://www.feedburner.com/fb/images/pub/feed-icon16x16.png" alt="" style="vertical-align:middle;border:0;padding-bottom:2px;" /></a></h1>

<!-- -NEWS START- -->
<?php 
	//Mixxx RSS feed (news with full text)
	//$url = "http://mixxxblog.blogspot.com/feeds/posts/default"; 
	$url = "http://feeds.feedburner.com/MixxxNews";
	//$url = "http://feeds.feedburner.com/MixxxNews?format=rss2";

	require 'SimplePie/simplepie.inc';
	 
	$feed = new SimplePie();
	$feed->set_feed_url($url);
	$feed->init();
	$feed->handle_content_type();

	//Useful for figuring out which fields we're supposed to use in the loop below
	//die(var_dump($rss));

	//Iterate through each item in the RSS feed.
	$itemcount = $feed->get_item_quantity();
	$count = 0;
	for ($i = 0; $i < $itemcount && $count < 2; $i++)
	{
		$item = $feed->get_item($i);
		$title = $item->get_title();
		$desc  = $item->get_content();
		$url   = $item->get_link();
		$date  = $item->get_date('l, F jS Y');
		
		//Trim the description and add a link to the full thing, if necessary.
		if (strlen($desc) > 60)
		{
			$desc = str_replace("<br />", " ", $desc); //Replace line breaks with spaces. (Looks better)
			$desc = strip_tags($desc); //Strip remaining HTML tags...
			$desc = substr($desc, 0, 150); //Truncate the description to 150 characters
			$desc = trim($desc); //Strip whitespace

			// Truncate at end of last word and remove any trailing punctuation
			$desc = preg_replace('/\s+?(\S+)?$/', '', $desc);
			$desc = preg_replace('/[^a-zA-Z]$/', '', $desc);

			$desc .= "... <a href=\"$url\">Read more</a>";
		}
		
		// Create timestamp from date, then format
		//$stamp =  mktime(0, 0, 0, substr($date, 5, 2), substr($date, 8, 2), substr($date, 0, 4));
		//$date = date('l, F jS Y', $stamp);
		
		echo "<a href=$url class=\"newsHeading\">$title</a>\n";
		echo "<p class=\"newsDate\">$date</p>\n";
		echo "<p class=\"newsSummary\">$desc</p>\n\n";

		// Update displayed item count
		$count++;
	}
?>         
          <div class="more"><a href="http://mixxxblog.blogspot.com">More News &raquo;</a></div>

          <p class="filler"><!-- Filler para to extend left vertical line --></p>
<!-- -NEWS END- -->

        </div>
		<div class="oneThird noBorderOnLeft">
 		<center><img src="images/dj_kid90small.jpg" style="padding-top: 25px;"></center>
        </div>
        <div class="oneThird">
          <h1>Why Mixxx?</h1>
          	<p>
          	   <ul>
          		 <li>Free, open source DJ software for Windows, Mac OS X, and Linux</li>
          		 <li>MIDI controller support</li>
          		 <li>Superior mixing engine with
          		     recording, vinyl control,
          		     and more</li>
          		 <li>Created for DJs, by DJs</li>
          		 <li>Check out our <a href="features.php">full list</a> of battle-tested
          		     and DJ approved <a href="features.php">features</a>!
          	   </ul>
            </p> 
        </div>
      </div>
    </div>

    <!-- ##### Footer ##### -->
	<?php include 'footer.php' ?>
  </body>
</html>
