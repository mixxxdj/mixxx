<?php

class SimplePie_First_Item_Category_Label_Test_RSS_092_Category extends SimplePie_First_Item_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<rss version="0.92">
	<channel>
		<item>
			<category>Item Category</category>
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