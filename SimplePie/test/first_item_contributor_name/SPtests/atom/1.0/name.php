<?php

class SimplePie_First_Item_Contributor_Name_Test_Atom_10_Name extends SimplePie_First_Item_Contributor_Name_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<entry>
		<contributor>
			<name>Item Contributor</name>
		</contributor>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Item Contributor';
	}
}

?>