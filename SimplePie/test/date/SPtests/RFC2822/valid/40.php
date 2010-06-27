<?php

class SimplePie_Date_Test_RFC2822_40 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = 'Fri, 05 Nov 94 13:15:30 -0000';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>