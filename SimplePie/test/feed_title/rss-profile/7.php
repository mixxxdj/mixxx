<?php

class RSS_Profile_Title_7 extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<title>Nice &#x3C;gorilla&#x3E; what\'s he weigh?</title>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Nice &lt;gorilla&gt; what\'s he weigh?';
	}
}

?>