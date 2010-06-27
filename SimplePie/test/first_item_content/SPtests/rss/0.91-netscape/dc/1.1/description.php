<?php

class SimplePie_First_Item_Content_Test_RSS_091_Netscape_DC_11_Description extends SimplePie_First_Item_Content_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss SYSTEM "http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91" xmlns:dc="http://purl.org/dc/elements/1.1/">
	<channel>
		<item>
			<dc:description>Item Description</dc:description>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Item Description';
	}
}

?>