<?php

class SimplePie_First_Item_Title_Test_Bug_475_Test_2 extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<entry>
		<title>Item Title</title>
		<x:foo xmlns:x="urn:foo">
			<title>Extension Title</title>
		</x:foo>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Item Title';
	}
}

?>