<?php

class SimplePie_Feed_Copyright_Test_RSS_091_Netscape_DC_11_Rights extends SimplePie_Feed_Copyright_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss SYSTEM "http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91" xmlns:dc="http://purl.org/dc/elements/1.1/">
	<channel>
		<dc:rights>Example Copyright Information</dc:rights>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Example Copyright Information';
	}
}

?>