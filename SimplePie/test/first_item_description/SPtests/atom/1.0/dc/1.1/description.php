<?php

class SimplePie_First_Item_Description_Test_Atom_10_DC_11_Description extends SimplePie_First_Item_Description_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom" xmlns:dc="http://purl.org/dc/elements/1.1/">
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