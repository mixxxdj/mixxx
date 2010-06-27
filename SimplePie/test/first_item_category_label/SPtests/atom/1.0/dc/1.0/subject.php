<?php

class SimplePie_First_Item_Category_Label_Test_Atom_10_DC_10_Subject extends SimplePie_First_Item_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<entry>
		<dc:subject>Item Category</dc:subject>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Item Category';
	}
}

?>