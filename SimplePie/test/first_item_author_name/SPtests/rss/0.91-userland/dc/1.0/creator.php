<?php

class SimplePie_First_Item_Author_Name_Test_RSS_091_Userland_DC_10_Creator extends SimplePie_First_Item_Author_Name_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<channel>
		<item>
			<dc:creator>Item Author</dc:creator>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Item Author';
	}
}

?>