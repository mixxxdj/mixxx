<?php

class SimplePie_First_Item_Category_Label_Test_Atom_10_Category_Term extends SimplePie_First_Item_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<entry>
		<category term="Item Category"/>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Item Category';
	}
}

?>