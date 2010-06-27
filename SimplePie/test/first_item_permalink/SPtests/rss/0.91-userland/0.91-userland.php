<?php

class SimplePie_First_Item_Permalink_Test_RSS_091_Userland_Link extends SimplePie_First_Item_Permalink_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91">
	<channel>
		<item>
			<link>http://example.com/</link>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>