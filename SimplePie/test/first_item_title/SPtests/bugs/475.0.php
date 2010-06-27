<?php

class SimplePie_First_Item_Title_Test_Bug_475_Test_0 extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<entry>
		<source>
			<title>Source Title</title>
		</source>
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