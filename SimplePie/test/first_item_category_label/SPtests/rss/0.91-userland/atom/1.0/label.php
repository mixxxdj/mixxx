<?php

class SimplePie_First_Item_Category_Label_Test_RSS_091_Userland_Atom_10_Category_Label extends SimplePie_First_Item_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.91" xmlns:a="http://www.w3.org/2005/Atom">
	<channel>
		<item>
			<a:category label="Item Category"/>
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