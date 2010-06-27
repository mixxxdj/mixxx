<?php

class SimplePie_Feed_Image_Height_Test_RSS_092_Atom_10_Icon_Default extends SimplePie_Feed_Image_Height_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92" xmlns:a="http://www.w3.org/2005/Atom">
	<channel>
		<a:icon>http://example.com/</a:icon>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = NULL;
	}
}

?>