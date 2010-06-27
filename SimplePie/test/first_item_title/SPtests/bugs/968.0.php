<?php

class SimplePie_First_Item_Title_Test_Bug_968_Test_0 extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = "\xFF\xFE" . chunk_split(
'<?xml version="1.0" encoding="UTF-16LE"?>
<feed xmlns="http://www.w3.org/2005/Atom">
	<entry>
		<title>Item Title</title>
	</entry>
</feed>', 1, "\x00");
	}
	
	function expected()
	{
		$this->expected = 'Item Title';
	}
}

?>