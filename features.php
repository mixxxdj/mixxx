<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en-AU">
  <head>
    <meta http-equiv="content-type" content="application/xhtml+xml; charset=UTF-8" />
    <meta name="author" content="haran" />
    <meta name="generator" content="haran" />

    <link rel="stylesheet" type="text/css" href="./css/prosimii-screen.css" media="screen, tv, projection" title="Default" />
    <link rel="stylesheet alternative" type="text/css" href="./css/prosimii-print.css" media="screen" title="Print Preview" />
    <link rel="stylesheet" type="text/css" href="./css/prosimii-print.css" media="print" />
    <!--[if lt IE 7.]>
    <script defer type="text/javascript" src="js/pngfix.js"></script>
    <![endif]-->
	<script type="text/javascript" src="js/jquery.js"></script>  
    <script type="text/javascript">
$(function() {
    $(".features_link").click(function() {
    	var value = $(this).attr("features_idx");
    	//alert("value =" + value);
    	$(".features:visible").fadeOut(500);        //Wax on
		showdiv = $("div#features" + value);
    	setTimeout('showdiv.fadeIn(500);', 500);  //Wax off
    });
   
});
    </script>
    
    <title>Mixxx | Features</title>
  </head>

  <body>
    <!-- For non-visual user agents: -->
      <div id="top"><a href="#main-copy" class="doNotDisplay doNotPrint">Skip to main content.</a></div>

    <!-- ##### Header ##### -->
	<?php  include 'header.php';?>
	
    <!-- ##### Main Copy ##### -->

    <div id="main-copy">
      <div class="rowOfBoxes">
        <div class="twoThirds noBorderOnLeft">
          <!--<img src="images/Mixxx_screen_floor.png" align=right style="padding-left: 25px; padding-top: 35px;" border=0px>-->
          <h1>Features</h1>
          <br>
          <div id="main_fade" style="margin:0 auto; text-align: center;">
				<div class="features" id="features1"><br> <!-- Splash page -->
				<center><img src="images/features_mixxxscreen.png"></center><br>
				  <h1>What is Mixxx?</h1>
				  <p>Mixxx is <b>free mixing software</b> for DJs, providing everything needed
				  to create live mixes. It allows you to beatmatch songs and crossfade them together, like you would
				  with turntables and a mixer. </p> 
				  <p>If you're new to DJing, don't sweat it - we have you covered. Mixxx runs without any extra
				  hardware, and is a full replacement for a traditional "turntables and mixer" DJ setup. <b>Thousands
				  of people learn to DJ using Mixxx each year!</b></p>
				  <p>For experienced DJs, Mixxx offers a wide range of advanced features. Our comprehensive
				  <b>MIDI controller and vinyl control support</b> gives you the tactile feel you need for ultimate
				  control of your mixes. 
				  To obtain the lowest latency and highest responsiveness, Mixxx uses hardware video acceleration
				  and takes advantage of multi-core CPUs.</p> 
				  <br/>
				<center><img src="images/features_mixing.png"></center>
				<h1>Advanced Mixing Engine</h1>
				<p>Whether you're blending psy-trance or mashing up
                    the latest Top 40 tracks, Mixxx's looping and hot
                    cue controls give you the power to be more creative
                    with your mixes.</p>
                <p>Between BPM estimation and the parallel waveform displays, beatmatching
                   has never been easier. Our new ramping pitchbend buttons allow you to subtley
                   nudge songs back in sync without anyone noticing.</p>
                <p>Stretch your music without changing the pitch! Mixxx's pitch-independent
                   time stretch locks the pitch of your music while you're mixing, so you never
                   have any awkward detuned moments. Our alternative vinyl emulation mode changes
                   the pitch you change the tempo, and is perfect for scratching with a MIDI controller
                   or with vinyl control.</p>
				<center><img src="images/features_midi.png"></center> 
 				<h1>DJ MIDI Controllers</h1>
				<p>Use the latest and greatest DJ MIDI controllers with Mixxx!
				  Advanced MIDI controller support is provided by our groundbreaking
				   <b>MIDI scripting engine</b>. Take advantage of your MIDI controller
				   with our JavaScript-like language. Check it out:</p>
				<p>Mixxx supports most popular DJ MIDI controllers like the <b>Hercules DJ 
				   Console MK2, RMX, and Stanton SCS.3d</b>. In addition to our natively
				   supported controllers, our <b>MIDI learning wizard</b> helps you set 
				   up other controllers with Mixxx.</p>
 				<center><img src="images/features_vinylcontrol_smaller.png"></center>                   
                <h1>Vinyl Control</h1>
                <p>Using a turntable and <b>timecoded vinyl</b>, Mixxx's playback can
                   be syncronized to the turntable. This let's you scratch and 
                   mix with your digital music collection as if it were on vinyl!</p>
                <p>Mixxx supports Serato, Traktor Scratch, and FinalScratch vinyl, as
                   well as Serato CD for control from CDJ units. Position synchronization
                   ("Absolute Mode") is fully supported for each of
                   these timecodes as well.</p>
                <p>Even better, you can <b>save money and use your existing soundcard</b>.
                   Mixxx's vinyl control works 
                   with any soundcard that has a stereo line-input jack. This
                   means most consumer and professional soundcards can be used for vinyl control
                   with Mixxx, so there's no need to purchase expensive vinyl control hardware.
                </p>
                <center><iframe class="youtube-player" type="text/html" width="445" height="364" src="http://www.youtube.com/embed/nAqI4HAcQi4" frameborder="0"></iframe></center>
				<center><img src="images/features_flexibility.png"</img></center>
				<h1>Flexibility and Freedom</h1>
				<p>We love to customize and tinker, and so we've made Mixxx as 
				   flexible as we can. From our cross-platform approach, to Mixxx's
				   skinnable interface and customizable EQ shelves, we've tried to
				   give power users more options. Did we mention our crossfader curve control
				   let's you adjust for mixing, scratching, or anywhere in between?
				
				</div><br>

			</div>
 
	  	 <ul>
            
        </div>
        
<!-- -Full features START- -->
        <div class="oneThird">
          <h1>Full Feature List</h1>
          <ul>
          <li>Cross-platform (Windows XP/Vista/7, Mac OS X, Linux)</li>
          <li>Free and open source (GPL v2)</li>          
          <li>Parallel waveform displays</li>
          <li>Waveform summaries</li>
          <li>MP3, OGG, WAVE, and FLAC playback</li>
          <li>Extra playback formats through plugins</li>
          <li>Fast, database-powered library</li>
          <li>Automatic crossfading with Auto DJ</li>
          <li>Crates and playlists</li>
          <li>Reads iTunes and Rhythmbox libraries</li>
          <li>Pitch-independent time stretch (key lock)</li>
          <li>Vinyl emulation</li>
          <li>Hot cues</li>
          <li>Looping</li>
          <li>Wave recording</li>
          <li>BPM detection and estimation</li>
          <li>Bulk BPM analysis</li>
          <li>Community Supported MIDI Controllers:
            <ul>
            <li>Stanton SCS.3d</li>
            <li>Stanton SCS.3m</li>
            <li>Hercules DJ Console MK2</li>
            <li>Hercules DJ MP3 Control</li>
            <li>Hercules DJ Console RMX</li>
            <li>Hercules DJ Console Steel</li>
            <li>Stanton SCS.1m</li>
            <li>Mixman DM2</li>
            <li>Tascam US-428</li>
            <li>M-Audio X-Session Pro</li>
            <li>Evolution X-Session</li>
            <li>FaderFox DJ2</li>
            <li>Vestax VCI-100</li>
            <li>Numark Total Control</li>
            <li>... and more! See our <a href="http://mixxx.org/wiki/doku.php/hardware_compatibility">Hardware Compatibility</a> page.</li>
            </ul>
          </li>  
            <li>Multichannel soundcard support (playback and capture)</li>
            <li>Multiple soundcard support</li>
            <li>Adjustable EQ shelves</li>
            <li>Crossfader curve control</li>
            <li>Skinnable interface with extra skins bundled</li>
            <li>Advanced MIDI scripting engine</li>
            <li>Multiple simultaneous MIDI controllers</li>
            <li>Adjustable pitch range</li>
            <li>Ramping pitchbend controls</li>
            <li>Multi-core CPU support</li>
            <li>24-bit/96000 Hz playback and capture</li>
            <li>Crystal clear audio</li>
            <li>Hardware video acceleration</li>
            <li>Vinyl control support for:
                <ul>
                    <li>Serato CV02 vinyl</li>
                    <li>Traktor Scratch vinyl</li>
                    <li>FinalScratch Standard vinyl</li>
                    <li>FinalScratch Scratch vinyl</li>
                    <li>Serato CD</li>
                    <li><a href="http://www.mixxx.org/wiki/doku.php/vinyl_control">Read more...</a></li>
                </ul>
            </li>
          </ul>

<!-- -Full features END- -->
	    </div>
      </div>

    <!-- ##### Footer ##### -->
	<?php include 'footer.php' ?>

  </body>
</html>
