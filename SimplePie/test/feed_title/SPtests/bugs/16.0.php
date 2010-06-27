<?php

class SimplePie_Feed_Title_Test_Bug_16_Test_0 extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<!DOCTYPE rss PUBLIC "-//Netscape Communications//DTD RSS 0.91//EN" 
"http://my.netscape.com/publish/formats/rss-0.91.dtd">
<rss version="0.91">
	<channel>
		<title>Feed with DOCTYPE</title>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Feed with DOCTYPE';
	}
}

?>