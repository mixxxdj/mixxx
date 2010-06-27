<?php

class SimplePie_Feed_Title_Test_Atom_03_DC_11_Title extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#" xmlns:dc="http://purl.org/dc/elements/1.1/">
	<dc:title>Feed Title</dc:title>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Title';
	}
}

?>