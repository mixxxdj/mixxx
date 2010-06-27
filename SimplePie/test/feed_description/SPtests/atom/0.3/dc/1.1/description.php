<?php

class SimplePie_Feed_Description_Test_Atom_03_DC_11_Description extends SimplePie_Feed_Description_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#" xmlns:dc="http://purl.org/dc/elements/1.1/">
	<dc:description>Feed Description</dc:description>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Description';
	}
}

?>