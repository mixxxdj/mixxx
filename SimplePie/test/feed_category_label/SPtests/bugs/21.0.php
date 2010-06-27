<?php

class SimplePie_Feed_Category_Label_Test_Bug_21_Test_0 extends SimplePie_Feed_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<category term="Example category"/>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Example category';
	}
}

?>