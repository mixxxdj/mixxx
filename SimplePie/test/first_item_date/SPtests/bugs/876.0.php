<?php

class SimplePie_First_Item_Date_Test_Bug_876_Test_0 extends SimplePie_First_Item_Date_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<item>
			<pubDate></pubDate>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = null;
	}
}

?>