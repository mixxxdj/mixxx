<?php

class SimplePie_Feed_Language_Test_Atom_10_DC_10_Language extends SimplePie_Feed_Language_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<dc:language>en-GB</dc:language>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'en-GB';
	}
}

?>