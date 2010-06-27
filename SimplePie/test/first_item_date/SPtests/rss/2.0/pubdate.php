<?php

class SimplePie_First_Item_Date_Test_RSS_20_pubDate extends SimplePie_First_Item_Date_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<item>
			<pubDate>2007-01-11T16:00:00Z</pubDate>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 1168531200;
	}
}

?>