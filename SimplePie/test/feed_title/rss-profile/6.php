<?php

class RSS_Profile_Title_6 extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<title>A&#x3C;B</title>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'A&lt;B';
	}
}

?>