<?php

class SimplePie_Feed_Image_Height_Test_RSS_20_Height extends SimplePie_Feed_Image_Height_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
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