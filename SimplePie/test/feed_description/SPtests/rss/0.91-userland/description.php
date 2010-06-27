<?php

class SimplePie_Feed_Description_Test_RSS_091_Userland_Description extends SimplePie_Feed_Description_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91">
	<channel>
		<description>Feed Description</description>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Description';
	}
}

?>