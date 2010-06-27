<?php

class SimplePie_First_Item_Description_Test_RSS_091_Netscape_Description extends SimplePie_First_Item_Description_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss SYSTEM "http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91">
	<channel>
		<item>
			<description>Item Description</description>
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