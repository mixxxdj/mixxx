<?php

class SimplePie_First_Item_Content_Test_RSS_092_Atom_10_Content extends SimplePie_First_Item_Content_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92" xmlns:a="http://www.w3.org/2005/Atom">
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