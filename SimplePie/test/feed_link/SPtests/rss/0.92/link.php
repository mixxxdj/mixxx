<?php

class SimplePie_Feed_Link_Test_RSS_092_Link extends SimplePie_Feed_Link_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92">
	<channel>
		<link>http://example.com/</link>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>