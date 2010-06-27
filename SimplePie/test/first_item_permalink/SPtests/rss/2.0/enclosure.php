<?php

class SimplePie_First_Item_Permalink_Test_RSS_20_Enclosure extends SimplePie_First_Item_Permalink_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<item>
			<enclosure url="http://example.com/" length="1" type="text/html"/>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>