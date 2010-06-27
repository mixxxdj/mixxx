<?php

class SimplePie_First_Item_Title_Test_Atom_03_Title extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#">
	<entry>
		<title>Item Title</title>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Item Title';
	}
}

?>