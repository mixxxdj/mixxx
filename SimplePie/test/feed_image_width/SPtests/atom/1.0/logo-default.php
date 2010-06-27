<?php

class SimplePie_Feed_Image_Width_Test_Atom_10_Logo_Default extends SimplePie_Feed_Image_Width_Test
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
		$this->expected = NULL;
	}
}

?>
