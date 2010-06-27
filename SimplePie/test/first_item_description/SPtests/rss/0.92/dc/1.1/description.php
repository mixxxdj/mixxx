<?php

class SimplePie_First_Item_Description_Test_RSS_092_DC_11_Description extends SimplePie_First_Item_Description_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92" xmlns:dc="http://purl.org/dc/elements/1.1/">
	<channel>
		<item>
			<dc:description>Item Description</dc:description>
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