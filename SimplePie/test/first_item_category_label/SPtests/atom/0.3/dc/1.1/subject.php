<?php

class SimplePie_First_Item_Category_Label_Test_Atom_03_DC_11_Subject extends SimplePie_First_Item_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#" xmlns:dc="http://purl.org/dc/elements/1.1/">
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