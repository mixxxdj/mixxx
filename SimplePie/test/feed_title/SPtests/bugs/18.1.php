<?php

class SimplePie_Feed_Title_Test_Bug_18_Test_1 extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<image>
			<title>Image title</title>
		</image>
		<title>Channel title</title>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Channel title';
	}
}

?>