<?php

class SimplePie_Feed_Image_Title_Test_RSS_091_Userland_Title extends SimplePie_Feed_Image_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91">
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