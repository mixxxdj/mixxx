<?php

class SimplePie_First_Item_Title_Test_RSS_20_Title extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<item>
			<title>Item Title</title>
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