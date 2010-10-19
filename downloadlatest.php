<?php
$os = $_GET["os"];
$url = "http://downloads.mixxx.org/mixxx-1.8.1/mixxx-1.8.1-";
if ($os == "Windows") {
	$url .= "win32.exe";
}
/*else if ($os == "Mac OS X")
	$url .= "macppc.dmg"; 
else if ($os == "Mac OS X 10.5 (Universal)")
	$url .= "osx-universal.dmg";
else if ($os == "Mac OS X 10.4 (Universal)") 
	$url .= "osx-universal.dmg"; */
	//$url = "http://downloads.mixxx.org/mixxx-1.6.0/mixxx-1.6.0-macintel-tiger.dmg"; 
else // if ($os == "Linux")
{
	header("Location:http://www.mixxx.org/download.php");
	return;
}	
	header("Location:" . $url);
?>
