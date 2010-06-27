<?php

class SimplePie_Feed_Link_Test_RSS_20_Atom_10_Link extends SimplePie_Feed_Link_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0" xmlns:a="http://www.w3.org/2005/Atom">
	<channel>
		<a:link href="http://example.com/"/>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>