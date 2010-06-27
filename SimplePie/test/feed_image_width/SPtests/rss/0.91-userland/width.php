<?php

class SimplePie_Feed_Image_Width_Test_RSS_091_Userland_Width extends SimplePie_Feed_Image_Width_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91">
	<channel>
		<image>
			<width>100</width>
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