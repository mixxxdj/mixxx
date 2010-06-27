<?php

class SimplePie_Feed_Link_Test_Atom_03_Link extends SimplePie_Feed_Link_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#">
	<link href="http://example.com/"/>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>