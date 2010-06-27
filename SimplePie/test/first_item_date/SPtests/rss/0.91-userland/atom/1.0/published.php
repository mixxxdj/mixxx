<?php

class SimplePie_First_Item_Date_Test_RSS_091_Userland_Atom_10_Published extends SimplePie_First_Item_Date_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91" xmlns:a="http://www.w3.org/2005/Atom">
	<channel>
		<item>
			<a:published>2007-01-11T16:00:00Z</a:published>
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