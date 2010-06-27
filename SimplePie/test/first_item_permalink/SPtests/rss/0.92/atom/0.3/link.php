<?php

class SimplePie_First_Item_Permalink_Test_RSS_092_Atom_03_Link extends SimplePie_First_Item_Permalink_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92" xmlns:a="http://purl.org/atom/ns#">
	<channel>
		<item>
			<a:link href="http://example.com/"/>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>