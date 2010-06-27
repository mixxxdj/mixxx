<?php

class SimplePie_Feed_Description_Test_RSS_092_Description extends SimplePie_Feed_Description_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92">
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