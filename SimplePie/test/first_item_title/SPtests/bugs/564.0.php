<?php

class SimplePie_First_Item_Title_Test_Bug_564_Test_0 extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<entry>
		<title type="html"><![CDATA[<blink>A<blink>B</blink>C</blink>]]></title>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'ABC';
	}
}

?>