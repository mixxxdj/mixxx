<?php

class SimplePie_Absolutize_Test_Bug_1091_Test_0 extends SimplePie_Absolutize_Test
{
	function data()
	{
		$this->data['base'] = 'http:g';
		$this->data['relative'] = 'a';
	}
	
	function expected()
	{
		$this->expected = 'http:a';
	}
}

?>