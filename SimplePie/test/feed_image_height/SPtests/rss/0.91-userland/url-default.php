<?php

class SimplePie_Feed_Image_Height_Test_RSS_091_Userland_URL_Default extends SimplePie_Feed_Image_Height_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91">
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