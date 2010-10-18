<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en-AU">
  <head>
    <meta http-equiv="content-type" content="application/xhtml+xml; charset=UTF-8" />
    <meta name="author" content="haran" />
    <meta name="generator" content="haran" />

    <link rel="stylesheet" type="text/css" href="./css/prosimii-screen.css" media="screen, tv, projection" title="Default" />
    <link rel="stylesheet" type="text/css" href="./css/download.css" media="screen, tv, projection" title="Default" />
    <link rel="stylesheet alternative" type="text/css" href="./css/prosimii-print.css" media="screen" title="Print Preview" />
    <link rel="stylesheet" type="text/css" href="./css/prosimii-print.css" media="print" />
    <link rel="stylesheet" type="text/css" href="./css/jquery-ui/dark-hive/jquery-ui-1.7.2.custom.css"/>
    
    <script type="text/javascript" src="js/jquery.js"></script>
    <script type="text/javascript" src="js/jquery-ui.js"></script>
    
    <script type="text/javascript">
        $(document).ready(function() {
            $("#getwindlg").dialog({autoOpen : false});
            
            $("#getwin").bind("click", function(e) {
                $("#getwindlg").dialog("open");
                //$("#dgetwindlg").css("visibility", "visible");
                return false;
            });

            $("#ubuntudlg").dialog({autoOpen : false, width : 500});
            
            $("#getubu").bind("click", function(e) {
                $("#ubuntudlg").dialog("open");
                //$("#dgetwindlg").css("visibility", "visible");
                return false;
            });
            
        });
    </script>
    <!--[if lt IE 7.]>
    <script defer type="text/javascript" src="js/pngfix.js"></script>
    <![endif]-->
    
    <title>Mixxx | Download</title>
  </head>

  <body>
    <!-- For non-visual user agents: -->
      <div id="top"><a href="#main-copy" class="doNotDisplay doNotPrint">Skip to main content.</a></div>

    <!-- ##### Header ##### -->
	<?php  include 'header.php';?>
	<div id="getwindlg" style="display: none;" title="32-bit or 64-bit?">
		<p>The 64-bit version of Mixxx requires a <b>64-bit version of Windows</b>. All
                   other Windows users should download the 32-bit version of Mixxx.<br><br>
	           To check if you're running a 64-bit version of Windows:<br>
		1. Open <b>System</b> by clicking the Start button, clicking Control Panel, clicking System and Maintenance, and then clicking System.<br>
		2. Under System, if you see the phrase "64-bit Operating System" or "64-bit Edition", your computer can run the 64-bit version of Mixxx.<br>
		</p>
	</div>

	<div id="ubuntudlg" style="display: none;" title="Ubuntu Downloads">
	   <center><b>Download:</b></center>
	   <br>
	   <b>Ubuntu 10.04 (Lucid) and 10.10 (Maverick):</b><br>
	   Open a terminal, and enter:
	   <pre>
sudo add-apt-repository ppa:mixxxdevelopers/ppa
sudo apt-get update
sudo apt-get install mixxx libportaudio2
	   </pre>
	   This will install the latest version of Mixxx from the <a href="https://launchpad.net/~mixxxdevelopers/+archive/ppa">Mixxx Developers PPA</a> on Launchpad.

	   <b>Ubuntu 10.10 (Maverick):</b><br>
	   Our older Mixxx 1.8.0 release can be installed directly from the Ubuntu Software Centre.<br><br>

	</div>
	
    <!-- ##### Main Copy ##### -->

    <div id="main-copy">
    <div id="main-copy-inside">
    <br>
		        <h1>Download</h1>
		        
		        <div>

			        <div id="download">
			        <br>
			        
			        	<!--<div id="screenshotbox">
			        		
			        	</div>-->
					    <div id="windows">
						    <table> 
						        <tr>
						        	<td>
						        	<img src="images/downloads_win.png">
						        	</td>
						        	<td width="50%"><a href="http://downloads.mixxx.org/mixxx-1.8.1/mixxx-1.8.1-win32.exe"  onClick="javascript: pageTracker._trackPageview('/downloads/181win32'); ">Mixxx 1.8.1</a><br>(32-bit)
						            </td>
						            <td width="50%"><a href="http://downloads.mixxx.org/mixxx-1.8.1/mixxx-1.8.1-win64.exe"  onClick="javascript: pageTracker._trackPageview('/downloads/181win64'); ">Mixxx 1.8.1</a><br>(64-bit)

                                    <!--
		            			    <p style="font-size: 0.6em;margin:0px;padding:0px;">
		            			        <a id="getwin" href="#win32vs64">Not sure which to get?</a>
		            			    </p>
                                    -->
						            </td>
						            <td><img src="images/windows.png"></td>
						            
		        			    </tr>
		        			    <tr><td colspan=2 style="vertical-align: bottom;">
		        			    <div style="float:middle; clear:left;">
		        			    </div>
		        			    </td></tr>
						    </table>
					    </div>
					    <div id="mac">
						    <table>
						        <tr>
						            <td>
						                <img src="images/downloads_mac.png">
						            </td>
						            <td width="100%"><a href="http://downloads.mixxx.org/mixxx-1.8.1/mixxx-1.8.1-macintel.dmg"  onClick="javascript: pageTracker._trackPageview('/downloads/181osxintel'); ">Mixxx 1.8.1</a><br>
						    		(10.5+, Intel)<br>
						    		</td>
						            <td><img src="images/mac.png"></td>
						        </tr>
						    </table>						    
					    </div>
					    <div id="linux">
						    <table>
						        <tr>
						        	<td><img src="images/downloads_linux.png"></td>
						            <td width="30%">
						             <a href="http://downloads.mixxx.org/mixxx-1.8.1/mixxx-1.8.1-src.tar.gz"  onClick="javascript: pageTracker._trackPageview('/downloads/181linuxsrc'); ">Mixxx 1.8.1</a><br>
						    (Source)
						            </td>
						            <td width="30%">
						                <a id="getubu" href="#ubuntudlg" onClick="javascript: pageTracker._trackPageview('/downloads/181ubuntu'); ">Mixxx 1.8.1</a><br>(Ubuntu)
						    		</td>
						    		<td><img src="images/ubuntu.png" align="center" border="0px">
						    		</td>
						        </tr>
						    </table>						   
					    </div>
<!--
<div id="beta">
							<img src="images/beta.png" style="float: left; padding: 10px">
							<p>The Mixxx 1.8.1 beta includes a brand new library, looping controls, support for multiple MIDI controllers, and more! <p>If you'd like to try out the latest features and are willing to <a href="https://bugs.launchpad.net/mixxx/">report any bugs</a> you find, try the latest <b>(unstable) beta release</b>:</p>
<br>
<ul><li><a href="http://downloads.mixxx.org/mixxx-1.8.1-beta2/mixxx-1.8.1-beta2-win32.exe" onClick="javascript: pageTracker._trackPageview('/downloads/180beta2win32'); ">Mixxx 1.8.1-beta2 for Windows (32-bit)</a></li>
<li><a href="http://downloads.mixxx.org/mixxx-1.8.1-beta2/mixxx-1.8.1-beta2-win64.exe" onClick="javascript: pageTracker._trackPageview('/downloads/180beta2win64'); ">Mixxx 1.8.1-beta2 for Windows (64-Bit)</a><p>(Note: Will ONLY work with 64-bit Windows)</p></li>
<li><a href="http://downloads.mixxx.org/mixxx-1.8.1-beta2/mixxx-1.8.1-beta2-ubuntu-i386.deb" onClick="javascript: pageTracker._trackPageview('/downloads/180beta2ubuntu'); " >Mixxx 1.8.1-beta2 for Ubuntu 9.10+ (i386 deb)</a></li>
<li><a href="http://downloads.mixxx.org/mixxx-1.8.1-beta2/mixxx-1.8.1-beta2-ubuntu-amd64.deb" onClick="javascript: pageTracker._trackPageview('/downloads/180beta2ubuntu'); " >Mixxx 1.8.1-beta2 for Ubuntu 9.10+ (amd64 deb)</a></li>
<li><a href="http://downloads.mixxx.org/mixxx-1.8.1-beta2/mixxx-1.8.1-beta2-osx-universal.dmg" onClick="javascript: pageTracker._trackPageview('/downloads/180beta2osx'); ">Mixxx 1.8.1-beta2 for Mac OS X 10.4+ (Universal)</a></li>
<li><a href="http://downloads.mixxx.org/mixxx-1.8.1-beta2/mixxx-1.8.1-beta2-src.tar.gz" onClick="javascript: pageTracker._trackPageview('/downloads/180beta2linuxsrc'); ">Mixxx 1.8.1-beta2 for Linux (source)</a></li>
</ul>
</div>
-->
						<div id="donate">
							<h2>Show your Support</h2><br>
							<!--<img src="images/face-grin.png" style="float: left; padding: 10px">-->
							<p>If you enjoy Mixxx or use it professionally, please consider <b>donating to the project</b> using the link below to help support and enhance development. </p>
<center>
<!--
<form action="https://www.paypal.com/cgi-bin/webscr" method="post">
<input type="hidden" name="cmd" value="_s-xclick">
<input type="image" src="https://www.paypal.com/en_GB/i/btn/btn_donate_LG.gif" border="0" name="submit" alt="PayPal - The safer, easier way to pay online.">
<img alt="" border="0" src="https://www.paypal.com/en_GB/i/scr/pixel.gif" width="1" height="1">
<input type="hidden" name="encrypted" value="-----BEGIN PKCS7-----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-----END PKCS7-----
">
</form> -->
<a href="http://www.pledgie.com/campaigns/13624" style="margin-left: 1em; margin-right: 1em;"><img alt="Click here to lend your support to: Mixxx 1.9 Build Server Fundraiser and make a donation at www.pledgie.com !" border="0" src="http://www.pledgie.com/campaigns/13624.png?skin_name=chrome" /></a>
</center>
<br>

						</div>
						<div id="sourcecode">
							<h2>Source Code</h2><br>
							<img src="images/source.png" style="float: right; padding: 10px;">
							<p>Mixxx is also available as source code, licensed under the GPL v2. Please check the LICENSE file for complete licensing information. You can grab the latest distribution or get the latest code directly from BZR:</p>
							<ul>
								<li><a href="http://downloads.mixxx.org/mixxx-1.8.1/mixxx-1.8.1-src.tar.gz" onClick="javascript: pageTracker._trackPageview('/downloads/181linuxsrc'); ">Mixxx 1.8.1 source code</a></li>
		           				<li>Checking out from BZR:
							<pre>bzr branch lp:mixxx</pre></li>
							</ul>
						</div>
					</div>
				</div>
			</div>
		</div>
    </div>

    <!-- ##### Footer ##### -->
	<?php include 'footer.php' ?>
	
  </body>
</html>
