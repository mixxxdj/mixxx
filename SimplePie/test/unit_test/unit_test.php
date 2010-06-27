<?php

/**
 * @package Unit Test
 * @author Geoffrey Sneddon <geoffers@gmail.com>
 * @version $Id: unit_test.php 6 2007-04-23 15:15:40Z gsnedders $
 * @license http://www.opensource.org/licenses/zlib-license.php zlib/libpng license
 * @license http://opensource.org/licenses/lgpl-license.php GNU Lesser General Public License
 * @copyright Copyright © 2007, Geoffrey Sneddon
 */

class Unit_Test
{
	var $passed;
	var $failed;
	var $success_callback;
	var $fail_callback;
	
	function Unit_Test($success, $fail)
	{
		$this->success_callback = $success;
		$this->fail_callback = $fail;
	}
	
	function do_test($callback, $dir, $vars = 'data')
	{
		$files = $this->get_files($dir);
		foreach ($files as $file)
		{
			$istest = true;
			$debug = false;
			include $file;
			if ($istest)
			{
				$args = compact($vars);
				$result = call_user_func_array($callback, $args);
				$this->run_test($file, $result === $expected);
				if ($debug)
				{
					var_dump($file, $args, $result, $expected);
				}
			}
		}
	}
	
	function run_test($file, $success)
	{
		if ($success)
		{
			$this->passed++;
			call_user_func($this->success_callback, $file);
		}
		else
		{
			$this->failed++;
			call_user_func($this->fail_callback, $file);
		}
	}
	
	function passed()
	{
		return $this->passed;
	}
	
	function failed()
	{
		return $this->failed;
	}
	
	function total()
	{
		return $this->passed + $this->failed;
	}
	
	function get_files($dir)
	{
		static $extension = null;
		if (!$extension)
		{
			$extension = pathinfo(__FILE__, PATHINFO_EXTENSION);
		}
		$files = array();
		if ($dh = opendir($dir))
		{
			while (($file = readdir($dh)) !== false)
			{
				if (substr($file, 0, 1) != '.')
				{
					$files[] = "$dir/$file";
				}
			}
			closedir($dh);
			usort($files, array(&$this, 'sort_files'));
			foreach ($files as $file)
			{
				if (is_dir($file))
				{
					array_splice($files, array_search($file, $files), 0, $this->get_files($file));
				}
				if (pathinfo($file, PATHINFO_EXTENSION) != $extension)
				{
					unset($files[array_search($file, $files)]);
				}
			}
		}
		return $files;
	}
	
	function sort_files(&$a, &$b)
	{
		if (is_dir($a) && is_dir($b) || !(is_dir($a) || is_dir($b)))
		{
			return strnatcasecmp($a, $b);
		}
		else if (is_dir($a))
		{
			return 1;
		}
		else if (is_dir($b))
		{
			return -1;
		}
	}
}

?>