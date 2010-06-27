<?php

class SimplePie_First_Item_ID_Test_Atom_10_DC_11_Identifier extends SimplePie_First_Item_ID_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom" xmlns:dc="http://purl.org/dc/elements/1.1/">
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