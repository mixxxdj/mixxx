<?php

class SimplePie_Feed_Title_Test_RSS_091_Netscape_Title extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss SYSTEM "http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91">
	<channel>
		<title>Feed Title</title>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Title';
	}
}

?>