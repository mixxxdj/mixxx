<?php

class SimplePie_Feed_Title_Test_RSS_091_Userland_DC_10_Title extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91" xmlns:dc="http://purl.org/dc/elements/1.0/">
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