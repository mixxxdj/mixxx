<?php

class SimplePie_Feed_Description_Test_Atom_10_Subtitle extends SimplePie_Feed_Description_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<subtitle>Feed Description</subtitle>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Description';
	}
}

?>