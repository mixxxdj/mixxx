<?php

class SimplePie_First_Item_Author_Name_Test_Atom_03_Name extends SimplePie_First_Item_Author_Name_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#">
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