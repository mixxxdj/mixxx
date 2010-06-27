<?php

class SimplePie_Absolutize_Test_Bug_691_Test_6 extends SimplePie_Absolutize_Test
{
	function data()
	{
		$this->data['base'] = 'http://0/b/c';
		$this->data['relative'] = 'd';
	}
	
	function expected()
	{
		$this->expected = 'http://0/b/d';
	}
}

?>