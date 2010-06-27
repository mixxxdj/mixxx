<?php

class RSS_Profile_Title_2 extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<title>Bill &#x26; Ted\'s Excellent Adventure</title>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Bill &amp; Ted\'s Excellent Adventure';
	}
}

?>