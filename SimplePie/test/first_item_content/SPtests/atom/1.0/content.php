<?php

class SimplePie_First_Item_Content_Test_Atom_10_Content extends SimplePie_First_Item_Content_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
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