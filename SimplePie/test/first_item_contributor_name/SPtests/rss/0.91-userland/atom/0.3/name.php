<?php

class SimplePie_First_Item_Contributor_Name_Test_RSS_091_Userland_Atom_03_Name extends SimplePie_First_Item_Contributor_Name_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91" xmlns:a="http://purl.org/atom/ns#">
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