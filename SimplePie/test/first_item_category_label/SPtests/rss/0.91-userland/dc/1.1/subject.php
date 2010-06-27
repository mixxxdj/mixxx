<?php

class SimplePie_First_Item_Category_Label_Test_RSS_091_Userland_DC_11_Subject extends SimplePie_First_Item_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91" xmlns:dc="http://purl.org/dc/elements/1.1/">
	<channel>
		<item>
			<dc:subject>Item Category</dc:subject>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Item Category';
	}
}

?>