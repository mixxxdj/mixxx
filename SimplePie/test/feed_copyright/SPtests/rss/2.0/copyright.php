<?php

class SimplePie_Feed_Copyright_Test_RSS_20_Copyright extends SimplePie_Feed_Copyright_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<copyright>Example Copyright Information</copyright>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Example Copyright Information';
	}
}

?>