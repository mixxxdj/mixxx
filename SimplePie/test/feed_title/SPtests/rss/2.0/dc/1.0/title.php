<?php

class SimplePie_Feed_Title_Test_RSS_20_DC_10_Title extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<channel>
		<dc:title>Feed Title</dc:title>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Title';
	}
}

?>