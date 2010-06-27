<?php

class SimplePie_Feed_Description_Test_RSS_20_Description extends SimplePie_Feed_Description_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
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