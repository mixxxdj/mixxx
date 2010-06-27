<?php

class SimplePie_Feed_Category_Label_Test_Atom_10_DC_10_Subject extends SimplePie_Feed_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<dc:subject>Feed Category</dc:subject>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Category';
	}
}

?>