<?php

class SimplePie_Feed_Category_Label_Test_RSS_20_Category extends SimplePie_Feed_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<category>Feed Category</category>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Category';
	}
}

?>