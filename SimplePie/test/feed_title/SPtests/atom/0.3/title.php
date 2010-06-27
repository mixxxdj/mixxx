<?php

class SimplePie_Feed_Title_Test_Atom_03_Title extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#">
	<title>Feed Title</title>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Title';
	}
}

?>