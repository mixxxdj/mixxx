<?php

class SimplePie_First_Item_Title_Test_Atom_03_DC_10_Title extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<entry>
		<dc:title>Item Title</dc:title>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Item Title';
	}
}

?>