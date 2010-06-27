<?php

class SimplePie_Feed_Image_Title_Test_RSS_091_Netscape_DC_10_Title extends SimplePie_Feed_Image_Title_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss SYSTEM "http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<channel>
		<image>
			<dc:title>Image Title</dc:title>
		</image>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Image Title';
	}
}

?>