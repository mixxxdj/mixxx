<?php

class SimplePie_First_Item_Category_Label_Test_RSS_092_Atom_10_Category_Term extends SimplePie_First_Item_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92" xmlns:a="http://www.w3.org/2005/Atom">
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