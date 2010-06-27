<?php

class SimplePie_Feed_Image_Width_Test_RSS_092_Width extends SimplePie_Feed_Image_Width_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92">
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