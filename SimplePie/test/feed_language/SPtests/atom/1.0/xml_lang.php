<?php

class SimplePie_Feed_Language_Test_Atom_10_xmllang extends SimplePie_Feed_Language_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom" xml:lang="en-GB">
	<title>Feed Title</title>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'en-GB';
	}
}

?>