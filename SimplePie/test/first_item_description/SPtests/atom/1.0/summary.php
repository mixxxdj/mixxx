<?php

class SimplePie_First_Item_Description_Test_Atom_10_Summary extends SimplePie_First_Item_Description_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<entry>
		<summary>Item Description</summary>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Item Description';
	}
}

?>