<?php

class SimplePie_Feed_Image_URL_Test_Atom_10_Icon extends SimplePie_Feed_Image_URL_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<icon>http://example.com/</icon>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'http://example.com/';
	}
}

?>