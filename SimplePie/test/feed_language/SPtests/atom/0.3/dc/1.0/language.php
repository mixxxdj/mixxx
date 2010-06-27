<?php

class SimplePie_Feed_Language_Test_Atom_03_DC_10_Language extends SimplePie_Feed_Language_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<dc:language>en-GB</dc:language>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'en-GB';
	}
}

?>