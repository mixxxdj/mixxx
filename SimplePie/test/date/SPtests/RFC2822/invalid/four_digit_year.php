<?php

class SimplePie_Date_Test_RFC2822_Four_Digit_Year extends SimplePie_Date_Test
{
	function data()
	{
		$this->data = 'Fri, 05 Nov 1994 13:15:30 GMT';
	}
	
	function expected()
	{
		$this->expected = 784041330;
	}
}

?>