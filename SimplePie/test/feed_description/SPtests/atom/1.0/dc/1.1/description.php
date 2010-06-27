<?php

class SimplePie_Feed_Description_Test_Atom_10_DC_11_Description extends SimplePie_Feed_Description_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom" xmlns:dc="http://purl.org/dc/elements/1.1/">
	<dc:description>Feed Description</dc:description>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Description';
	}
}

?>