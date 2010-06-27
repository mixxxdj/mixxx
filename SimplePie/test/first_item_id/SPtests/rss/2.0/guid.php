<?php

class SimplePie_First_Item_ID_Test_RSS_20_GUID extends SimplePie_First_Item_ID_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<item>
			<guid>http://example.com/</guid>
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