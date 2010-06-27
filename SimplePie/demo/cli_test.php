#!/usr/bin/php
<?php
include_once('../simplepie.inc');

// Parse it
$feed = new SimplePie();
if (isset($argv[1]) && $argv[1] !== '')
{
	$feed->set_feed_url($argv[1]);
	$feed->enable_cache(false);
	$feed->init();
}

$items = $feed->get_items();

foreach ($items as $item)
{
	echo $item->get_title() . "\n";
}

var_dump($feed->get_item_quantity());

?>