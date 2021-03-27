#[=======================================================================[.rst:
DefaultOption
-------------

Macros to provide an option with defaults dependent on other options.

Usage:

.. code-block:: cmake

  default_option(<option> "<help_text>" <depends>)

Where ``<option>`` uses the given ``<help_text>`` and is ``ON`` by default if
``<depends>`` is true.  If the ``<depends>`` condition is not true,
``<option>`` will be ``OFF`` by default.  Each element in the ``<depends>``
parameter is evaluated as an if-condition, so :ref:`Condition Syntax` can be
used.

Example invocation:

.. code-block:: cmake

  default_option(USE_FOO "Use Foo" "USE_BAR;NOT USE_ZOT")

If ``USE_BAR`` is true and ``USE_ZOT`` is false, the option called ``USE_FOO``
will default to ``ON``, otherwise it will default to ``OFF``.

In contrast to ``cmake_dependent_option`` which disables the option completely
if the ``<depends>`` condition evaluates to false, ``default_option`` will only
set a default and the value may be overridden by the user.
#]=======================================================================]

macro(DEFAULT_OPTION option doc depends)
  set(${option}_DEFAULT_ON 1)
  foreach(d ${depends})
    string(REGEX REPLACE " +" ";" DEFAULT_OPTION_DEP "${d}")
    if(${DEFAULT_OPTION_DEP})
    else()
      set(${option}_DEFAULT_ON 0)
    endif()
  endforeach()
  if(${option}_DEFAULT_ON)
    option(${option} "${doc}" "ON")
  else()
    option(${option} "${doc}" "OFF")
  endif()
endmacro()
