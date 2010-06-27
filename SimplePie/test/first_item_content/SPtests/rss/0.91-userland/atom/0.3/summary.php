<?php

class SimplePie_First_Item_Content_Test_RSS_091_Userland_Atom_03_Summary extends SimplePie_First_Item_Content_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91" xmlns:a="http://purl.org/atom/ns#">
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