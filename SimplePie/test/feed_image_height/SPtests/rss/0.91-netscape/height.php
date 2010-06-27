<?php

class SimplePie_Feed_Image_Height_Test_RSS_091_Netscape_Height extends SimplePie_Feed_Image_Height_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss SYSTEM "http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91">
	<channel>
		<image>
			<height>100</height>
		</image>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 100.0;
	}
}

?>