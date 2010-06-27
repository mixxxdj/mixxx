<?php

class SimplePie_First_Item_ID_Test_RSS_091_Netscape_DC_10_Identifier extends SimplePie_First_Item_ID_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss SYSTEM "http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<channel>
		<item>
			<dc:identifier>http://example.com/</dc:identifier>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>