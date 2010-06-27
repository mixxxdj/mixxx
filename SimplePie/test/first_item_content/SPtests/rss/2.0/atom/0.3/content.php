<?php

class SimplePie_First_Item_Content_Test_RSS_20_Atom_03_Content extends SimplePie_First_Item_Content_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0" xmlns:a="http://purl.org/atom/ns#">
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