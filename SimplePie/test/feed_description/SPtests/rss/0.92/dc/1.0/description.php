<?php

class SimplePie_Feed_Description_Test_RSS_092_DC_10_Description extends SimplePie_Feed_Description_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<channel>
		<dc:description>Feed Description</dc:description>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Description';
	}
}

?>