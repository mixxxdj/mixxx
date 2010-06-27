<?php

class SimplePie_First_Item_Date_Test_RSS_092_Atom_10_Updated extends SimplePie_First_Item_Date_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92" xmlns:a="http://www.w3.org/2005/Atom">
	<channel>
		<item>
			<a:updated>2007-01-11T16:00:00Z</a:updated>
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