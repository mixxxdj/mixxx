<?php

class SimplePie_First_Item_Contributor_Name_Test_Atom_03_Name extends SimplePie_First_Item_Contributor_Name_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#">
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