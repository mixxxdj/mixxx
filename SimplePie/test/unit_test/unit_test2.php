<?php

/**
 * @package Unit Test
 * @author Geoffrey Sneddon <geoffers@gmail.com>
 * @version $Id: unit_test2.php 16 2007-08-08 14:52:36Z gsnedders $
 * @license http://www.opensource.org/licenses/zlib-license.php zlib/libpng license
 * @license http://opensource.org/licenses/lgpl-license.php GNU Lesser General Public License
 * @copyright Copyright © 2007, Geoffrey Sneddon
 */

/**
 * Unit Test
 *
 * @abstract
 * @package Unit Test
 */
class Unit_Test2
{
	/**
	 * Sets whether this class is a unit test or not
	 *
	 * @access protected
	 * @var bool
	 */
	var $test = true;

	/**
	 * Test name
	 *
	 * @access protected
	 * @var mixed
	 */
	var $name;

	/**
	 * Test data
	 *
	 * @access protected
	 * @var mixed
	 */
	var $data;

	/**
	 * Expected result
	 *
	 * @access protected
	 * @var mixed
	 */
	var $expected;

	/**
	 * Test result
	 *
	 * @access protected
	 * @var mixed
	 */
	var $result;

	/**
	 * Number of tests passed
	 *
	 * @access protected
	 * @var int
	 */
	var $passes = 0;

	/**
	 * Number of tests failed
	 *
	 * @access protected
	 * @var int
	 */
	var $fails = 0;

	/**
	 * Set the test name to the class name by default, replacing "_" with " "
	 */
	function Unit_Test2()
	{
		$this->name = str_replace('_', ' ', get_class($this));
	}

	/**
	 * Whether this class is a test
	 *
	 * @final
	 * @access public
	 * @return bool
	 */
	function is_test()
	{
		return (bool) $this->test;
	}

	/**
	 * Test name
	 *
	 * @final
	 * @access public
	 * @return mixed
	 */
	function name()
	{
		return $this->name;
	}

	/**
	 * Number of tests passed
	 *
	 * @final
	 * @access public
	 * @return int
	 */
	function passes()
	{
		return (int) $this->passes;
	}

	/**
	 * Number of tests failed
	 *
	 * @final
	 * @access public
	 * @return int
	 */
	function fails()
	{
		return (int) $this->fails;
	}

	/**
	 * Total number of tests
	 *
	 * @final
	 * @access public
	 * @return int
	 */
	function total()
	{
		return $this->passes() + $this->fails();
	}

	/**
	 * Run the test
	 *
	 * @final
	 * @access public
	 */
	function run()
	{
		$this->init();
		$this->data();
		$this->expected();
		$this->test();
		$this->result();
	}

	/**
	 * First method called when running the test
	 *
	 * This isn't defined as abstract as it's optional
	 *
	 * @access protected
	 */
	function init()
	{
	}

	/**
	 * Set Unit_Test2::$data
	 *
	 * @abstract
	 * @access protected
	 * @see Unit_Test2::$data
	 */
	function data()
	{
	}

	/**
	 * Set Unit_Test2::$expected
	 *
	 * @abstract
	 * @access protected
	 * @see Unit_Test2::$expected
	 */
	function expected()
	{
	}

	/**
	 * Actually run the test (should set Unit_Test::$result)
	 *
	 * @abstract
	 * @access protected
	 * @see Unit_Test2::$result
	 */
	function test()
	{
	}

	/**
	 * Check whether the result is valid (should call Unit_Test2::pass() or Unit_Test2::fail())
	 *
	 * @abstract
	 * @access protected
	 * @see Unit_Test2::$expected
	 * @see Unit_Test2::$result
	 */
	function result()
	{
	}

	/**
	 * Process a pass
	 *
	 * @access protected
	 */
	function pass()
	{
		$this->passes++;
	}

	/**
	 * Process a fail
	 *
	 * @access protected
	 */
	function fail()
	{
		$this->fails++;
	}
}

/**
 * Unit Test Group
 *
 * @package Unit Test
 */
class Unit_Test2_Group
{
	/**
	 * Unit Test Group Name
	 *
	 * @access protected
	 * @var mixed
	 */
	var $name;

	/**
	 * Tests
	 *
	 * @access protected
	 * @var array
	 */
	var $tests = array(array());

	/**
	 * Number of tests passed
	 *
	 * @access protected
	 * @var int
	 */
	var $passes = 0;

	/**
	 * Number of tests failed
	 *
	 * @access protected
	 * @var int
	 */
	var $fails = 0;

	/**
	 * Time taken to run tests
	 *
	 * @access protected
	 * @var float
	 */
	var $time = 0.0;

	/**
	 * Create Unit Test Group
	 *
	 * @access public
	 * @param string $name Unit Test Group Name
	 */
	function Unit_Test2_Group($name)
	{
		$this->name = $name;
	}

	/**
	 * Unit Test Group Name
	 *
	 * @final
	 * @access public
	 * @return mixed
	 */
	function name()
	{
		return $this->name;
	}

	/**
	 * Number of tests passed
	 *
	 * @final
	 * @access public
	 * @return int
	 */
	function passes()
	{
		return (int) $this->passes;
	}

	/**
	 * Number of tests failed
	 *
	 * @final
	 * @access public
	 * @return int
	 */
	function fails()
	{
		return (int) $this->fails;
	}

	/**
	 * Total number of tests
	 *
	 * @final
	 * @access public
	 * @return int
	 */
	function total()
	{
		return $this->passes() + $this->fails();
	}

	/**
	 * Time to run tests
	 *
	 * @final
	 * @access public
	 * @return float
	 */
	function time()
	{
		return (float) $this->time;
	}

	/**
	 * Add a test (a Unit_Test2 child, or a Unit_Test2_Group)
	 *
	 * @access public
	 * @param object $test Test to add
	 */
	function add($test)
	{
		$this->tests[$test->name()][] = $test;
	}

	/**
	 * Remove a test
	 *
	 * @access public
	 * @param string $name Test name
	 */
	function remove($name)
	{
		unset($this->tests[$name]);
	}

	/**
	 * Load tests in folder
	 *
	 * This loads all the Unit_Test2 classes within files with the same
	 * extension as this file within the specified folder
	 *
	 * @access public
	 * @param string $folder Folder name
	 */
	function load_folder($folder)
	{
		static $extension = null;
		if (!$extension)
		{
			$extension = pathinfo(__FILE__, PATHINFO_EXTENSION);
		}
		$files = Unit_Test2_Files::get_files($folder);
		$count_classes = count(get_declared_classes());
		foreach ($files as $file)
		{
			if (is_file($file) && pathinfo($file, PATHINFO_EXTENSION) === $extension)
			{
				include $file;
			}
		}
		$classes = array_slice(get_declared_classes(), $count_classes);
		foreach ($classes as $class)
		{
			if ($this->is_subclass_of($class, 'Unit_Test2'))
			{
				$class = new $class;
				if ($class->is_test())
				{
					$this->add($class);
				}
			}
		}
	}

	/**
	 * Run the tests
	 *
	 * @access public
	 */
	function run()
	{
		$this->pre();
		$start_time = $this->microtime(true);
		foreach ($this->tests as $tests)
		{
			foreach ($tests as $test)
			{
				if ($this->is_a($test, 'Unit_Test2') || $this->is_a($test, 'Unit_Test2_Group'))
				{
					$test->run();
					$this->passes += $test->passes();
					$this->fails += $test->fails();
				}
			}
		}
		$this->time = $this->microtime(true) - $start_time;
		$this->post();
	}

	/**
	 * Executed before the tests are executed
	 *
	 * @abstract
	 * @access protected
	 */
	function pre()
	{
	}

	/**
	 * Executed after the tests are executed
	 *
	 * @abstract
	 * @access protected
	 */
	function post()
	{
	}

	/**
	 * Re-implementation of PHP 5.0.3's is_subclass_of()
	 *
	 * @access public
	 * @param mixed $object
	 * @param string $class_name
	 */
	function is_subclass_of($object, $class_name)
	{
		if (func_num_args() != 2)
		{
			trigger_error('Wrong parameter count for SimplePie_Misc::is_subclass_of()', E_USER_WARNING);
		}
		else
		{
			if (version_compare(phpversion(), '5.0.3', '>=') || is_object($object))
			{
				return is_subclass_of($object, $class_name);
			}
			else if (is_string($object) && is_string($class_name))
			{
				if (class_exists($object))
				{
					if (class_exists($class_name))
					{
						$class_name = strtolower($class_name);
						while ($object = strtolower(get_parent_class($object)))
						{
							if ($object == $class_name)
							{
								return true;
							}
						}
					}
				}
				else
				{
					trigger_error('Unknown class passed as parameter', E_USER_WARNNG);
				}
			}
			return false;
		}
	}

	/**
	 * Re-implementation of PHP 4.2.0's is_a()
	 *
	 * @access public
	 * @param object $object The tested object
	 * @param string $class_name The class name
	 * @return bool Returns true if the object is of this class or has this class as one of its parents, false otherwise
	 */
	 function is_a($object, $class_name)
	 {
	 	if (function_exists('is_a'))
	 	{
	 		return is_a($object, $class_name);
	 	}
	 	elseif (!is_object($object))
	 	{
	 		return false;
	 	}
	 	elseif (get_class($object) == strtolower($class_name))
	 	{
	 		return true;
	 	}
	 	else
	 	{
	 		return is_subclass_of($object, $class_name);
	 	}
	 }

	/**
	 * Re-implementation of PHP 5's microtime()
	 *
	 * @access public
	 * @param bool $get_as_float
	 */
	function microtime($get_as_float = false)
	{
		if ($get_as_float)
		{
			if (is_float($time = microtime(true)))
			{
				return $time;
			}
			else
			{
				list($user, $sec) = explode(' ', $time);
				return ((float) $user + (float) $sec);
			}
		}
		else
		{
			// PHP6 will likely return a float by default, so explicitly pass false (this is just ignored under PHP < 5)
			return microtime(false);
		}
	}
}

/**
 * File listing class
 *
 * @package Unit Test
 */
class Unit_Test2_Files
{
	/**
	 * Get a list of files/folders within $dir
	 *
	 * @static
	 * @access public
	 * @param string $dir Folder to get listing for
	 * @return array
	 */
	function get_files($dir)
	{
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
			usort($files, array(__CLASS__, 'sort_files'));
			foreach ($files as $file)
			{
				if (is_dir($file))
				{
					array_splice($files, array_search($file, $files), 0, Unit_Test2_Files::get_files($file));
				}
			}
		}
		return $files;
	}

	/**
	 * Sort files/folders with files listed before inner folders
	 *
	 * @static
	 * @access public
	 * @param string $a File/folder 1
	 * @param string $b File/folder 2
	 * @return int
	 */
	function sort_files($a, $b)
	{
		if (is_dir($a) && is_dir($b))
		{
			return strnatcmp($a, $b);
		}
		else if (is_dir($a))
		{
			return 1;
		}
		else if (is_dir($b))
		{
			return -1;
		}
		else
		{
			return strnatcmp($a, $b);
		}
	}
}

?>