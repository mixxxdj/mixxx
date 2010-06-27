<?php

class SimplePie_First_Item_Contributor_Name_Test_RSS_091_Netscape_Atom_10_Name extends SimplePie_First_Item_Contributor_Name_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss SYSTEM "http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91" xmlns:a="http://www.w3.org/2005/Atom">
	<channel>
		<item>
			<a:contributor>
				<a:name>Item Contributor</a:name>
			</a:contributor>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Item Contributor';
	}
}

?>