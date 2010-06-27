<?php

class SimplePie_Feed_Title_Test_Bug_174_Test_0 extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<?xml version = "1.0" encoding = "UTF-8" ?>
<feed xmlns="http://www.w3.org/2005/Atom">
	<title>Spaces in prolog</title>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Spaces in prolog';
	}
}

?>