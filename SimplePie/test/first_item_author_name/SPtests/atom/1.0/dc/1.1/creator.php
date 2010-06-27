<?php

class SimplePie_First_Item_Author_Name_Test_Atom_10_DC_11_Creator extends SimplePie_First_Item_Author_Name_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom" xmlns:dc="http://purl.org/dc/elements/1.1/">
	<entry>
		<dc:creator>Item Author</dc:creator>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Item Author';
	}
}

?>