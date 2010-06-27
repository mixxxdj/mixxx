<?php

class SimplePie_First_Item_Description_Test_RSS_091_Netscape_Atom_10_Summary extends SimplePie_First_Item_Description_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss SYSTEM "http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91" xmlns:a="http://www.w3.org/2005/Atom">
	<channel>
		<item>
			<a:summary>Item Description</a:summary>
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