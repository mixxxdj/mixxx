<?php

class SimplePie_Feed_Title_Test_Bug_272_Test_1 extends SimplePie_Feed_Title_Test
{
	function data()
	{
		$this->data = 
'<feed xmlns="http://www.w3.org/2005/Atom">
	<title><![CDATA[&]]>: Ampersand</title>
</feed>';
	}
	
	function expected()
	{
		$this->expected = '&amp;: Ampersand';
	}
}

?>