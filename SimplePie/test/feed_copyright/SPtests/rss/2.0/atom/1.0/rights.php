<?php

class SimplePie_Feed_Copyright_Test_RSS_20_Atom_10_Rights extends SimplePie_Feed_Copyright_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0" xmlns:a="http://www.w3.org/2005/Atom">
	<channel>
		<a:rights>Example Copyright Information</a:rights>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Example Copyright Information';
	}
}

?>