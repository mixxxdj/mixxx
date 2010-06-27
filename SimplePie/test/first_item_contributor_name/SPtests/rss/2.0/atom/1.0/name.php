<?php

class SimplePie_First_Item_Contributor_Name_Test_RSS_20_Atom_10_Name extends SimplePie_First_Item_Contributor_Name_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0" xmlns:a="http://www.w3.org/2005/Atom">
	<channel>
		<item>
			<a:contributor>
				<a:name>Item Contributor</a:name>
			</a:contributor>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Item Contributor';
	}
}

?>