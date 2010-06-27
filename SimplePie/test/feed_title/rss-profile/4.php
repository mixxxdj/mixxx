<?php

class RSS_Profile_Title_4 extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<title>I &#x3C;3 Phil Ringnalda</title>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'I &lt;3 Phil Ringnalda';
	}
}

?>