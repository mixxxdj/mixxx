<?php

class SimplePie_First_Item_Category_Label_Test_RSS_20_Atom_10_Category_Term extends SimplePie_First_Item_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0" xmlns:a="http://www.w3.org/2005/Atom">
	<channel>
		<item>
			<a:category term="Item Category"/>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Item Category';
	}
}

?>