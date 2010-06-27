<?php

class SimplePie_Feed_Link_Test_Atom_03_Link_Alternate extends SimplePie_Feed_Link_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#">
	<link href="http://example.com/" rel="alternate"/>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>