<?php

class SimplePie_Feed_Category_Label_Test_RSS_092_DC_11_Subject extends SimplePie_Feed_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92" xmlns:dc="http://purl.org/dc/elements/1.1/">
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