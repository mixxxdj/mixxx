<?php

class SimplePie_Feed_Title_Test_RSS_092_Title extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92">
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