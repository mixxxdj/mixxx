<?php

class SimplePie_First_Item_Date_Test_RSS_20_Atom_03_Modified extends SimplePie_First_Item_Date_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0" xmlns:a="http://purl.org/atom/ns#">
	<channel>
		<item>
			<a:modified>2007-01-11T16:00:00Z</a:modified>
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