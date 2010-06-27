<?php

class SimplePie_First_Item_Title_Test_Bug_629_Test_0 extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<x:foo xmlns:x="urn:foo">
		<entry>
			<title>Extension title</title>
		</entry>
	</x:foo>
	<entry>
		<title>Item title</title>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Item title';
	}
}

?>