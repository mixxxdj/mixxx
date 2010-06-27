<?php

class SimplePie_Feed_Title_Test_Bug_272_Test_0 extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<title>Ampersand: <![CDATA[&]]></title>
</feed>';
	}
	
	function expected()
	{
		$this->expected = 'Ampersand: &amp;';
	}
}

?>