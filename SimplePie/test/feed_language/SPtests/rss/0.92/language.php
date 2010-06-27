<?php

class SimplePie_Feed_Language_Test_RSS_092_Language extends SimplePie_Feed_Language_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92">
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