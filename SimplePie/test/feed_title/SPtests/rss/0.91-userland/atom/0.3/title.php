<?php

class SimplePie_Feed_Title_Test_RSS_091_Userland_Atom_03_Title extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91" xmlns:a="http://purl.org/atom/ns#">
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