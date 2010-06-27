<?php

class SimplePie_Feed_Image_Width_Test_Atom_10_Icon_Default extends SimplePie_Feed_Image_Width_Test
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
		$this->expected = NULL;
	}
}

?>
