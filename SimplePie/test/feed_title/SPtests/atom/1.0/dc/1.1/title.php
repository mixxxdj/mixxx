<?php

class SimplePie_Feed_Title_Test_Atom_10_DC_11_Title extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom" xmlns:dc="http://purl.org/dc/elements/1.1/">
	<dc:title>Feed Title</dc:title>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Title';
	}
}

?>