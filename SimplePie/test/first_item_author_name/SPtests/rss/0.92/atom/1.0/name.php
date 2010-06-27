<?php

class SimplePie_First_Item_Author_Name_Test_RSS_092_Atom_10_Name extends SimplePie_First_Item_Author_Name_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92" xmlns:a="http://www.w3.org/2005/Atom">
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