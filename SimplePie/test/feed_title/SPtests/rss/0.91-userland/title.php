<?php

class SimplePie_Feed_Title_Test_RSS_091_Userland_Title extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91">
	<channel>
		<title>Feed Title</title>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Title';
	}
}

?>