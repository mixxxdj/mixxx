<?php

class SimplePie_First_Item_Author_Name_Test_RSS_20_Atom_03_Name extends SimplePie_First_Item_Author_Name_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0" xmlns:a="http://purl.org/atom/ns#">
	<channel>
		<item>
			<a:author>
				<a:name>Item Author</a:name>
			</a:author>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Item Author';
	}
}

?>