<?php

class SimplePie_First_Item_Category_Label_Test_Bug_21_Test_0 extends SimplePie_First_Item_Category_Label_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<entry>
		<category term="Example category"/>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Example category';
	}
}

?>