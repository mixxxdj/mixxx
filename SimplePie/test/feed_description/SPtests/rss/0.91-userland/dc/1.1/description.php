<?php

class SimplePie_Feed_Description_Test_RSS_091_Userland_DC_11_Description extends SimplePie_Feed_Description_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91" xmlns:dc="http://purl.org/dc/elements/1.1/">
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