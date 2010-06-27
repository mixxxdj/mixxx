<?php

class SimplePie_First_Item_Content_Test_Atom_03_DC_10_Description extends SimplePie_First_Item_Content_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<entry>
		<dc:description>Item Description</dc:description>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Item Description';
	}
}

?>