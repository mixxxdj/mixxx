<?php

class SimplePie_Absolutize_Test_RFC3986_Normal_7 extends SimplePie_Absolutize_Test_RFC3986
{
	function data()
	{
		$this->data['relative'] = '?y';
	}
	
	function expected()
	{
		$this->expected = 'http://a/b/c/d;p?y';
	}
}

?>