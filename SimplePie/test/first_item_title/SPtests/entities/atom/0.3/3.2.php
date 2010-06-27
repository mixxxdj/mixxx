<?php

class SimplePie_First_Item_Title_Test_Atom_03_Title_Text_3 extends SimplePie_First_Item_Title_Test
{
	function data()
	{
		$this->data = 
'<feed version="0.3" xmlns="http://purl.org/atom/ns#">
	<entry>
		<title type="text/plain">This <![CDATA[&amp;]]>amp; this</title>
	</entry>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'This &amp;amp;amp; this';
	}
}

?>