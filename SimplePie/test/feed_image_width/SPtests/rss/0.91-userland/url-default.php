<?php

class SimplePie_Feed_Image_Width_Test_RSS_091_Userland extends SimplePie_Feed_Image_Width_Test
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
		$this->expected = 88.0;
	}
}

?>