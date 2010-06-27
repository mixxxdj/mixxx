<?php

class SimplePie_First_Item_Title_Test_Bug_629_Test_1 extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<entry>
		<title>Item title</title>
	</entry>
	<x:foo xmlns:x="urn:foo">
		<entry>
			<title>Extension title</title>
		</entry>
	</x:foo>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Item title';
	}
}

?>