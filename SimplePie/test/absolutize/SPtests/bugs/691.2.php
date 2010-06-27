<?php

class SimplePie_Absolutize_Test_Bug_691_Test_2 extends SimplePie_Absolutize_Test
{
	function data()
	{
		$this->data['base'] = 'http://a/b/c';
		$this->data['relative'] = '0';
	}
	
	function expected()
	{
		$this->expected = 'http://a/b/0';
	}
}

?>