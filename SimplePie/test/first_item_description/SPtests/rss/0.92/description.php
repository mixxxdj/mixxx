<?php

class SimplePie_First_Item_Description_Test_RSS_092_Description extends SimplePie_First_Item_Description_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92">
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