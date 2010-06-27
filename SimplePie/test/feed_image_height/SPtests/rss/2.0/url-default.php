<?php

class SimplePie_Feed_Image_Height_Test_RSS_20_URL_Default extends SimplePie_Feed_Image_Height_Test
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
		$this->expected = 31.0;
	}
}

?>