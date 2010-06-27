<?php

class SimplePie_Feed_Title_Test_RSS_092_DC_11_Title extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92" xmlns:dc="http://purl.org/dc/elements/1.1/">
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