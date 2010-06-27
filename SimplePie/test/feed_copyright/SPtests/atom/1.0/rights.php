<?php

class SimplePie_Feed_Copyright_Test_Atom_10_Rights extends SimplePie_Feed_Copyright_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<rights>Example Copyright Information</rights>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Example Copyright Information';
	}
}

?>