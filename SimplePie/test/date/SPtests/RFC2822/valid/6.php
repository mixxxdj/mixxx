<?php

class SimplePie_Date_Test_RFC2822_6 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = 'Fri, 05 Nov 94 08:15:30 EST';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>