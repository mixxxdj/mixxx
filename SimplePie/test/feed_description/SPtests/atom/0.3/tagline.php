<?php

class SimplePie_Feed_Description_Test_Atom_03_Tagline extends SimplePie_Feed_Description_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#">
	<tagline>Feed Description</tagline>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Description';
	}
}

?>