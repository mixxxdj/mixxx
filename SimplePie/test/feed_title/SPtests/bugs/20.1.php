<?php

class SimplePie_Feed_Title_Test_Bug_20_Test_1 extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<a:feed xmlns:a="http://www.w3.org/2005/Atom" xmlns="http://www.w3.org/1999/xhtml">
	<a:title type="xhtml"><div>Non-default namespace</div></a:title>
</a:feed>';
	}
	
	function expected()
	{
		$this->expected = 'Non-default namespace';
	}
}

?>