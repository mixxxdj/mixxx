<?php

class SimplePie_First_Item_Title_Test_Bug_270_Test_0 extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<entry>
		<content type="xhtml"><div xmlns="http://www.w3.org/1999/xhtml"><![CDATA[<title>]]></div></content>
		<title>The title</title>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'The title';
	}
}

?>