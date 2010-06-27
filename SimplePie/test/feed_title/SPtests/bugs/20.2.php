<?php

class SimplePie_Feed_Title_Test_Bug_20_Test_2 extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom" xmlns:h="http://www.w3.org/1999/xhtml">
	<title type="xhtml"><h:div>Non-default namespace</h:div></title>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Non-default namespace';
	}
}

?>