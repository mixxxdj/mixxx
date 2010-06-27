<?php

class SimplePie_First_Item_Content_Test_RSS_20_Description extends SimplePie_First_Item_Content_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
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