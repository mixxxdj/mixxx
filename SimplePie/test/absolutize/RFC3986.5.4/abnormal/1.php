<?php

class SimplePie_Absolutize_Test_RFC3986_Abnormal_1 extends SimplePie_Absolutize_Test_RFC3986
{
	function data()
	{
		$this->data['relative'] = '../../../g';
	}
	
	function expected()
	{
		$this->expected = 'http://a/g';
	}
}

?>