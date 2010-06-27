<?php

class SimplePie_First_Item_Title_Test_Bug_431_Test_0 extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<rss version="2.0">
	<channel>
		<item>
			<title>Item title</title>
			<image>
				<title>Image title</title>
			</image>
		</item>
	</channel>
</rss>';
	}
	
	function expected()
	{
		$this->expected = 'Item title';
	}
}

?>