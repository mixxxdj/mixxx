<?php

class SimplePie_Feed_Image_URL_Test_RSS_20_URL extends SimplePie_Feed_Image_URL_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<image>
			<url>http://example.com/</url>
		</image>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>