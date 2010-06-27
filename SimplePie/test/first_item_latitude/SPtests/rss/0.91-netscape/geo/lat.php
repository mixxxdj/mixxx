<?php

class SimplePie_First_Item_Latitude_Test_RSS_091_Netscape_Geo_Lat extends SimplePie_First_Item_Latitude_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss SYSTEM "http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91" xmlns:geo="http://www.w3.org/2003/01/geo/wgs84_pos#">
	<channel>
		<item>
			<geo:lat>55.701</geo:lat>
			<geo:long>12.552</geo:long>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 55.701;
	}
}

?>