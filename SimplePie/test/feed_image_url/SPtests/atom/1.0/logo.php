<?php

class SimplePie_Feed_Image_URL_Test_Atom_10_Logo extends SimplePie_Feed_Image_URL_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<logo>http://example.com/</logo>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>