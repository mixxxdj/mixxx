<?php

class SimplePie_First_Item_Description_Test_RSS_091_Userland_Atom_03_Content extends SimplePie_First_Item_Description_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91" xmlns:a="http://purl.org/atom/ns#">
	<channel>
		<item>
			<a:content>Item Description</a:content>
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