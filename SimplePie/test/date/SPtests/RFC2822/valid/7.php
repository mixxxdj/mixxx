<?php

class SimplePie_Date_Test_RFC2822_7 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = 'Fri, 05 Nov 94 09:15:30 EDT';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>