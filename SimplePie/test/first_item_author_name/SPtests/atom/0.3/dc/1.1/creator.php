<?php

class SimplePie_First_Item_Author_Name_Test_Atom_03_DC_11_Creator extends SimplePie_First_Item_Author_Name_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#" xmlns:dc="http://purl.org/dc/elements/1.1/">
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