<?php

class RSS_Profile_Title_1 extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<title>AT&#x26;T</title>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'AT&amp;T';
	}
}

?>