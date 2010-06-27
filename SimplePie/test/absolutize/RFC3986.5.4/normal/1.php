<?php

class SimplePie_Absolutize_Test_RFC3986_Normal_1 extends SimplePie_Absolutize_Test_RFC3986
{
	function data()
	{
		$this->data['relative'] = 'g:h';
	}
	
	function expected()
	{
		$this->expected = 'g:h';
	}
}

?>