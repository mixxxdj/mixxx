<?php

class SimplePie_Date_Test_RFC2822_25 extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = 'Fri, 05 Nov 94 13:15:30 M';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>