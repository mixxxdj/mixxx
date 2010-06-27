<?php

class SimplePie_First_Item_Content_Test_Atom_03_Content extends SimplePie_First_Item_Content_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#">
	<entry>
		<content>Item Description</content>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Item Description';
	}
}

?>