<?php

class SimplePie_First_Item_Title_Test_Bug_174_Test_0 extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<?xml version="1.0" encoding = "UTF-8" ?>
<feed xmlns="http://www.w3.org/2005/Atom">
	<entry>
		<title>Item Title</title>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Item Title';
	}
}

?>