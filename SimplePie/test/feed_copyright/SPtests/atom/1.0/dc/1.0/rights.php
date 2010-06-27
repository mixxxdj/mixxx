<?php

class SimplePie_Feed_Copyright_Test_Atom_10_DC_10 extends SimplePie_Feed_Copyright_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<dc:rights>Example Copyright Information</dc:rights>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Example Copyright Information';
	}
}

?>