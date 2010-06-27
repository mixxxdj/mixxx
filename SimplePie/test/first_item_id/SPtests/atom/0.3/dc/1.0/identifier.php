<?php

class SimplePie_First_Item_ID_Test_Atom_03_DC_10_Identifier extends SimplePie_First_Item_ID_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<entry>
		<dc:identifier>http://example.com/</dc:identifier>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>