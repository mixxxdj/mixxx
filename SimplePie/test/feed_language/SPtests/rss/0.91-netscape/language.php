<?php

class SimplePie_Feed_Language_Test_RSS_091_Netscape_Language extends SimplePie_Feed_Language_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss SYSTEM "http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91">
	<channel>
		<language>en-GB</language>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'en-GB';
	}
}

?>