<?php

class SimplePie_Feed_Category_Label_Test_Atom_03_DC_10_Subject extends SimplePie_Feed_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#" xmlns:dc="http://purl.org/dc/elements/1.0/">
	<dc:subject>Feed Category</dc:subject>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Category';
	}
}

?>