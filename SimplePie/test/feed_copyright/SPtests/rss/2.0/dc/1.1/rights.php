<?php

class SimplePie_Feed_Copyright_Test_RSS_20_DC_11_Rights extends SimplePie_Feed_Copyright_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0" xmlns:dc="http://purl.org/dc/elements/1.1/">
	<channel>
		<dc:rights>Example Copyright Information</dc:rights>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Example Copyright Information';
	}
}

?>