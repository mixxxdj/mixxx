<?php

class SimplePie_Feed_Link_Test_RSS_091_Netscape_Atom_03_Link extends SimplePie_Feed_Link_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss SYSTEM "http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91" xmlns:a="http://purl.org/atom/ns#">
	<channel>
		<a:link href="http://example.com/"/>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>