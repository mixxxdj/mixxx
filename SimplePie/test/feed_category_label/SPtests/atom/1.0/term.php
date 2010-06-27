<?php

class SimplePie_Feed_Category_Label_Test_Atom_10_Category_Term extends SimplePie_Feed_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<category term="Feed Category"/>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Feed Category';
	}
}

?>