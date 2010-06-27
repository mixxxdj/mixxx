<?php

class SimplePie_Absolutize_Test_RFC3986_Abnormal_16 extends SimplePie_Absolutize_Test_RFC3986
{
	function data()
	{
		$this->data['relative'] = 'g?y/../x';
	}
	
	function expected()
	{
		$this->expected = 'http://a/b/c/g?y/../x';
	}
}

?>