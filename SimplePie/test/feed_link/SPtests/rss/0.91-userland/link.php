<?php

class SimplePie_Feed_Link_Test_RSS_091_Userland_Link extends SimplePie_Feed_Link_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91">
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