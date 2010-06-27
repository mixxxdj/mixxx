<?php

class SimplePie_Feed_Title_Test_Atom_10_Title extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<title>Feed Title</title>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Title';
	}
}

?>