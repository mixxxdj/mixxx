<?php

class SimplePie_Feed_Category_Label_Test_RSS_091_Netscape_DC_10_Subject extends SimplePie_Feed_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss SYSTEM "http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<channel>
		<dc:subject>Feed Category</dc:subject>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Category';
	}
}

?>