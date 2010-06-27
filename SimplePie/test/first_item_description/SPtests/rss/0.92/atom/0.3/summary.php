<?php

class SimplePie_First_Item_Description_Test_RSS_092_Atom_03_Summary extends SimplePie_First_Item_Description_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92" xmlns:a="http://purl.org/atom/ns#">
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