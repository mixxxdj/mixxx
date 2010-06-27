<?php

class SimplePie_First_Item_Description_Test_Atom_03_Summary extends SimplePie_First_Item_Description_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#">
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