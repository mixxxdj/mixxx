<?php

class SimplePie_First_Item_Title_Test_Atom_10_Title_XHTML_3 extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<entry>
		<title type="xhtml"><div xmlns="http://www.w3.org/1999/xhtml">This <![CDATA[&amp;]]>amp; this</div></title>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'This &amp;amp;amp; this';
	}
}

?>