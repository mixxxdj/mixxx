<?php

class SimplePie_First_Item_Title_Test_Atom_03_Title_XHTML_2 extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#">
	<entry>
		<title type="application/xhtml+xml" mode="xml"><div xmlns="http://www.w3.org/1999/xhtml"><![CDATA[This &amp;amp; this]]></div></title>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'This &amp;amp;amp; this';
	}
}

?>