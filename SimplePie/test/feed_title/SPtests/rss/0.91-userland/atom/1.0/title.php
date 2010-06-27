<?php

class SimplePie_Feed_Title_Test_RSS_091_Userland_Atom_10_Title extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91" xmlns:a="http://www.w3.org/2005/Atom">
	<channel>
		<a:title>Feed Title</a:title>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Title';
	}
}

?>