<?php

class SimplePie_Feed_Description_Test_RSS_091_Userland_Atom_10_Subtitle extends SimplePie_Feed_Description_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91" xmlns:a="http://www.w3.org/2005/Atom">
	<channel>
		<a:subtitle>Feed Description</a:subtitle>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Description';
	}
}

?>