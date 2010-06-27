<?php

class SimplePie_Feed_Copyright_Test_RSS_092_DC_10_Rights extends SimplePie_Feed_Copyright_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92" xmlns:dc="http://purl.org/dc/elements/1.0/">
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