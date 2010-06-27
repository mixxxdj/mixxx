<?php

class SimplePie_First_Item_ID_Test_RSS_092_DC_11_Identifier extends SimplePie_First_Item_ID_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92" xmlns:dc="http://purl.org/dc/elements/1.1/">
	<channel>
		<item>
			<dc:identifier>http://example.com/</dc:identifier>
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