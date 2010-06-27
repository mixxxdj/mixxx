<?php

class RSS_Profile_Title_3 extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<title>The &#x26;amp; entity</title>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'The &amp;amp; entity';
	}
}

?>