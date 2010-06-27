<?php

class SimplePie_First_Item_Title_Test_RSS_20_DC_10_Title extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<channel>
		<item>
			<dc:title>Item Title</dc:title>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Item Title';
	}
}

?>