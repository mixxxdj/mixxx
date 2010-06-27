<?php

class SimplePie_Feed_Title_Test_Bug_18_Test_0 extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<title>Channel title</title>
		<image>
			<title>Image title</title>
		</image>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Channel title';
	}
}

?>