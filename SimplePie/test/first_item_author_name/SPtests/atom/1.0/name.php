<?php

class SimplePie_First_Item_Author_Name_Test_Atom_10_Name extends SimplePie_First_Item_Author_Name_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<entry>
		<author>
			<name>Item Author</name>
		</author>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Item Author';
	}
}

?>