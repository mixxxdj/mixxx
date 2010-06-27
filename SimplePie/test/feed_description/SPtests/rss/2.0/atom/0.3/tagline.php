<?php

class SimplePie_Feed_Description_Test_RSS_20_Atom_03_Tagline extends SimplePie_Feed_Description_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0" xmlns:a="http://purl.org/atom/ns#">
	<channel>
		<a:tagline>Feed Description</a:tagline>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Description';
	}
}

?>