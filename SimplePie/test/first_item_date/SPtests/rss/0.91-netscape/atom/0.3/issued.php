<?php

class SimplePie_First_Item_Date_Test_RSS_091_Netscape_Atom_03_Issued extends SimplePie_First_Item_Date_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss SYSTEM "http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91" xmlns:a="http://purl.org/atom/ns#">
	<channel>
		<item>
			<a:issued>2007-01-11T16:00:00Z</a:issued>
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