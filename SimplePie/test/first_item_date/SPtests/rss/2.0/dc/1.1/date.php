<?php

class SimplePie_First_Item_Date_Test_RSS_20_DC_11_Date extends SimplePie_First_Item_Date_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0" xmlns:dc="http://purl.org/dc/elements/1.1/">
	<channel>
		<item>
			<dc:date>2007-01-11T16:00:00Z</dc:date>
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