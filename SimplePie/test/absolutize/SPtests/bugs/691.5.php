<?php

class SimplePie_Absolutize_Test_Bug_691_Test_5 extends SimplePie_Absolutize_Test
{
	function data()
	{
		$this->data['base'] = 'zero://a/b/c';
		$this->data['relative'] = 'd';
	}
	
	function expected()
	{
		$this->expected = 'zero://a/b/d';
	}
}

?>