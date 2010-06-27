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
            <a class="features_link" features_idx="1" href="javascript:void(0);" style="clear:both">Overview</a> | 
			<a class="features_link" features_idx="2" href="javascript:void(0);" style="clear:both">Mixing</a> | 
			<a class="features_link" features_idx="3" href="javascript:void(0);" style="clear:both">Vinyl Control</a> |
			<a class="features_link" features_idx="4" href="javascript:void(0);" style="clear:both">Hardware Support</a> | 
			<a class="features_link" features_idx="5" href="javascript:void(0);" style="clear:both">Flexibility</a> | 
			<a class="features_link" features_idx="6" href="javascript:void(0);" style="clear:both">Performance</a>
			<br><br>	
				<div class="features" id="features1"><br> <!-- Splash page -->
				  <h1>Overview</h1><br>
				  <center><img src="images/features_overview.png"></center>
				  <ul>
                      <li><h2>Complete DJ Solution</h2> 
                        <p>Mixxx is a complete package for amateur and professional DJs alike, providing everything you
                           need to create live mixes.</p>
                      </li>
                      <li><h2>New to DJing?</h2>
                        <p>No problem - Mixxx can run without any extra hardware, and is fully functional replacement
                        for a traditional "turntables and mixer" DJ setup.</p>
                      </li>
                      <li><h2>Experienced DJ?</h2>
                        <p>Mixxx supports advanced features like comprehensive MIDI controller support, vinyl control, and
                           multi-core CPU support.</p>
                      </li>
                  </ul>
				  <br/>
				</div><br>
				<div class="features" id="features2" style="display: none;"><br>
				  <h1>Mixing</h1>
				  <center><img src="images/features_mixing.png"></center>
				  <ul>
                      <li><h2>BPM estimation</h2> 
                        <p>Mixxx measures the BPM of each song to help you beat match faster.</p>
                      </li>
                      <li><h2>Parallel waveform displays</h2>
                        <p>See the beats in songs line up when mixing and quickly spot drift.</p>
                      </li>
                      <li><h2>Waveform summaries</h2>
                        <p>Visually see the dynamics of an entire track, just like with vinyl.</p>
                      </li>
                      <li><h2>MP3 / OGG / WAVE / FLAC playback</h2>
                        <p>Mix your tracks without converting your collection.</p>
                      </li>
                      <li><h2>Pitch-independent time stretch</h2>
                          <p>Change the speed of a song without altering the pitch.</p>
                      </li>
                      <li><h2>Vinyl emulation</h2>
                        <p>Adjust the pitch of a song, just like on a real turntable.</p>
                      </li>
                      <li><h2>Wave Recording</h2>
                        <p>Save your mixes on-the-fly.</p>
                      </li>
                  </ul>
				  <br/>
				  
				</div><br>
				<div class="features" id="features3" style="display: none;">
					<h1>Vinyl Control</h1>
					<br>
					<center><img src="images/features_vinylcontrol.png"></center>
					<br>
                    <ul>
                        <li><h2>Serato, Traktor Scratch, and FinalScratch too!</h2>
                        <p>Use your turntables with timecoded vinyl to control Mixxx, without buying expensive hardware. <a href="http://www.mixxx.org/wiki/doku.php/vinyl_control">Read more</a>...</p>
                        </li>
                        <li><h2>Serato CD support</h2>
                        <p>If you've got CDJs, you can use them to control Mixxx with a pair of Serato CDs.</p>
                        </li>	
                        <li><h2>Save Money - Use your Existing Soundcard</h2>
                        <p>Mixxx's vinyl control works with any soundcard that has a stereo line-input jack. This
                           means that most consumer and professional soundcards can be used for vinyl control
                           with Mixxx, so there's no need to purchase expensive kits.<p>
                        </li>	
                        <li><h2>Absolute Mode</h2>
                        <p>Absolute mode is available with every timecode supported by Mixxx, so you can needle drop
                        to seek through a track without losing focus.</p>
                        </li>
                    </ul>
    
                </div>
				<div class="features" id="features4" style="display: none;">
					<h1>Hardware Support</h1>
					<center><img src="images/features_hardware.png"></center>
					<br>
                <ul>
                    <li><h2>MIDI controller support</h2>
                        <p>Control Mixxx with your favourite MIDI devices and hardware like the 
                           Hercules DJ Console MK2 and RMX, M-Audio X-Session Pro, and more!</p>
                    </li>
                    <li><h2>Advanced MIDI Scripting Engine</h2>
                        <p>Script advanced controller functionality using our JavaScript-like language and
                           take full advantage of your MIDI controller. <a href="http://www.youtube.com/watch?v=8DUpTikA8u0">See it in action!</a></p>
                    </li>
                    <li><h2>Multichannel soundcard support</h2>
                        <p>Use Mixxx with high performance 4x4 soundcards like the <a href="http://www.echoaudio.com/">Echo
                        AudioFire</a> line.</p>
                    </li>
                    <li><h2>Multiple soundcard support</h2>
                        <p>If you're stuck with single output jack on your laptop or desktop PC, just plug in an extra stereo
                        soundcard. Mixxx can route your master and headphone outputs to different soundcards for
                        maximum flexibility, and supports most USB and Firewire soundcards.</p>
                    </li>
                </ul>				
                </div>
				
				<div class="features" id="features5" style="display: none;">
				    <h1>Flexibility</h1>
					<center><img src="images/features_flexibility.png"></center>
				<ul>
                    <li><h2>Cross-platform</h2>
                        <p>Mixxx runs on Windows XP, Windows Vista, Mac OS X Leopard, Ubuntu, and other distributions of Linux.</p>
                    </li>
                    <li><h2>Free and Open Source</h2>
                        <p>If you don't like something the way it is, change it! Mixxx is licensed under the GPL v2.</p>
                    </li>
                    <li><h2>Custom EQ Shelves</h2>
                    <p>Adjust the equalizer bands to suite your taste.</p>
                    </li>
                    <li><h2>Crossfader curve control</h2>
                    <p>Slow fade, fast cut, or anywhere in between - Adjust your crossfader curve like on a hardware DJ mixer.</p>
                    </li>
                    <li><h2>Skinnable Interface</h2>
                    <p>Customize Mixxx with the bundled skins or make your own.</p>
                    </li>
                    <li><h2>Adjustable pitch range</h2>
                    <p>Decrease the range for more precise control or increase it for wilder mixes.</p>
                    </li>
                </ul>
				</div>
				<div class="features" id="features6" style="display: none;">
				<h1>Performance</h1>
				<ul>
                    <li><h2>Multi-core support</h2>
                        <p>Fully utilize your CPU for maximum performance and lower latencies.</p>
                    </li>
                    <li><h2>Sub 10-ms latency</h2>
                        <p>Achieve maximum responsiveness for better mixing with Intel Core and newer CPUs.</p>
                    </li>
                    <li><h2>24-bit/96000 Hz playback and capture</h2>
                        <p>Crystal-clear audio for all setups, from cafe gigs to the biggest clubs.</p>
                    </li>
                    <li><h2>Club-worthy sound quality</h2>
                        <p>Our superior mixing engine is designed for fully-accurate audio reproduction.</p>
                    </li>
                    <li><h2>Hardware acceleration</h2>
                        <p>Harness the power of your videocard for faster performance through OpenGL.</p>
                    </li>
				</ul>
				
				</div>
			</div>
 
	  	 <ul>
            
        </div>
        
<!-- -Full features START- -->
        <div class="oneThird">
          <h1>Full Feature List</h1>
          <ul>
          <li>Parallel waveform displays</li>
          <li>Waveform summaries</li>
          <li>MP3, OGG, WAVE, and FLAC playback</li>
          <li>Pitch-independent time stretch (key lock)</li>
          <li>Vinyl emulation</li>
          <li>Wave recording</li>
          <li>Free, open source software</li>
          <li>BPM detection and estimation</li>
          <li>Supported MIDI controllers:
            <ul>
            <li>Hercules DJ Console MK2</li>
            <li>Hercules DJ MP3 Control</li>
            <li>Hercules DJ Console RMX</li>
            <li>Hercules DJ Console Steel
            <li>Stanton SCS.3d</li>
            <li>Stanton SCS.1m</li>
            <li>Mixman DM2</li>
            <li>Tascam US-428</li>
            <li>M-Audio X-Session Pro</li>
            <li>M-Audio Xponent</li>
            <li>Evolution X-Session</li>
            <li>FaderFox DJ2</li>
            <li>Vestax VCI-100</li>
            <li>Numark Total Control</li>
            <li>... and more - See our <a href="http://mixxx.org/wiki/doku.php/hardware_compatibility">Hardware Compatibility</a> page.</li>
            </ul>
          </li>  
            <li>Multichannel soundcard support (playback and capture)</li>
            <li>Multiple soundcard support</li>
            <li>Cross-platform (Windows XP and Vista, Mac OS X, Linux)</li>
            <li>Adjustable EQ shelves</li>
            <li>Crossfader curve control</li>
            <li>Skinnable interface with extra skins bundled</li>
            <li>Advanced MIDI scripting engine</li>
            <li>Adjustable pitch range</li>
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
