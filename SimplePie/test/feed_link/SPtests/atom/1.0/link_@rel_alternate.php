<?php

class SimplePie_Feed_Link_Test_Atom_10_Link_Relative_IRI extends SimplePie_Feed_Link_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<link href="http://example.com/" rel="alternate"/>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>