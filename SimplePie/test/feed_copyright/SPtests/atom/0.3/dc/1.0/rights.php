<?php

class SimplePie_Feed_Copyright_Test_Atom_03_DC_10 extends SimplePie_Feed_Copyright_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<dc:rights>Example Copyright Information</dc:rights>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Example Copyright Information';
	}
}

?>