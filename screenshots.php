<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en-AU">
  <head>
    <meta http-equiv="content-type" content="application/xhtml+xml; charset=UTF-8" />
    <meta name="author" content="haran" />
    <meta name="generator" content="haran" />

    <link rel="stylesheet" type="text/css" href="css/prosimii-screen.css" media="screen, tv, projection" title="Default" />
    <link rel="stylesheet alternative" type="text/css" href="css/prosimii-print.css" media="screen" title="Print Preview" />
    <link rel="stylesheet" type="text/css" href="css/prosimii-print.css" media="print" />
    <script type="text/javascript" src="js/jquery.js"></script>  
    <script type="text/javascript" src="js/jquery.cycle.pack.js"></script>
    
    <link rel="stylesheet" type="text/css" href="fancybox/jquery.fancybox/jquery.fancybox.css" media="screen" />
    <script type="text/javascript" src="fancybox/jquery.fancybox/jquery.easing.1.3.js"></script>
    <script type="text/javascript" src="fancybox/jquery.fancybox/jquery.fancybox-1.2.1.pack.js"></script>
    <script type="text/javascript">
        $(document).ready(function() {
            $("a.screenshot").fancybox({
                'zoomSpeedIn': 0,
                'zoomSpeedOut': 0,
                'zoomSpeedChange': 0,
                'easingIn': false,
                'easingOut': false
            });
        });
    </script>
    <style>
    /*
        div#wrap {
            width: 500px;
            margin: 50px auto;
        }
    */
    </style>

    <!--[if lt IE 7.]>
        <script defer type="text/javascript" src="js/pngfix.js"></script>
    <![endif]-->
    <title>Mixxx | Screenshots</title>
  </head>

  <body>
    <!-- For non-visual user agents: -->
      <div id="top"><a href="#main-copy" class="doNotDisplay doNotPrint">Skip to main content.</a></div>

    <!-- ##### Header ##### -->
	<?php  include 'header.php';?>

    <!-- ##### Main Copy ##### -->

    <div id="main-copy">
    	<div class="rowOfBoxes">
    		<div class="fullWidth noBorderOnLeft" id="wrap">
			        <h1>Screenshots</h1>
			        
			<br>

			<center>
			<table cellspacing="20px">
			<tr>
    			<td>
    			<b>Mixing</b><br>
    			<br>
    			<a class="screenshot" href="images/screenshots/mixxx_170_1.png" border=0><img src="images/screenshots/mixxx_170_1_th.png" border=0 rel="ibox"></a>
    			</td>
    			<td>
    			<b>Extra Skins</b><br>
    			<br>
    			<a class="screenshot" href="images/screenshots/mixxx_170_collusion_1.png" border=0><img src="images/screenshots/mixxx_160_collusion_1_th.png" border=0 rel="ibox"></a>
    			</td>
    			<td>
    			<b>Preferences</b><br>
    			<br>
    			<?php
    			$prefs = Array(
    			            'sound',
    			            'midi',
    			            'interface',
    			            'eq',
    			            'xfader',
    			            'rec',
    			            'bpm',
    			            'vinyl'
                                );
                        ?>
    			<?php foreach($prefs as $pref):?>
  			<a class="screenshot" rel="prefsgroup" <?php print $pref != 'sound' ? 'style="display:none"': ''?> href="images/screenshots/prefs_170_<?php print $pref;?>.png" border=0><img src="images/screenshots/prefs_170_<?php print $pref;?>_th.png" border=0></a>
  			<?php endforeach; ?>
    			<br><br>
    			</td>
    		</tr>
			</table>
			</center>

<center>Click the screenshots to enlarge.</center>
		</div>
        </div>
    </div>

    <!-- ##### Footer ##### -->
	<?php include 'footer.php' ?>
	
  </body>
</html>
