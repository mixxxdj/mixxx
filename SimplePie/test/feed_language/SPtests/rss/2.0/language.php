<?php

class SimplePie_Feed_Language_Test_RSS_20_Language extends SimplePie_Feed_Language_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
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