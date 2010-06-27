<?php

if (version_compare(PHP_VERSION, '5.3', '>='))
{
	error_reporting(E_ALL ^ E_DEPRECATED ^ E_USER_DEPRECATED);
}
else
{
	error_reporting(E_ALL);
}

if (version_compare(PHP_VERSION, '5.1', '>='))
{
	$tz = timezone_identifiers_list();
	date_default_timezone_set($tz[array_rand($tz)]);
}

header('Content-type: text/html; charset=UTF-8');

?><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN">
<html lang="en">
<head>
<title>SimplePie: Unit Tests</title>
<meta http-equiv="content-type" content="text/html; charset=UTF-8">

<style type="text/css">
* {
	margin: 0;
	padding: 0;
}

body {
	font: 12px/18px Verdana, sans-serif;
	color: #333;
	background: #fff url(background.gif) repeat-x top left;
}

#site {
	width: 500px;
	margin: 20px auto;
}

a {
	color: #000;
	text-decoration: underline;
}

a:hover {
	color: #fff;
	background-color: #333;
	text-decoration: none;
}

h1 {
	font-size: 18px;
	margin: 30px 0 10px 0;
	text-align: center;
}

h1 + p {
	text-align: center;
}

h2 {
	font-size: 16px;
	margin: 20px 0 5px 0;
	padding-top: 20px;
	border-top: 1px solid #ccc;
}

h2 + p + p, h2 + p + p a {
	font-size: 10px;
	line-height: 12px;
	color: #666;
}

#summary + p + p {
	margin: 20px 0 0 0;
	padding-top: 10px;
	border-top: 1px solid #ccc;
}

small {
	font-size: 10px;
}

.pass {
	color: green;
}

.fail {
	color: red;
}
</style>

<script type="text/javascript">
// Sleight - Alpha transparency PNG's in Internet Explorer 5.5/6.0
// (c) 2001, Aaron Boodman; http://www.youngpup.net

if (navigator.platform == "Win32" && navigator.appName == "Microsoft Internet Explorer" && window.attachEvent) {
	document.writeln('<style type="text/css">img, input.image { visibility:hidden; } </style>');
	window.attachEvent("onload", fnLoadPngs);
}

function fnLoadPngs() {
	var rslt = navigator.appVersion.match(/MSIE (\d+\.\d+)/, '');
	var itsAllGood = (rslt != null && Number(rslt[1]) >= 5.5);

	for (var i = document.images.length - 1, img = null; (img = document.images[i]); i--) {
		if (itsAllGood && img.src.match(/\png$/i) != null) {
			var src = img.src;
			var div = document.createElement("DIV");
			div.style.filter = "progid:DXImageTransform.Microsoft.AlphaImageLoader(src='" + src + "', sizing='scale')";
			div.style.width = img.width + "px";
			div.style.height = img.height + "px";
			img.replaceNode(div);
		}
		img.style.visibility = "visible";
	}
}
</script>

</head>

<body>

<div id="site">
	<h1><img src="logo.png" alt="SimplePie Compatibility Test" title="SimplePie Compatibility Test"></h1>
	<p><a href="#summary">Skip to the results</a> | 
	<?php
	if (isset($_GET['remote'])) echo '<a href="?">Re-run without remote tests</a>';
	else echo '<a href="?remote=true">Re-run with remote tests</a>';
	?>
	</p>
<?php

require_once '../simplepie.inc';
require_once 'functions.php';

$tests = array(
	'absolutize',
	'date',
	'feed_category_label',
	'feed_copyright',
	'feed_description',
	'feed_image_height',
	'feed_image_link',
	'feed_image_title',
	'feed_image_url',
	'feed_image_width',
	'feed_language',
	'feed_link',
	'feed_title',
	'first_item_author_name',
	'first_item_category_label',
	'first_item_content',
	'first_item_contributor_name',
	'first_item_date',
	'first_item_description',
	'first_item_id',
	'first_item_latitude',
	'first_item_longitude',
	'first_item_permalink',
	'first_item_title',
);

$master = new Unit_Test2_Group('SimplePie Test Suite');

foreach ($tests as $test)
{
	$test_group = new SimplePie_Unit_Test2_Group(ucwords(str_replace('_', ' ', $test)));
	$test_group->load_folder($test);
	$master->add($test_group);
}

$test_group = new SimplePie_Unit_Test2_Group('Who knows a <title> from a hole in the ground?');
$test_group->load_folder('who_knows_a_title_from_a_hole_in_the_ground');
$master->add($test_group);

$test_group = new SimplePie_Unit_Test2_Group('iTunesRSS');
$test_group->load_folder('itunes_rss');
$master->add($test_group);

if (isset($_GET['remote']))
{
	$test_group = new SimplePie_Unit_Test2_Group('Atom autodiscovery test suite');
	$test_group->add(new diveintomark_Atom_Autodiscovery);
	$master->add($test_group);
}

$master->run();

$passed_percentage = floor($master->passes() / $master->total() * 100);
$failed_percentage = ceil($master->fails() / $master->total() * 100);

?>
	<h2 id="summary" class=<?php echo ($passed_percentage == 100) ? 'pass' : 'fail'; ?>><?php echo $passed_percentage; ?>% passed!</h2>
	<?php echo '<p>We ran ' . $master->total() . ' tests in ' . round($master->time(), 3) . ' seconds (' . round($master->time() / $master->total(), 3) . ' seconds per test) of which ' . $master->passes() . ' (' . $passed_percentage . '%) passed, and ' . $master->fails() . ' (' . $failed_percentage . '%) failed.</p>'; ?>
	
	<p><small>Powered by <a href="<?php echo SIMPLEPIE_URL; ?>"><?php echo SIMPLEPIE_NAME . ' ' . SIMPLEPIE_VERSION . ', Build ' . SIMPLEPIE_BUILD; ?></a>.  SimplePie is &copy; 2004&ndash;<?php echo date('Y'); ?>, Ryan Parman and Geoffrey Sneddon, and licensed under the <a href="http://www.opensource.org/licenses/bsd-license.php">BSD License</a>.</small></p>

</div>

</body>
</html>
