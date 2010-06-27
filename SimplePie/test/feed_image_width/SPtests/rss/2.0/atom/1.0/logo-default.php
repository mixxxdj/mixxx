<?php

class SimplePie_Feed_Image_Width_Test_RSS_20_Atom_10_Logo extends SimplePie_Feed_Image_Width_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0" xmlns:a="http://www.w3.org/2005/Atom">
	<channel>
		<a:logo>http://example.com/</a:logo>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = NULL;
	}
}

?>