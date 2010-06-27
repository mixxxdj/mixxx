<?php

class SimplePie_First_Item_Author_Name_Test_RSS_20_Author extends SimplePie_First_Item_Author_Name_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<item>
			<author>example@example.com (Item Author)</author>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = NULL;
	}
}

?>