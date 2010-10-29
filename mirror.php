<?php
//Variables:
$enableMirroring = true;
$basepath = "";//"/home/mixxx/public_html";

$fullpath = $_GET["f"]; // eg. /home/mixxx/public_html/mixxx-1.8.0/mixxx-1.8.0-win32.exe
if ($fullpath === "" || $fullpath == null) {
    header('Location: http://www.mixxx.org');
    return;
}
//echo $fullpath;
//$patharray = explode("/", $fullpath); 
$shortpath = substr($fullpath, strlen($basepath)); //eg. mixxx-1.8.0/mixxx-1.8.0-win32.exe
//echo "<BR>";
//echo $shortpath;
//echo "<BR>";
$mirrors = array(0=>"http://downloads.mixxx.org",
	         //1=>"http://moo.glines.org/mixxx",
	         1=>"http://web.mit.edu/rryan/www/downloads.mixxx.org/");

$mirror = "";
//Pick a random mirror if it's enabled
if ($enableMirroring)
{
    $mirror = $mirrors[rand(0, count($mirrors)-1)];
}
else {
    $mirror = "http://downloads.mixxx.org";
}
$newURL = $mirror . $shortpath;

//If we're on downloads.mixxx.org, append a ?r to the URL so that
//the .htaccess rewrite rule doesn't bounce us back to this mirroring script.
//In other words, it makes sure we actually get to the file. :-)
if ($mirror === $mirrors[0]) {
    $newURL .= "?r";
}
//echo $newURL;
header('Location: ' . $newURL); 
?>
