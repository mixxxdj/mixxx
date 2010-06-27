<?php

class SimplePie_First_Item_Date_Test_RSS_091_Netscape_DC_10_Date extends SimplePie_First_Item_Date_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss SYSTEM "http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<channel>
		<item>
			<dc:date>2007-01-11T16:00:00Z</dc:date>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 1168531200;
	}
}

?>