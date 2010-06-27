<?php

class SimplePie_Feed_Image_Title_Test_RSS_20_Title extends SimplePie_Feed_Image_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<image>
			<title>Image Title</title>
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