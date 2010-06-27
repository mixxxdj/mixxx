<?php

class SimplePie_Feed_Image_Link_Test_RSS_20_Link extends SimplePie_Feed_Image_Link_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<image>
			<link>http://example.com/</link>
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