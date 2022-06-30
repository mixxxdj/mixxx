var NovationLaunchpad = (function () {
  function _classCallCheck(instance, Constructor) {
    if (!(instance instanceof Constructor)) {
      throw new TypeError("Cannot call a class as a function");
    }
  }

  function _defineProperties(target, props) {
    for (var i = 0; i < props.length; i++) {
      var descriptor = props[i];
      descriptor.enumerable = descriptor.enumerable || false;
      descriptor.configurable = true;
      if ("value" in descriptor) descriptor.writable = true;
      Object.defineProperty(target, descriptor.key, descriptor);
    }
  }

  function _createClass(Constructor, protoProps, staticProps) {
    if (protoProps) _defineProperties(Constructor.prototype, protoProps);
    if (staticProps) _defineProperties(Constructor, staticProps);
    return Constructor;
  }

  function _defineProperty(obj, key, value) {
    if (key in obj) {
      Object.defineProperty(obj, key, {
        value: value,
        enumerable: true,
        configurable: true,
        writable: true
      });
    } else {
      obj[key] = value;
    }

    return obj;
  }

  function _assertThisInitialized(self) {
    if (self === void 0) {
      throw new ReferenceError("this hasn't been initialised - super() hasn't been called");
    }

    return self;
  }

  function _typeof(obj) {
    "@babel/helpers - typeof";

    if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") {
      _typeof = function _typeof(obj) {
        return typeof obj;
      };
    } else {
      _typeof = function _typeof(obj) {
        return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj;
      };
    }

    return _typeof(obj);
  }

  function _possibleConstructorReturn(self, call) {
    if (call && (_typeof(call) === "object" || typeof call === "function")) {
      return call;
    }

    return _assertThisInitialized(self);
  }

  function _getPrototypeOf(o) {
    _getPrototypeOf = Object.setPrototypeOf ? Object.getPrototypeOf : function _getPrototypeOf(o) {
      return o.__proto__ || Object.getPrototypeOf(o);
    };
    return _getPrototypeOf(o);
  }

  function _setPrototypeOf(o, p) {
    _setPrototypeOf = Object.setPrototypeOf || function _setPrototypeOf(o, p) {
      o.__proto__ = p;
      return o;
    };

    return _setPrototypeOf(o, p);
  }

  function _inherits(subClass, superClass) {
    if (typeof superClass !== "function" && superClass !== null) {
      throw new TypeError("Super expression must either be null or a function");
    }

    subClass.prototype = Object.create(superClass && superClass.prototype, {
      constructor: {
        value: subClass,
        writable: true,
        configurable: true
      }
    });
    if (superClass) _setPrototypeOf(subClass, superClass);
  }

  function _classCallCheck$1(instance, Constructor) {
    if (!(instance instanceof Constructor)) {
      throw new TypeError("Cannot call a class as a function");
    }
  }

  function _defineProperties$1(target, props) {
    for (var i = 0; i < props.length; i++) {
      var descriptor = props[i];
      descriptor.enumerable = descriptor.enumerable || false;
      descriptor.configurable = true;
      if ("value" in descriptor) descriptor.writable = true;
      Object.defineProperty(target, descriptor.key, descriptor);
    }
  }

  function _createClass$1(Constructor, protoProps, staticProps) {
    if (protoProps) _defineProperties$1(Constructor.prototype, protoProps);
    if (staticProps) _defineProperties$1(Constructor, staticProps);
    return Constructor;
  }

  function _defineProperty$1(obj, key, value) {
    if (key in obj) {
      Object.defineProperty(obj, key, {
        value: value,
        enumerable: true,
        configurable: true,
        writable: true
      });
    } else {
      obj[key] = value;
    }

    return obj;
  }

  function _arrayWithHoles(arr) {
    if (Array.isArray(arr)) return arr;
  }

  function _iterableToArrayLimit(arr, i) {
    if (typeof Symbol === "undefined" || !(Symbol.iterator in Object(arr))) return;
    var _arr = [];
    var _n = true;
    var _d = false;
    var _e = undefined;

    try {
      for (var _i = arr[Symbol.iterator](), _s; !(_n = (_s = _i.next()).done); _n = true) {
        _arr.push(_s.value);

        if (i && _arr.length === i) break;
      }
    } catch (err) {
      _d = true;
      _e = err;
    } finally {
      try {
        if (!_n && _i["return"] != null) _i["return"]();
      } finally {
        if (_d) throw _e;
      }
    }

    return _arr;
  }

  function _arrayLikeToArray(arr, len) {
    if (len == null || len > arr.length) len = arr.length;

    for (var i = 0, arr2 = new Array(len); i < len; i++) {
      arr2[i] = arr[i];
    }

    return arr2;
  }

  function _unsupportedIterableToArray(o, minLen) {
    if (!o) return;
    if (typeof o === "string") return _arrayLikeToArray(o, minLen);
    var n = Object.prototype.toString.call(o).slice(8, -1);
    if (n === "Object" && o.constructor) n = o.constructor.name;
    if (n === "Map" || n === "Set") return Array.from(n);
    if (n === "Arguments" || /^(?:Ui|I)nt(?:8|16|32)(?:Clamped)?Array$/.test(n)) return _arrayLikeToArray(o, minLen);
  }

  function _nonIterableRest() {
    throw new TypeError("Invalid attempt to destructure non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
  }

  function _slicedToArray(arr, i) {
    return _arrayWithHoles(arr) || _iterableToArrayLimit(arr, i) || _unsupportedIterableToArray(arr, i) || _nonIterableRest();
  }

  /* global engine, midi, script */
  var engine_1 = engine;
  var midi_1 = midi;
  var script_1 = script;

  /** Detect free variable `global` from Node.js. */
  var freeGlobal = (typeof global === "undefined" ? "undefined" : _typeof(global)) == 'object' && global && global.Object === Object && global;

  /** Detect free variable `self`. */

  var freeSelf = (typeof self === "undefined" ? "undefined" : _typeof(self)) == 'object' && self && self.Object === Object && self;
  /** Used as a reference to the global object. */

  var root = freeGlobal || freeSelf || Function('return this')();

  /** Built-in value references. */

  var _Symbol = root.Symbol;

  /** Used for built-in method references. */

  var objectProto = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty = objectProto.hasOwnProperty;
  /**
   * Used to resolve the
   * [`toStringTag`](http://ecma-international.org/ecma-262/7.0/#sec-object.prototype.tostring)
   * of values.
   */

  var nativeObjectToString = objectProto.toString;
  /** Built-in value references. */

  var symToStringTag = _Symbol ? _Symbol.toStringTag : undefined;
  /**
   * A specialized version of `baseGetTag` which ignores `Symbol.toStringTag` values.
   *
   * @private
   * @param {*} value The value to query.
   * @returns {string} Returns the raw `toStringTag`.
   */

  function getRawTag(value) {
    var isOwn = hasOwnProperty.call(value, symToStringTag),
        tag = value[symToStringTag];

    try {
      value[symToStringTag] = undefined;
      var unmasked = true;
    } catch (e) {}

    var result = nativeObjectToString.call(value);

    if (unmasked) {
      if (isOwn) {
        value[symToStringTag] = tag;
      } else {
        delete value[symToStringTag];
      }
    }

    return result;
  }

  /** Used for built-in method references. */
  var objectProto$1 = Object.prototype;
  /**
   * Used to resolve the
   * [`toStringTag`](http://ecma-international.org/ecma-262/7.0/#sec-object.prototype.tostring)
   * of values.
   */

  var nativeObjectToString$1 = objectProto$1.toString;
  /**
   * Converts `value` to a string using `Object.prototype.toString`.
   *
   * @private
   * @param {*} value The value to convert.
   * @returns {string} Returns the converted string.
   */

  function objectToString(value) {
    return nativeObjectToString$1.call(value);
  }

  /** `Object#toString` result references. */

  var nullTag = '[object Null]',
      undefinedTag = '[object Undefined]';
  /** Built-in value references. */

  var symToStringTag$1 = _Symbol ? _Symbol.toStringTag : undefined;
  /**
   * The base implementation of `getTag` without fallbacks for buggy environments.
   *
   * @private
   * @param {*} value The value to query.
   * @returns {string} Returns the `toStringTag`.
   */

  function baseGetTag(value) {
    if (value == null) {
      return value === undefined ? undefinedTag : nullTag;
    }

    return symToStringTag$1 && symToStringTag$1 in Object(value) ? getRawTag(value) : objectToString(value);
  }

  /**
   * Checks if `value` is the
   * [language type](http://www.ecma-international.org/ecma-262/7.0/#sec-ecmascript-language-types)
   * of `Object`. (e.g. arrays, functions, objects, regexes, `new Number(0)`, and `new String('')`)
   *
   * @static
   * @memberOf _
   * @since 0.1.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is an object, else `false`.
   * @example
   *
   * _.isObject({});
   * // => true
   *
   * _.isObject([1, 2, 3]);
   * // => true
   *
   * _.isObject(_.noop);
   * // => true
   *
   * _.isObject(null);
   * // => false
   */
  function isObject(value) {
    var type = _typeof(value);

    return value != null && (type == 'object' || type == 'function');
  }

  /** `Object#toString` result references. */

  var asyncTag = '[object AsyncFunction]',
      funcTag = '[object Function]',
      genTag = '[object GeneratorFunction]',
      proxyTag = '[object Proxy]';
  /**
   * Checks if `value` is classified as a `Function` object.
   *
   * @static
   * @memberOf _
   * @since 0.1.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a function, else `false`.
   * @example
   *
   * _.isFunction(_);
   * // => true
   *
   * _.isFunction(/abc/);
   * // => false
   */

  function isFunction(value) {
    if (!isObject(value)) {
      return false;
    } // The use of `Object#toString` avoids issues with the `typeof` operator
    // in Safari 9 which returns 'object' for typed arrays and other constructors.


    var tag = baseGetTag(value);
    return tag == funcTag || tag == genTag || tag == asyncTag || tag == proxyTag;
  }

  /** Used to detect overreaching core-js shims. */

  var coreJsData = root['__core-js_shared__'];

  /** Used to detect methods masquerading as native. */

  var maskSrcKey = function () {
    var uid = /[^.]+$/.exec(coreJsData && coreJsData.keys && coreJsData.keys.IE_PROTO || '');
    return uid ? 'Symbol(src)_1.' + uid : '';
  }();
  /**
   * Checks if `func` has its source masked.
   *
   * @private
   * @param {Function} func The function to check.
   * @returns {boolean} Returns `true` if `func` is masked, else `false`.
   */


  function isMasked(func) {
    return !!maskSrcKey && maskSrcKey in func;
  }

  /** Used for built-in method references. */
  var funcProto = Function.prototype;
  /** Used to resolve the decompiled source of functions. */

  var funcToString = funcProto.toString;
  /**
   * Converts `func` to its source code.
   *
   * @private
   * @param {Function} func The function to convert.
   * @returns {string} Returns the source code.
   */

  function toSource(func) {
    if (func != null) {
      try {
        return funcToString.call(func);
      } catch (e) {}

      try {
        return func + '';
      } catch (e) {}
    }

    return '';
  }

  /**
   * Used to match `RegExp`
   * [syntax characters](http://ecma-international.org/ecma-262/7.0/#sec-patterns).
   */

  var reRegExpChar = /[\\^$.*+?()[\]{}|]/g;
  /** Used to detect host constructors (Safari). */

  var reIsHostCtor = /^\[object .+?Constructor\]$/;
  /** Used for built-in method references. */

  var funcProto$1 = Function.prototype,
      objectProto$2 = Object.prototype;
  /** Used to resolve the decompiled source of functions. */

  var funcToString$1 = funcProto$1.toString;
  /** Used to check objects for own properties. */

  var hasOwnProperty$1 = objectProto$2.hasOwnProperty;
  /** Used to detect if a method is native. */

  var reIsNative = RegExp('^' + funcToString$1.call(hasOwnProperty$1).replace(reRegExpChar, '\\$&').replace(/hasOwnProperty|(function).*?(?=\\\()| for .+?(?=\\\])/g, '$1.*?') + '$');
  /**
   * The base implementation of `_.isNative` without bad shim checks.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a native function,
   *  else `false`.
   */

  function baseIsNative(value) {
    if (!isObject(value) || isMasked(value)) {
      return false;
    }

    var pattern = isFunction(value) ? reIsNative : reIsHostCtor;
    return pattern.test(toSource(value));
  }

  /**
   * Gets the value at `key` of `object`.
   *
   * @private
   * @param {Object} [object] The object to query.
   * @param {string} key The key of the property to get.
   * @returns {*} Returns the property value.
   */
  function getValue(object, key) {
    return object == null ? undefined : object[key];
  }

  /**
   * Gets the native function at `key` of `object`.
   *
   * @private
   * @param {Object} object The object to query.
   * @param {string} key The key of the method to get.
   * @returns {*} Returns the function if it's native, else `undefined`.
   */

  function getNative(object, key) {
    var value = getValue(object, key);
    return baseIsNative(value) ? value : undefined;
  }

  var defineProperty = function () {
    try {
      var func = getNative(Object, 'defineProperty');
      func({}, '', {});
      return func;
    } catch (e) {}
  }();

  /**
   * The base implementation of `assignValue` and `assignMergeValue` without
   * value checks.
   *
   * @private
   * @param {Object} object The object to modify.
   * @param {string} key The key of the property to assign.
   * @param {*} value The value to assign.
   */

  function baseAssignValue(object, key, value) {
    if (key == '__proto__' && defineProperty) {
      defineProperty(object, key, {
        'configurable': true,
        'enumerable': true,
        'value': value,
        'writable': true
      });
    } else {
      object[key] = value;
    }
  }

  /**
   * Performs a
   * [`SameValueZero`](http://ecma-international.org/ecma-262/7.0/#sec-samevaluezero)
   * comparison between two values to determine if they are equivalent.
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to compare.
   * @param {*} other The other value to compare.
   * @returns {boolean} Returns `true` if the values are equivalent, else `false`.
   * @example
   *
   * var object = { 'a': 1 };
   * var other = { 'a': 1 };
   *
   * _.eq(object, object);
   * // => true
   *
   * _.eq(object, other);
   * // => false
   *
   * _.eq('a', 'a');
   * // => true
   *
   * _.eq('a', Object('a'));
   * // => false
   *
   * _.eq(NaN, NaN);
   * // => true
   */
  function eq(value, other) {
    return value === other || value !== value && other !== other;
  }

  /** Used for built-in method references. */

  var objectProto$3 = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$2 = objectProto$3.hasOwnProperty;
  /**
   * Assigns `value` to `key` of `object` if the existing value is not equivalent
   * using [`SameValueZero`](http://ecma-international.org/ecma-262/7.0/#sec-samevaluezero)
   * for equality comparisons.
   *
   * @private
   * @param {Object} object The object to modify.
   * @param {string} key The key of the property to assign.
   * @param {*} value The value to assign.
   */

  function assignValue(object, key, value) {
    var objValue = object[key];

    if (!(hasOwnProperty$2.call(object, key) && eq(objValue, value)) || value === undefined && !(key in object)) {
      baseAssignValue(object, key, value);
    }
  }

  /**
   * Copies properties of `source` to `object`.
   *
   * @private
   * @param {Object} source The object to copy properties from.
   * @param {Array} props The property identifiers to copy.
   * @param {Object} [object={}] The object to copy properties to.
   * @param {Function} [customizer] The function to customize copied values.
   * @returns {Object} Returns `object`.
   */

  function copyObject(source, props, object, customizer) {
    var isNew = !object;
    object || (object = {});
    var index = -1,
        length = props.length;

    while (++index < length) {
      var key = props[index];
      var newValue = customizer ? customizer(object[key], source[key], key, object, source) : undefined;

      if (newValue === undefined) {
        newValue = source[key];
      }

      if (isNew) {
        baseAssignValue(object, key, newValue);
      } else {
        assignValue(object, key, newValue);
      }
    }

    return object;
  }

  /**
   * This method returns the first argument it receives.
   *
   * @static
   * @since 0.1.0
   * @memberOf _
   * @category Util
   * @param {*} value Any value.
   * @returns {*} Returns `value`.
   * @example
   *
   * var object = { 'a': 1 };
   *
   * console.log(_.identity(object) === object);
   * // => true
   */
  function identity(value) {
    return value;
  }

  /**
   * A faster alternative to `Function#apply`, this function invokes `func`
   * with the `this` binding of `thisArg` and the arguments of `args`.
   *
   * @private
   * @param {Function} func The function to invoke.
   * @param {*} thisArg The `this` binding of `func`.
   * @param {Array} args The arguments to invoke `func` with.
   * @returns {*} Returns the result of `func`.
   */
  function apply(func, thisArg, args) {
    switch (args.length) {
      case 0:
        return func.call(thisArg);

      case 1:
        return func.call(thisArg, args[0]);

      case 2:
        return func.call(thisArg, args[0], args[1]);

      case 3:
        return func.call(thisArg, args[0], args[1], args[2]);
    }

    return func.apply(thisArg, args);
  }

  /* Built-in method references for those with the same name as other `lodash` methods. */

  var nativeMax = Math.max;
  /**
   * A specialized version of `baseRest` which transforms the rest array.
   *
   * @private
   * @param {Function} func The function to apply a rest parameter to.
   * @param {number} [start=func.length-1] The start position of the rest parameter.
   * @param {Function} transform The rest array transform.
   * @returns {Function} Returns the new function.
   */

  function overRest(func, start, transform) {
    start = nativeMax(start === undefined ? func.length - 1 : start, 0);
    return function () {
      var args = arguments,
          index = -1,
          length = nativeMax(args.length - start, 0),
          array = Array(length);

      while (++index < length) {
        array[index] = args[start + index];
      }

      index = -1;
      var otherArgs = Array(start + 1);

      while (++index < start) {
        otherArgs[index] = args[index];
      }

      otherArgs[start] = transform(array);
      return apply(func, this, otherArgs);
    };
  }

  /**
   * Creates a function that returns `value`.
   *
   * @static
   * @memberOf _
   * @since 2.4.0
   * @category Util
   * @param {*} value The value to return from the new function.
   * @returns {Function} Returns the new constant function.
   * @example
   *
   * var objects = _.times(2, _.constant({ 'a': 1 }));
   *
   * console.log(objects);
   * // => [{ 'a': 1 }, { 'a': 1 }]
   *
   * console.log(objects[0] === objects[1]);
   * // => true
   */
  function constant(value) {
    return function () {
      return value;
    };
  }

  /**
   * The base implementation of `setToString` without support for hot loop shorting.
   *
   * @private
   * @param {Function} func The function to modify.
   * @param {Function} string The `toString` result.
   * @returns {Function} Returns `func`.
   */

  var baseSetToString = !defineProperty ? identity : function (func, string) {
    return defineProperty(func, 'toString', {
      'configurable': true,
      'enumerable': false,
      'value': constant(string),
      'writable': true
    });
  };

  /** Used to detect hot functions by number of calls within a span of milliseconds. */
  var HOT_COUNT = 800,
      HOT_SPAN = 16;
  /* Built-in method references for those with the same name as other `lodash` methods. */

  var nativeNow = Date.now;
  /**
   * Creates a function that'll short out and invoke `identity` instead
   * of `func` when it's called `HOT_COUNT` or more times in `HOT_SPAN`
   * milliseconds.
   *
   * @private
   * @param {Function} func The function to restrict.
   * @returns {Function} Returns the new shortable function.
   */

  function shortOut(func) {
    var count = 0,
        lastCalled = 0;
    return function () {
      var stamp = nativeNow(),
          remaining = HOT_SPAN - (stamp - lastCalled);
      lastCalled = stamp;

      if (remaining > 0) {
        if (++count >= HOT_COUNT) {
          return arguments[0];
        }
      } else {
        count = 0;
      }

      return func.apply(undefined, arguments);
    };
  }

  /**
   * Sets the `toString` method of `func` to return `string`.
   *
   * @private
   * @param {Function} func The function to modify.
   * @param {Function} string The `toString` result.
   * @returns {Function} Returns `func`.
   */

  var setToString = shortOut(baseSetToString);

  /**
   * The base implementation of `_.rest` which doesn't validate or coerce arguments.
   *
   * @private
   * @param {Function} func The function to apply a rest parameter to.
   * @param {number} [start=func.length-1] The start position of the rest parameter.
   * @returns {Function} Returns the new function.
   */

  function baseRest(func, start) {
    return setToString(overRest(func, start, identity), func + '');
  }

  /** Used as references for various `Number` constants. */
  var MAX_SAFE_INTEGER = 9007199254740991;
  /**
   * Checks if `value` is a valid array-like length.
   *
   * **Note:** This method is loosely based on
   * [`ToLength`](http://ecma-international.org/ecma-262/7.0/#sec-tolength).
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a valid length, else `false`.
   * @example
   *
   * _.isLength(3);
   * // => true
   *
   * _.isLength(Number.MIN_VALUE);
   * // => false
   *
   * _.isLength(Infinity);
   * // => false
   *
   * _.isLength('3');
   * // => false
   */

  function isLength(value) {
    return typeof value == 'number' && value > -1 && value % 1 == 0 && value <= MAX_SAFE_INTEGER;
  }

  /**
   * Checks if `value` is array-like. A value is considered array-like if it's
   * not a function and has a `value.length` that's an integer greater than or
   * equal to `0` and less than or equal to `Number.MAX_SAFE_INTEGER`.
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is array-like, else `false`.
   * @example
   *
   * _.isArrayLike([1, 2, 3]);
   * // => true
   *
   * _.isArrayLike(document.body.children);
   * // => true
   *
   * _.isArrayLike('abc');
   * // => true
   *
   * _.isArrayLike(_.noop);
   * // => false
   */

  function isArrayLike(value) {
    return value != null && isLength(value.length) && !isFunction(value);
  }

  /** Used as references for various `Number` constants. */
  var MAX_SAFE_INTEGER$1 = 9007199254740991;
  /** Used to detect unsigned integer values. */

  var reIsUint = /^(?:0|[1-9]\d*)$/;
  /**
   * Checks if `value` is a valid array-like index.
   *
   * @private
   * @param {*} value The value to check.
   * @param {number} [length=MAX_SAFE_INTEGER] The upper bounds of a valid index.
   * @returns {boolean} Returns `true` if `value` is a valid index, else `false`.
   */

  function isIndex(value, length) {
    var type = _typeof(value);

    length = length == null ? MAX_SAFE_INTEGER$1 : length;
    return !!length && (type == 'number' || type != 'symbol' && reIsUint.test(value)) && value > -1 && value % 1 == 0 && value < length;
  }

  /**
   * Checks if the given arguments are from an iteratee call.
   *
   * @private
   * @param {*} value The potential iteratee value argument.
   * @param {*} index The potential iteratee index or key argument.
   * @param {*} object The potential iteratee object argument.
   * @returns {boolean} Returns `true` if the arguments are from an iteratee call,
   *  else `false`.
   */

  function isIterateeCall(value, index, object) {
    if (!isObject(object)) {
      return false;
    }

    var type = _typeof(index);

    if (type == 'number' ? isArrayLike(object) && isIndex(index, object.length) : type == 'string' && index in object) {
      return eq(object[index], value);
    }

    return false;
  }

  /**
   * Creates a function like `_.assign`.
   *
   * @private
   * @param {Function} assigner The function to assign values.
   * @returns {Function} Returns the new assigner function.
   */

  function createAssigner(assigner) {
    return baseRest(function (object, sources) {
      var index = -1,
          length = sources.length,
          customizer = length > 1 ? sources[length - 1] : undefined,
          guard = length > 2 ? sources[2] : undefined;
      customizer = assigner.length > 3 && typeof customizer == 'function' ? (length--, customizer) : undefined;

      if (guard && isIterateeCall(sources[0], sources[1], guard)) {
        customizer = length < 3 ? undefined : customizer;
        length = 1;
      }

      object = Object(object);

      while (++index < length) {
        var source = sources[index];

        if (source) {
          assigner(object, source, index, customizer);
        }
      }

      return object;
    });
  }

  /** Used for built-in method references. */
  var objectProto$4 = Object.prototype;
  /**
   * Checks if `value` is likely a prototype object.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a prototype, else `false`.
   */

  function isPrototype(value) {
    var Ctor = value && value.constructor,
        proto = typeof Ctor == 'function' && Ctor.prototype || objectProto$4;
    return value === proto;
  }

  /**
   * The base implementation of `_.times` without support for iteratee shorthands
   * or max array length checks.
   *
   * @private
   * @param {number} n The number of times to invoke `iteratee`.
   * @param {Function} iteratee The function invoked per iteration.
   * @returns {Array} Returns the array of results.
   */
  function baseTimes(n, iteratee) {
    var index = -1,
        result = Array(n);

    while (++index < n) {
      result[index] = iteratee(index);
    }

    return result;
  }

  /**
   * Checks if `value` is object-like. A value is object-like if it's not `null`
   * and has a `typeof` result of "object".
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is object-like, else `false`.
   * @example
   *
   * _.isObjectLike({});
   * // => true
   *
   * _.isObjectLike([1, 2, 3]);
   * // => true
   *
   * _.isObjectLike(_.noop);
   * // => false
   *
   * _.isObjectLike(null);
   * // => false
   */
  function isObjectLike(value) {
    return value != null && _typeof(value) == 'object';
  }

  /** `Object#toString` result references. */

  var argsTag = '[object Arguments]';
  /**
   * The base implementation of `_.isArguments`.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is an `arguments` object,
   */

  function baseIsArguments(value) {
    return isObjectLike(value) && baseGetTag(value) == argsTag;
  }

  /** Used for built-in method references. */

  var objectProto$5 = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$3 = objectProto$5.hasOwnProperty;
  /** Built-in value references. */

  var propertyIsEnumerable = objectProto$5.propertyIsEnumerable;
  /**
   * Checks if `value` is likely an `arguments` object.
   *
   * @static
   * @memberOf _
   * @since 0.1.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is an `arguments` object,
   *  else `false`.
   * @example
   *
   * _.isArguments(function() { return arguments; }());
   * // => true
   *
   * _.isArguments([1, 2, 3]);
   * // => false
   */

  var isArguments = baseIsArguments(function () {
    return arguments;
  }()) ? baseIsArguments : function (value) {
    return isObjectLike(value) && hasOwnProperty$3.call(value, 'callee') && !propertyIsEnumerable.call(value, 'callee');
  };

  /**
   * Checks if `value` is classified as an `Array` object.
   *
   * @static
   * @memberOf _
   * @since 0.1.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is an array, else `false`.
   * @example
   *
   * _.isArray([1, 2, 3]);
   * // => true
   *
   * _.isArray(document.body.children);
   * // => false
   *
   * _.isArray('abc');
   * // => false
   *
   * _.isArray(_.noop);
   * // => false
   */
  var isArray = Array.isArray;

  /**
   * This method returns `false`.
   *
   * @static
   * @memberOf _
   * @since 4.13.0
   * @category Util
   * @returns {boolean} Returns `false`.
   * @example
   *
   * _.times(2, _.stubFalse);
   * // => [false, false]
   */
  function stubFalse() {
    return false;
  }

  /** Detect free variable `exports`. */

  var freeExports = (typeof exports === "undefined" ? "undefined" : _typeof(exports)) == 'object' && exports && !exports.nodeType && exports;
  /** Detect free variable `module`. */

  var freeModule = freeExports && (typeof module === "undefined" ? "undefined" : _typeof(module)) == 'object' && module && !module.nodeType && module;
  /** Detect the popular CommonJS extension `module.exports`. */

  var moduleExports = freeModule && freeModule.exports === freeExports;
  /** Built-in value references. */

  var Buffer = moduleExports ? root.Buffer : undefined;
  /* Built-in method references for those with the same name as other `lodash` methods. */

  var nativeIsBuffer = Buffer ? Buffer.isBuffer : undefined;
  /**
   * Checks if `value` is a buffer.
   *
   * @static
   * @memberOf _
   * @since 4.3.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a buffer, else `false`.
   * @example
   *
   * _.isBuffer(new Buffer(2));
   * // => true
   *
   * _.isBuffer(new Uint8Array(2));
   * // => false
   */

  var isBuffer = nativeIsBuffer || stubFalse;

  /** `Object#toString` result references. */

  var argsTag$1 = '[object Arguments]',
      arrayTag = '[object Array]',
      boolTag = '[object Boolean]',
      dateTag = '[object Date]',
      errorTag = '[object Error]',
      funcTag$1 = '[object Function]',
      mapTag = '[object Map]',
      numberTag = '[object Number]',
      objectTag = '[object Object]',
      regexpTag = '[object RegExp]',
      setTag = '[object Set]',
      stringTag = '[object String]',
      weakMapTag = '[object WeakMap]';
  var arrayBufferTag = '[object ArrayBuffer]',
      dataViewTag = '[object DataView]',
      float32Tag = '[object Float32Array]',
      float64Tag = '[object Float64Array]',
      int8Tag = '[object Int8Array]',
      int16Tag = '[object Int16Array]',
      int32Tag = '[object Int32Array]',
      uint8Tag = '[object Uint8Array]',
      uint8ClampedTag = '[object Uint8ClampedArray]',
      uint16Tag = '[object Uint16Array]',
      uint32Tag = '[object Uint32Array]';
  /** Used to identify `toStringTag` values of typed arrays. */

  var typedArrayTags = {};
  typedArrayTags[float32Tag] = typedArrayTags[float64Tag] = typedArrayTags[int8Tag] = typedArrayTags[int16Tag] = typedArrayTags[int32Tag] = typedArrayTags[uint8Tag] = typedArrayTags[uint8ClampedTag] = typedArrayTags[uint16Tag] = typedArrayTags[uint32Tag] = true;
  typedArrayTags[argsTag$1] = typedArrayTags[arrayTag] = typedArrayTags[arrayBufferTag] = typedArrayTags[boolTag] = typedArrayTags[dataViewTag] = typedArrayTags[dateTag] = typedArrayTags[errorTag] = typedArrayTags[funcTag$1] = typedArrayTags[mapTag] = typedArrayTags[numberTag] = typedArrayTags[objectTag] = typedArrayTags[regexpTag] = typedArrayTags[setTag] = typedArrayTags[stringTag] = typedArrayTags[weakMapTag] = false;
  /**
   * The base implementation of `_.isTypedArray` without Node.js optimizations.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a typed array, else `false`.
   */

  function baseIsTypedArray(value) {
    return isObjectLike(value) && isLength(value.length) && !!typedArrayTags[baseGetTag(value)];
  }

  /**
   * The base implementation of `_.unary` without support for storing metadata.
   *
   * @private
   * @param {Function} func The function to cap arguments for.
   * @returns {Function} Returns the new capped function.
   */
  function baseUnary(func) {
    return function (value) {
      return func(value);
    };
  }

  /** Detect free variable `exports`. */

  var freeExports$1 = (typeof exports === "undefined" ? "undefined" : _typeof(exports)) == 'object' && exports && !exports.nodeType && exports;
  /** Detect free variable `module`. */

  var freeModule$1 = freeExports$1 && (typeof module === "undefined" ? "undefined" : _typeof(module)) == 'object' && module && !module.nodeType && module;
  /** Detect the popular CommonJS extension `module.exports`. */

  var moduleExports$1 = freeModule$1 && freeModule$1.exports === freeExports$1;
  /** Detect free variable `process` from Node.js. */

  var freeProcess = moduleExports$1 && freeGlobal.process;
  /** Used to access faster Node.js helpers. */

  var nodeUtil = function () {
    try {
      // Use `util.types` for Node.js 10+.
      var types = freeModule$1 && freeModule$1.require && freeModule$1.require('util').types;

      if (types) {
        return types;
      } // Legacy `process.binding('util')` for Node.js < 10.


      return freeProcess && freeProcess.binding && freeProcess.binding('util');
    } catch (e) {}
  }();

  /* Node.js helper references. */

  var nodeIsTypedArray = nodeUtil && nodeUtil.isTypedArray;
  /**
   * Checks if `value` is classified as a typed array.
   *
   * @static
   * @memberOf _
   * @since 3.0.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a typed array, else `false`.
   * @example
   *
   * _.isTypedArray(new Uint8Array);
   * // => true
   *
   * _.isTypedArray([]);
   * // => false
   */

  var isTypedArray = nodeIsTypedArray ? baseUnary(nodeIsTypedArray) : baseIsTypedArray;

  /** Used for built-in method references. */

  var objectProto$6 = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$4 = objectProto$6.hasOwnProperty;
  /**
   * Creates an array of the enumerable property names of the array-like `value`.
   *
   * @private
   * @param {*} value The value to query.
   * @param {boolean} inherited Specify returning inherited property names.
   * @returns {Array} Returns the array of property names.
   */

  function arrayLikeKeys(value, inherited) {
    var isArr = isArray(value),
        isArg = !isArr && isArguments(value),
        isBuff = !isArr && !isArg && isBuffer(value),
        isType = !isArr && !isArg && !isBuff && isTypedArray(value),
        skipIndexes = isArr || isArg || isBuff || isType,
        result = skipIndexes ? baseTimes(value.length, String) : [],
        length = result.length;

    for (var key in value) {
      if ((inherited || hasOwnProperty$4.call(value, key)) && !(skipIndexes && ( // Safari 9 has enumerable `arguments.length` in strict mode.
      key == 'length' || // Node.js 0.10 has enumerable non-index properties on buffers.
      isBuff && (key == 'offset' || key == 'parent') || // PhantomJS 2 has enumerable non-index properties on typed arrays.
      isType && (key == 'buffer' || key == 'byteLength' || key == 'byteOffset') || // Skip index properties.
      isIndex(key, length)))) {
        result.push(key);
      }
    }

    return result;
  }

  /**
   * Creates a unary function that invokes `func` with its argument transformed.
   *
   * @private
   * @param {Function} func The function to wrap.
   * @param {Function} transform The argument transform.
   * @returns {Function} Returns the new function.
   */
  function overArg(func, transform) {
    return function (arg) {
      return func(transform(arg));
    };
  }

  /* Built-in method references for those with the same name as other `lodash` methods. */

  var nativeKeys = overArg(Object.keys, Object);

  /** Used for built-in method references. */

  var objectProto$7 = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$5 = objectProto$7.hasOwnProperty;
  /**
   * The base implementation of `_.keys` which doesn't treat sparse arrays as dense.
   *
   * @private
   * @param {Object} object The object to query.
   * @returns {Array} Returns the array of property names.
   */

  function baseKeys(object) {
    if (!isPrototype(object)) {
      return nativeKeys(object);
    }

    var result = [];

    for (var key in Object(object)) {
      if (hasOwnProperty$5.call(object, key) && key != 'constructor') {
        result.push(key);
      }
    }

    return result;
  }

  /**
   * Creates an array of the own enumerable property names of `object`.
   *
   * **Note:** Non-object values are coerced to objects. See the
   * [ES spec](http://ecma-international.org/ecma-262/7.0/#sec-object.keys)
   * for more details.
   *
   * @static
   * @since 0.1.0
   * @memberOf _
   * @category Object
   * @param {Object} object The object to query.
   * @returns {Array} Returns the array of property names.
   * @example
   *
   * function Foo() {
   *   this.a = 1;
   *   this.b = 2;
   * }
   *
   * Foo.prototype.c = 3;
   *
   * _.keys(new Foo);
   * // => ['a', 'b'] (iteration order is not guaranteed)
   *
   * _.keys('hi');
   * // => ['0', '1']
   */

  function keys(object) {
    return isArrayLike(object) ? arrayLikeKeys(object) : baseKeys(object);
  }

  /** Used for built-in method references. */

  var objectProto$8 = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$6 = objectProto$8.hasOwnProperty;
  /**
   * Assigns own enumerable string keyed properties of source objects to the
   * destination object. Source objects are applied from left to right.
   * Subsequent sources overwrite property assignments of previous sources.
   *
   * **Note:** This method mutates `object` and is loosely based on
   * [`Object.assign`](https://mdn.io/Object/assign).
   *
   * @static
   * @memberOf _
   * @since 0.10.0
   * @category Object
   * @param {Object} object The destination object.
   * @param {...Object} [sources] The source objects.
   * @returns {Object} Returns `object`.
   * @see _.assignIn
   * @example
   *
   * function Foo() {
   *   this.a = 1;
   * }
   *
   * function Bar() {
   *   this.c = 3;
   * }
   *
   * Foo.prototype.b = 2;
   * Bar.prototype.d = 4;
   *
   * _.assign({ 'a': 0 }, new Foo, new Bar);
   * // => { 'a': 1, 'c': 3 }
   */

  var assign = createAssigner(function (object, source) {
    if (isPrototype(source) || isArrayLike(source)) {
      copyObject(source, keys(source), object);
      return;
    }

    for (var key in source) {
      if (hasOwnProperty$6.call(source, key)) {
        assignValue(object, key, source[key]);
      }
    }
  });

  /* Built-in method references for those with the same name as other `lodash` methods. */
  var nativeCeil = Math.ceil,
      nativeMax$1 = Math.max;
  /**
   * The base implementation of `_.range` and `_.rangeRight` which doesn't
   * coerce arguments.
   *
   * @private
   * @param {number} start The start of the range.
   * @param {number} end The end of the range.
   * @param {number} step The value to increment or decrement by.
   * @param {boolean} [fromRight] Specify iterating from right to left.
   * @returns {Array} Returns the range of numbers.
   */

  function baseRange(start, end, step, fromRight) {
    var index = -1,
        length = nativeMax$1(nativeCeil((end - start) / (step || 1)), 0),
        result = Array(length);

    while (length--) {
      result[fromRight ? length : ++index] = start;
      start += step;
    }

    return result;
  }

  /** `Object#toString` result references. */

  var symbolTag = '[object Symbol]';
  /**
   * Checks if `value` is classified as a `Symbol` primitive or object.
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a symbol, else `false`.
   * @example
   *
   * _.isSymbol(Symbol.iterator);
   * // => true
   *
   * _.isSymbol('abc');
   * // => false
   */

  function isSymbol(value) {
    return _typeof(value) == 'symbol' || isObjectLike(value) && baseGetTag(value) == symbolTag;
  }

  /** Used as references for various `Number` constants. */

  var NAN = 0 / 0;
  /** Used to match leading and trailing whitespace. */

  var reTrim = /^\s+|\s+$/g;
  /** Used to detect bad signed hexadecimal string values. */

  var reIsBadHex = /^[-+]0x[0-9a-f]+$/i;
  /** Used to detect binary string values. */

  var reIsBinary = /^0b[01]+$/i;
  /** Used to detect octal string values. */

  var reIsOctal = /^0o[0-7]+$/i;
  /** Built-in method references without a dependency on `root`. */

  var freeParseInt = parseInt;
  /**
   * Converts `value` to a number.
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to process.
   * @returns {number} Returns the number.
   * @example
   *
   * _.toNumber(3.2);
   * // => 3.2
   *
   * _.toNumber(Number.MIN_VALUE);
   * // => 5e-324
   *
   * _.toNumber(Infinity);
   * // => Infinity
   *
   * _.toNumber('3.2');
   * // => 3.2
   */

  function toNumber(value) {
    if (typeof value == 'number') {
      return value;
    }

    if (isSymbol(value)) {
      return NAN;
    }

    if (isObject(value)) {
      var other = typeof value.valueOf == 'function' ? value.valueOf() : value;
      value = isObject(other) ? other + '' : other;
    }

    if (typeof value != 'string') {
      return value === 0 ? value : +value;
    }

    value = value.replace(reTrim, '');
    var isBinary = reIsBinary.test(value);
    return isBinary || reIsOctal.test(value) ? freeParseInt(value.slice(2), isBinary ? 2 : 8) : reIsBadHex.test(value) ? NAN : +value;
  }

  /** Used as references for various `Number` constants. */

  var INFINITY = 1 / 0,
      MAX_INTEGER = 1.7976931348623157e+308;
  /**
   * Converts `value` to a finite number.
   *
   * @static
   * @memberOf _
   * @since 4.12.0
   * @category Lang
   * @param {*} value The value to convert.
   * @returns {number} Returns the converted number.
   * @example
   *
   * _.toFinite(3.2);
   * // => 3.2
   *
   * _.toFinite(Number.MIN_VALUE);
   * // => 5e-324
   *
   * _.toFinite(Infinity);
   * // => 1.7976931348623157e+308
   *
   * _.toFinite('3.2');
   * // => 3.2
   */

  function toFinite(value) {
    if (!value) {
      return value === 0 ? value : 0;
    }

    value = toNumber(value);

    if (value === INFINITY || value === -INFINITY) {
      var sign = value < 0 ? -1 : 1;
      return sign * MAX_INTEGER;
    }

    return value === value ? value : 0;
  }

  /**
   * Creates a `_.range` or `_.rangeRight` function.
   *
   * @private
   * @param {boolean} [fromRight] Specify iterating from right to left.
   * @returns {Function} Returns the new range function.
   */

  function createRange(fromRight) {
    return function (start, end, step) {
      if (step && typeof step != 'number' && isIterateeCall(start, end, step)) {
        end = step = undefined;
      } // Ensure the sign of `-0` is preserved.


      start = toFinite(start);

      if (end === undefined) {
        end = start;
        start = 0;
      } else {
        end = toFinite(end);
      }

      step = step === undefined ? start < end ? 1 : -1 : toFinite(step);
      return baseRange(start, end, step, fromRight);
    };
  }

  /**
   * Creates an array of numbers (positive and/or negative) progressing from
   * `start` up to, but not including, `end`. A step of `-1` is used if a negative
   * `start` is specified without an `end` or `step`. If `end` is not specified,
   * it's set to `start` with `start` then set to `0`.
   *
   * **Note:** JavaScript follows the IEEE-754 standard for resolving
   * floating-point values which can produce unexpected results.
   *
   * @static
   * @since 0.1.0
   * @memberOf _
   * @category Util
   * @param {number} [start=0] The start of the range.
   * @param {number} end The end of the range.
   * @param {number} [step=1] The value to increment or decrement by.
   * @returns {Array} Returns the range of numbers.
   * @see _.inRange, _.rangeRight
   * @example
   *
   * _.range(4);
   * // => [0, 1, 2, 3]
   *
   * _.range(-4);
   * // => [0, -1, -2, -3]
   *
   * _.range(1, 5);
   * // => [1, 2, 3, 4]
   *
   * _.range(0, 20, 5);
   * // => [0, 5, 10, 15]
   *
   * _.range(0, -4, -1);
   * // => [0, -1, -2, -3]
   *
   * _.range(1, 4, 0);
   * // => [1, 1, 1]
   *
   * _.range(0);
   * // => []
   */

  var range = createRange();

  var Control = /*#__PURE__*/function () {
    function Control(def) {
      _classCallCheck$1(this, Control);

      _defineProperty$1(this, "def", void 0);

      this.def = def;
    }

    _createClass$1(Control, [{
      key: "setValue",
      value: function setValue(value) {
        engine_1.setValue(this.def.group, this.def.name, value);
      }
    }, {
      key: "getValue",
      value: function getValue() {
        return engine_1.getValue(this.def.group, this.def.name);
      }
    }]);

    return Control;
  }();
  var playListControlDef = {
    LoadSelectedIntoFirstStopped: {
      group: '[Playlist]',
      name: 'LoadSelectedIntoFirstStopped',
      type: 'binary',
      description: 'Loads the currently highlighted song into the first stopped deck'
    },
    SelectNextPlaylist: {
      group: '[Playlist]',
      name: 'SelectNextPlaylist',
      type: 'binary',
      description: 'Switches to the next view (Library, Queue, etc.)'
    },
    SelectPrevPlaylist: {
      group: '[Playlist]',
      name: 'SelectPrevPlaylist',
      type: 'binary',
      description: 'Switches to the previous view (Library, Queue, etc.)'
    },
    ToggleSelectedSidebarItem: {
      group: '[Playlist]',
      name: 'ToggleSelectedSidebarItem',
      type: 'binary',
      description: 'Toggles (expands/collapses) the currently selected sidebar item.'
    },
    SelectNextTrack: {
      group: '[Playlist]',
      name: 'SelectNextTrack',
      type: 'binary',
      description: 'Scrolls to the next track in the track table.'
    },
    SelectPrevTrack: {
      group: '[Playlist]',
      name: 'SelectPrevTrack',
      type: 'binary',
      description: 'Scrolls to the previous track in the track table.'
    },
    SelectTrackKnob: {
      group: '[Playlist]',
      name: 'SelectTrackKnob',
      type: 'relative value',
      description: 'Scrolls the given number of tracks in the track table (can be negative for reverse direction).'
    },
    AutoDjAddBottom: {
      group: '[Playlist]',
      name: 'AutoDjAddBottom',
      type: 'binary',
      description: 'Add selected track(s) to Auto DJ Queue (bottom).'
    },
    AutoDjAddTop: {
      group: '[Playlist]',
      name: 'AutoDjAddTop',
      type: 'binary',
      description: 'Add selected track(s) to Auto DJ Queue (top).'
    }
  };
  var playListControl = Object.keys(playListControlDef).reduce(function (obj, key) {
    return assign(obj, _defineProperty$1({}, key, new Control(playListControlDef[key])));
  }, {});

  var channelDef = function channelDef(type, i) {
    return {
      back: {
        group: "[".concat(type).concat(i, "]"),
        name: 'back',
        type: 'binary'
      },
      beat_active: {
        group: "[".concat(type).concat(i, "]"),
        name: 'beat_active',
        type: 'binary'
      },
      beatjump: {
        group: "[".concat(type).concat(i, "]"),
        name: 'beatjump',
        type: 'real number'
      },
      beatjumps: function beatjumps(x) {
        return {
          forward: {
            group: "[".concat(type).concat(i, "]"),
            name: "beatjump_".concat(x, "_forward"),
            type: 'binary'
          },
          backward: {
            group: "[".concat(type).concat(i, "]"),
            name: "beatjump_".concat(x, "_backward"),
            type: 'binary'
          }
        };
      },
      beatloop: {
        group: "[".concat(type).concat(i, "]"),
        name: 'beatloop',
        type: 'positive real number'
      },
      beatloops: function beatloops(x) {
        return {
          activate: {
            group: "[".concat(type).concat(i, "]"),
            name: "beatloop_".concat(x, "_activate"),
            type: 'binary'
          },
          toggle: {
            group: "[".concat(type).concat(i, "]"),
            name: "beatloop_".concat(x, "_toggle"),
            type: 'binary'
          },
          enabled: {
            group: "[".concat(type).concat(i, "]"),
            name: "beatloop_".concat(x, "_enabled"),
            type: 'binary'
          }
        };
      },
      beats_adjust_faster: {
        group: "[".concat(type).concat(i, "]"),
        name: 'beats_adjust_faster',
        type: 'binary'
      },
      beats_adjust_slower: {
        group: "[".concat(type).concat(i, "]"),
        name: 'beats_adjust_slower',
        type: 'binary'
      },
      beats_translate_curpos: {
        group: "[".concat(type).concat(i, "]"),
        name: 'beats_translate_curpos',
        type: 'binary'
      },
      beats_translate_match_alignment: {
        group: "[".concat(type).concat(i, "]"),
        name: 'beats_translate_match_alignment',
        type: 'binary'
      },
      beats_translate_earlier: {
        group: "[".concat(type).concat(i, "]"),
        name: 'beats_translate_earlier',
        type: 'binary'
      },
      beats_translate_later: {
        group: "[".concat(type).concat(i, "]"),
        name: 'beats_translate_later',
        type: 'binary'
      },
      beatsync: {
        group: "[".concat(type).concat(i, "]"),
        name: 'beatsync',
        type: 'binary'
      },
      beatsync_phase: {
        group: "[".concat(type).concat(i, "]"),
        name: 'beatsync_phase',
        type: 'binary'
      },
      beatsync_tempo: {
        group: "[".concat(type).concat(i, "]"),
        name: 'beatsync_tempo',
        type: 'binary'
      },
      bpm: {
        group: "[".concat(type).concat(i, "]"),
        name: 'bpm',
        type: 'real-valued'
      },
      bpm_tap: {
        group: "[".concat(type).concat(i, "]"),
        name: 'bpm_tap',
        type: 'binary'
      },
      cue_default: {
        group: "[".concat(type).concat(i, "]"),
        name: 'cue_default',
        type: 'binary'
      },
      cue_gotoandplay: {
        group: "[".concat(type).concat(i, "]"),
        name: 'cue_gotoandplay',
        type: 'binary'
      },
      cue_gotoandstop: {
        group: "[".concat(type).concat(i, "]"),
        name: 'cue_gotoandstop',
        type: 'binary'
      },
      cue_indicator: {
        group: "[".concat(type).concat(i, "]"),
        name: 'cue_indicator',
        type: 'binary'
      },
      cue_cdj: {
        group: "[".concat(type).concat(i, "]"),
        name: 'cue_cdj',
        type: 'binary'
      },
      cue_play: {
        group: "[".concat(type).concat(i, "]"),
        name: 'cue_play',
        type: 'binary'
      },
      cue_point: {
        group: "[".concat(type).concat(i, "]"),
        name: 'cue_point',
        type: 'absolute value'
      },
      cue_preview: {
        group: "[".concat(type).concat(i, "]"),
        name: 'cue_preview',
        type: 'binary'
      },
      cue_set: {
        group: "[".concat(type).concat(i, "]"),
        name: 'cue_set',
        type: 'binary'
      },
      cue_simple: {
        group: "[".concat(type).concat(i, "]"),
        name: 'cue_simple',
        type: 'binary'
      },
      duration: {
        group: "[".concat(type).concat(i, "]"),
        name: 'duration',
        type: 'absolute value'
      },
      eject: {
        group: "[".concat(type).concat(i, "]"),
        name: 'eject',
        type: 'binary'
      },
      end: {
        group: "[".concat(type).concat(i, "]"),
        name: 'end',
        type: 'binary'
      },
      file_bpm: {
        group: "[".concat(type).concat(i, "]"),
        name: 'file_bpm',
        type: 'positive value'
      },
      file_key: {
        group: "[".concat(type).concat(i, "]"),
        name: 'file_key',
        type: '?'
      },
      fwd: {
        group: "[".concat(type).concat(i, "]"),
        name: 'fwd',
        type: 'binary'
      },
      hotcues: function hotcues(x) {
        return {
          activate: {
            group: "[".concat(type).concat(i, "]"),
            name: "hotcue_".concat(x, "_activate"),
            type: 'binary'
          },
          clear: {
            group: "[".concat(type).concat(i, "]"),
            name: "hotcue_".concat(x, "_clear"),
            type: 'binary'
          },
          enabled: {
            group: "[".concat(type).concat(i, "]"),
            name: "hotcue_".concat(x, "_enabled"),
            type: 'read-only, binary'
          },
          "goto": {
            group: "[".concat(type).concat(i, "]"),
            name: "hotcue_".concat(x, "_goto"),
            type: 'binary'
          },
          gotoandplay: {
            group: "[".concat(type).concat(i, "]"),
            name: "hotcue_".concat(x, "_gotoandplay"),
            type: 'binary'
          },
          gotoandstop: {
            group: "[".concat(type).concat(i, "]"),
            name: "hotcue_".concat(x, "_gotoandstop"),
            type: 'binary'
          },
          position: {
            group: "[".concat(type).concat(i, "]"),
            name: "hotcue_".concat(x, "_position"),
            type: 'positive integer'
          },
          set: {
            group: "[".concat(type).concat(i, "]"),
            name: "hotcue_".concat(x, "_set"),
            type: 'binary'
          }
        };
      },
      key: {
        group: "[".concat(type).concat(i, "]"),
        name: 'key',
        type: 'real-valued'
      },
      keylock: {
        group: "[".concat(type).concat(i, "]"),
        name: 'keylock',
        type: 'binary'
      },
      LoadSelectedTrack: {
        group: "[".concat(type).concat(i, "]"),
        name: 'LoadSelectedTrack',
        type: 'binary'
      },
      LoadSelectedTrackAndPlay: {
        group: "[".concat(type).concat(i, "]"),
        name: 'LoadSelectedTrackAndPlay',
        type: 'binary'
      },
      loop_double: {
        group: "[".concat(type).concat(i, "]"),
        name: 'loop_double',
        type: 'binary'
      },
      loop_enabled: {
        group: "[".concat(type).concat(i, "]"),
        name: 'loop_enabled',
        type: 'read-only, binary'
      },
      loop_end_position: {
        group: "[".concat(type).concat(i, "]"),
        name: 'loop_end_position',
        type: 'positive integer'
      },
      loop_halve: {
        group: "[".concat(type).concat(i, "]"),
        name: 'loop_halve',
        type: 'binary'
      },
      loop_in: {
        group: "[".concat(type).concat(i, "]"),
        name: 'loop_in',
        type: 'binary'
      },
      loop_out: {
        group: "[".concat(type).concat(i, "]"),
        name: 'loop_out',
        type: 'binary'
      },
      loop_move: {
        group: "[".concat(type).concat(i, "]"),
        name: 'loop_move',
        type: 'real number'
      },
      loop_scale: {
        group: "[".concat(type).concat(i, "]"),
        name: 'loop_scale',
        type: '0.0 - infinity'
      },
      loop_start_position: {
        group: "[".concat(type).concat(i, "]"),
        name: 'loop_start_position',
        type: 'positive integer'
      },
      orientation: {
        group: "[".concat(type).concat(i, "]"),
        name: 'orientation',
        type: '0-2'
      },
      passthrough: {
        group: "[".concat(type).concat(i, "]"),
        name: 'passthrough',
        type: 'binary'
      },
      PeakIndicator: {
        group: "[".concat(type).concat(i, "]"),
        name: 'PeakIndicator',
        type: 'binary'
      },
      pfl: {
        group: "[".concat(type).concat(i, "]"),
        name: 'pfl',
        type: 'binary'
      },
      pitch: {
        group: "[".concat(type).concat(i, "]"),
        name: 'pitch',
        type: '-6.0..6.0'
      },
      pitch_adjust: {
        group: "[".concat(type).concat(i, "]"),
        name: 'pitch_adjust',
        type: '-3.0..3.0'
      },
      play: {
        group: "[".concat(type).concat(i, "]"),
        name: 'play',
        type: 'binary'
      },
      play_indicator: {
        group: "[".concat(type).concat(i, "]"),
        name: 'play_indicator',
        type: 'binary'
      },
      play_stutter: {
        group: "[".concat(type).concat(i, "]"),
        name: 'play_stutter',
        type: 'binary'
      },
      playposition: {
        group: "[".concat(type).concat(i, "]"),
        name: 'playposition',
        type: 'default'
      },
      pregain: {
        group: "[".concat(type).concat(i, "]"),
        name: 'pregain',
        type: '0.0..1.0..4.0'
      },
      quantize: {
        group: "[".concat(type).concat(i, "]"),
        name: 'quantize',
        type: 'binary'
      },
      rate: {
        group: "[".concat(type).concat(i, "]"),
        name: 'rate',
        type: '-1.0..1.0'
      },
      rate_dir: {
        group: "[".concat(type).concat(i, "]"),
        name: 'rate_dir',
        type: '-1 or 1'
      },
      rate_perm_down: {
        group: "[".concat(type).concat(i, "]"),
        name: 'rate_perm_down',
        type: 'binary'
      },
      rate_perm_down_small: {
        group: "[".concat(type).concat(i, "]"),
        name: 'rate_perm_down_small',
        type: 'binary'
      },
      rate_perm_up: {
        group: "[".concat(type).concat(i, "]"),
        name: 'rate_perm_up',
        type: 'binary'
      },
      rate_perm_up_small: {
        group: "[".concat(type).concat(i, "]"),
        name: 'rate_perm_up_small',
        type: 'binary'
      },
      rate_temp_down: {
        group: "[".concat(type).concat(i, "]"),
        name: 'rate_temp_down',
        type: 'binary'
      },
      rate_temp_down_small: {
        group: "[".concat(type).concat(i, "]"),
        name: 'rate_temp_down_small',
        type: 'binary'
      },
      rate_temp_up: {
        group: "[".concat(type).concat(i, "]"),
        name: 'rate_temp_up',
        type: 'binary'
      },
      rate_temp_up_small: {
        group: "[".concat(type).concat(i, "]"),
        name: 'rate_temp_up_small',
        type: 'binary'
      },
      rateRange: {
        group: "[".concat(type).concat(i, "]"),
        name: 'rateRange',
        type: '0.0..3.0'
      },
      reloop_andstop: {
        group: "[".concat(type).concat(i, "]"),
        name: 'reloop_andstop',
        type: 'binary'
      },
      reloop_exit: {
        group: "[".concat(type).concat(i, "]"),
        name: 'reloop_exit',
        type: 'binary'
      },
      repeat: {
        group: "[".concat(type).concat(i, "]"),
        name: 'repeat',
        type: 'binary'
      },
      reset_key: {
        group: "[".concat(type).concat(i, "]"),
        name: 'reset_key',
        type: 'binary'
      },
      reverse: {
        group: "[".concat(type).concat(i, "]"),
        name: 'reverse',
        type: 'binary'
      },
      reverseroll: {
        group: "[".concat(type).concat(i, "]"),
        name: 'reverseroll',
        type: 'binary'
      },
      slip_enabled: {
        group: "[".concat(type).concat(i, "]"),
        name: 'slip_enabled',
        type: 'binary'
      },
      start: {
        group: "[".concat(type).concat(i, "]"),
        name: 'start',
        type: 'binary'
      },
      start_play: {
        group: "[".concat(type).concat(i, "]"),
        name: 'start_play',
        type: 'binary'
      },
      start_stop: {
        group: "[".concat(type).concat(i, "]"),
        name: 'start_stop',
        type: 'binary'
      },
      stop: {
        group: "[".concat(type).concat(i, "]"),
        name: 'stop',
        type: 'binary'
      },
      sync_enabled: {
        group: "[".concat(type).concat(i, "]"),
        name: 'sync_enabled',
        type: 'binary'
      },
      sync_leader: {
        group: "[".concat(type).concat(i, "]"),
        name: 'sync_leader',
        type: 'binary'
      },
      sync_mode: {
        group: "[".concat(type).concat(i, "]"),
        name: 'sync_mode',
        type: 'binary'
      },
      sync_key: {
        group: "[".concat(type).concat(i, "]"),
        name: 'sync_key',
        type: '?'
      },
      track_samplerate: {
        group: "[".concat(type).concat(i, "]"),
        name: 'track_samplerate',
        type: 'absolute value'
      },
      track_samples: {
        group: "[".concat(type).concat(i, "]"),
        name: 'track_samples',
        type: 'absolute value'
      },
      volume: {
        group: "[".concat(type).concat(i, "]"),
        name: 'volume',
        type: 'default'
      },
      mute: {
        group: "[".concat(type).concat(i, "]"),
        name: 'mute',
        type: 'binary'
      },
      vinylcontrol_enabled: {
        group: "[".concat(type).concat(i, "]"),
        name: 'vinylcontrol_enabled',
        type: 'binary'
      },
      vinylcontrol_cueing: {
        group: "[".concat(type).concat(i, "]"),
        name: 'vinylcontrol_cueing',
        type: '0.0-2.0'
      },
      vinylcontrol_mode: {
        group: "[".concat(type).concat(i, "]"),
        name: 'vinylcontrol_mode',
        type: '0.0-2.0'
      },
      vinylcontrol_status: {
        group: "[".concat(type).concat(i, "]"),
        name: 'vinylcontrol_status',
        type: '0.0-3.0 (read-only)'
      },
      visual_bpm: {
        group: "[".concat(type).concat(i, "]"),
        name: 'visual_bpm',
        type: '?'
      },
      visual_key: {
        group: "[".concat(type).concat(i, "]"),
        name: 'visual_key',
        type: '?'
      },
      visual_key_distance: {
        group: "[".concat(type).concat(i, "]"),
        name: 'visual_key_distance',
        type: '-0.5..0.5'
      },
      VuMeter: {
        group: "[".concat(type).concat(i, "]"),
        name: 'VuMeter',
        type: 'default'
      },
      VuMeterL: {
        group: "[".concat(type).concat(i, "]"),
        name: 'VuMeterL',
        type: 'default'
      },
      VuMeterR: {
        group: "[".concat(type).concat(i, "]"),
        name: 'VuMeterR',
        type: 'default'
      },
      waveform_zoom: {
        group: "[".concat(type).concat(i, "]"),
        name: 'waveform_zoom',
        type: '1.0 - 6.0'
      },
      waveform_zoom_up: {
        group: "[".concat(type).concat(i, "]"),
        name: 'waveform_zoom_up',
        type: '?'
      },
      waveform_zoom_down: {
        group: "[".concat(type).concat(i, "]"),
        name: 'waveform_zoom_down',
        type: '?'
      },
      waveform_zoom_set_default: {
        group: "[".concat(type).concat(i, "]"),
        name: 'waveform_zoom_set_default',
        type: '?'
      },
      wheel: {
        group: "[".concat(type).concat(i, "]"),
        name: 'wheel',
        type: '-3.0..3.0'
      }
    };
  };

  var beatjumps = [0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64];
  var beatloops = beatjumps;

  var createEnumeratedControl = function createEnumeratedControl(array, one) {
    return array.reduce(function (arr, i) {
      var def = one(i);
      var control = Object.keys(def).reduce(function (obj, key) {
        return assign(obj, _defineProperty$1({}, key, new Control(def[key])));
      }, {});
      return assign(arr, _defineProperty$1({}, i, control));
    }, {});
  };

  var createChannelControl = function createChannelControl(i) {
    var _ref = i < 5 ? ['Channel', i] : ['Sampler', i - 4],
        _ref2 = _slicedToArray(_ref, 2),
        name = _ref2[0],
        number = _ref2[1];

    var channelDefInstance = channelDef(name, number);
    var channel = Object.keys(channelDefInstance).filter(function (key) {
      return key !== 'beatjumps' && key !== 'beatloops' && key !== 'hotcues';
    }).reduce(function (obj, key) {
      return assign(obj, _defineProperty$1({}, key, new Control(channelDefInstance[key])));
    }, {});
    return assign(channel, {
      beatjumps: createEnumeratedControl(beatjumps, channelDefInstance.beatjumps),
      beatloops: createEnumeratedControl(beatloops, channelDefInstance.beatloops),
      hotcues: createEnumeratedControl(range(16).map(function (x) {
        return x + 1;
      }), channelDefInstance.hotcues)
    });
  };
  var channelControls = range(8).map(function (i) {
    return createChannelControl(i + 1);
  });

  var callbackPrefix = '__ctrl';

  var sanitize = function sanitize(name) {
    return name.replace('.', '$dot$').replace('[', '$sbs$').replace(']', '$sbe$');
  };

  var ControlBus = /*#__PURE__*/function () {
    _createClass$1(ControlBus, null, [{
      key: "create",
      value: function create(moduleName, registry) {
        return new ControlBus(moduleName, registry);
      }
    }]);

    function ControlBus(registryName, registry) {
      _classCallCheck$1(this, ControlBus);

      _defineProperty$1(this, "_registryName", void 0);

      _defineProperty$1(this, "_registry", void 0);

      _defineProperty$1(this, "_callbackList", void 0);

      this._registryName = registryName;
      this._registry = registry;
      this._callbackList = {};
    }

    _createClass$1(ControlBus, [{
      key: "connect",
      value: function connect(id, control, cb) {
        var _this = this;

        var group = control.group,
            name = control.name;
        var key = "".concat(sanitize(group), "_").concat(sanitize(name));
        var engineCb = "".concat(callbackPrefix, "_").concat(key);

        if (!this._callbackList[key]) {
          this._callbackList[key] = {};
        }

        this._callbackList[key][id] = cb;

        if (!this._registry[engineCb]) {
          this._registry[engineCb] = function (value) {
            for (var _id in _this._callbackList[key]) {
              _this._callbackList[key][_id]({
                value: value,
                control: control,
                id: _id
              });
            }
          };

          engine_1.connectControl(group, name, "".concat(this._registryName, ".").concat(engineCb));
        }

        return {
          id: id,
          group: group,
          name: name,
          key: key
        };
      }
    }, {
      key: "disconnect",
      value: function disconnect(handle) {
        var id = handle.id,
            group = handle.group,
            name = handle.name,
            key = handle.key;
        var engineCb = "".concat(callbackPrefix, "_").concat(key);

        if (this._callbackList[key] && this._callbackList[key][id]) {
          delete this._callbackList[key][id];
        }

        if (!Object.keys(this._callbackList[key]).length && this._registry[engineCb]) {
          engine_1.connectControl(group, name, "".concat(this._registryName, ".").concat(engineCb), true);
          delete this._callbackList[key];
          delete this._registry[engineCb];
        }
      }
    }]);

    return ControlBus;
  }();

  var timerPrefix = '__timer';
  var Timer = /*#__PURE__*/function () {
    function Timer(registryName, registry, task) {
      _classCallCheck$1(this, Timer);

      _defineProperty$1(this, "task", void 0);

      _defineProperty$1(this, "_state", void 0);

      _defineProperty$1(this, "_registryName", void 0);

      _defineProperty$1(this, "_registry", void 0);

      this._registryName = registryName;
      this._registry = registry;
      this.task = task;
      this._state = undefined;
    }

    _createClass$1(Timer, [{
      key: "start",
      value: function start(interval) {
        if (this._state == null) {
          var started = Date.now();
          var key = "".concat(timerPrefix, "_").concat(started, "_").concat(parseInt(Math.random() * 100));
          var handle = engine_1.beginTimer(interval, "".concat(this._registryName, ".").concat(key));
          this._state = {
            handle: handle,
            key: key,
            started: started
          };
          this._registry[key] = this.task;
          return started;
        }
      }
    }, {
      key: "end",
      value: function end() {
        var state = this._state;

        if (state != null) {
          engine_1.stopTimer(state.handle);
          delete this._registry[state.key];
          this._state = undefined;
        }
      }
    }, {
      key: "restart",
      value: function restart(interval) {
        if (this._state != null) {
          this.end();
          return this.start(interval);
        }
      }
    }, {
      key: "getStartTime",
      value: function getStartTime() {
        return this._state && this._state.started;
      }
    }]);

    return Timer;
  }();
  var makeTimer = function makeTimer(moduleName, registry) {
    return function (task) {
      return new Timer(moduleName, registry, task);
    };
  };

  function createCommonjsModule(fn, module) {
  	return module = { exports: {} }, fn(module, module.exports), module.exports;
  }

  var stringify_1 = createCommonjsModule(function (module, exports) {
  exports = module.exports = stringify;
  exports.getSerialize = serializer;

  function stringify(obj, replacer, spaces, cycleReplacer) {
    return JSON.stringify(obj, serializer(replacer, cycleReplacer), spaces);
  }

  function serializer(replacer, cycleReplacer) {
    var stack = [],
        keys = [];
    if (cycleReplacer == null) cycleReplacer = function cycleReplacer(key, value) {
      if (stack[0] === value) return "[Circular ~]";
      return "[Circular ~." + keys.slice(0, stack.indexOf(value)).join(".") + "]";
    };
    return function (key, value) {
      if (stack.length > 0) {
        var thisPos = stack.indexOf(this);
        ~thisPos ? stack.splice(thisPos + 1) : stack.push(this);
        ~thisPos ? keys.splice(thisPos, Infinity, key) : keys.push(key);
        if (~stack.indexOf(value)) value = cycleReplacer.call(this, key, value);
      } else stack.push(value);

      return replacer == null ? value : replacer.call(this, key, value);
    };
  }
  });
  var stringify_2 = stringify_1.getSerialize;

  var eventemitter3 = createCommonjsModule(function (module) {

  var has = Object.prototype.hasOwnProperty,
      prefix = '~';
  /**
   * Constructor to create a storage for our `EE` objects.
   * An `Events` instance is a plain object whose properties are event names.
   *
   * @constructor
   * @private
   */

  function Events() {} //
  // We try to not inherit from `Object.prototype`. In some engines creating an
  // instance in this way is faster than calling `Object.create(null)` directly.
  // If `Object.create(null)` is not supported we prefix the event names with a
  // character to make sure that the built-in object properties are not
  // overridden or used as an attack vector.
  //


  if (Object.create) {
    Events.prototype = Object.create(null); //
    // This hack is needed because the `__proto__` property is still inherited in
    // some old browsers like Android 4, iPhone 5.1, Opera 11 and Safari 5.
    //

    if (!new Events().__proto__) prefix = false;
  }
  /**
   * Representation of a single event listener.
   *
   * @param {Function} fn The listener function.
   * @param {*} context The context to invoke the listener with.
   * @param {Boolean} [once=false] Specify if the listener is a one-time listener.
   * @constructor
   * @private
   */


  function EE(fn, context, once) {
    this.fn = fn;
    this.context = context;
    this.once = once || false;
  }
  /**
   * Add a listener for a given event.
   *
   * @param {EventEmitter} emitter Reference to the `EventEmitter` instance.
   * @param {(String|Symbol)} event The event name.
   * @param {Function} fn The listener function.
   * @param {*} context The context to invoke the listener with.
   * @param {Boolean} once Specify if the listener is a one-time listener.
   * @returns {EventEmitter}
   * @private
   */


  function addListener(emitter, event, fn, context, once) {
    if (typeof fn !== 'function') {
      throw new TypeError('The listener must be a function');
    }

    var listener = new EE(fn, context || emitter, once),
        evt = prefix ? prefix + event : event;
    if (!emitter._events[evt]) emitter._events[evt] = listener, emitter._eventsCount++;else if (!emitter._events[evt].fn) emitter._events[evt].push(listener);else emitter._events[evt] = [emitter._events[evt], listener];
    return emitter;
  }
  /**
   * Clear event by name.
   *
   * @param {EventEmitter} emitter Reference to the `EventEmitter` instance.
   * @param {(String|Symbol)} evt The Event name.
   * @private
   */


  function clearEvent(emitter, evt) {
    if (--emitter._eventsCount === 0) emitter._events = new Events();else delete emitter._events[evt];
  }
  /**
   * Minimal `EventEmitter` interface that is molded against the Node.js
   * `EventEmitter` interface.
   *
   * @constructor
   * @public
   */


  function EventEmitter() {
    this._events = new Events();
    this._eventsCount = 0;
  }
  /**
   * Return an array listing the events for which the emitter has registered
   * listeners.
   *
   * @returns {Array}
   * @public
   */


  EventEmitter.prototype.eventNames = function eventNames() {
    var names = [],
        events,
        name;
    if (this._eventsCount === 0) return names;

    for (name in events = this._events) {
      if (has.call(events, name)) names.push(prefix ? name.slice(1) : name);
    }

    if (Object.getOwnPropertySymbols) {
      return names.concat(Object.getOwnPropertySymbols(events));
    }

    return names;
  };
  /**
   * Return the listeners registered for a given event.
   *
   * @param {(String|Symbol)} event The event name.
   * @returns {Array} The registered listeners.
   * @public
   */


  EventEmitter.prototype.listeners = function listeners(event) {
    var evt = prefix ? prefix + event : event,
        handlers = this._events[evt];
    if (!handlers) return [];
    if (handlers.fn) return [handlers.fn];

    for (var i = 0, l = handlers.length, ee = new Array(l); i < l; i++) {
      ee[i] = handlers[i].fn;
    }

    return ee;
  };
  /**
   * Return the number of listeners listening to a given event.
   *
   * @param {(String|Symbol)} event The event name.
   * @returns {Number} The number of listeners.
   * @public
   */


  EventEmitter.prototype.listenerCount = function listenerCount(event) {
    var evt = prefix ? prefix + event : event,
        listeners = this._events[evt];
    if (!listeners) return 0;
    if (listeners.fn) return 1;
    return listeners.length;
  };
  /**
   * Calls each of the listeners registered for a given event.
   *
   * @param {(String|Symbol)} event The event name.
   * @returns {Boolean} `true` if the event had listeners, else `false`.
   * @public
   */


  EventEmitter.prototype.emit = function emit(event, a1, a2, a3, a4, a5) {
    var evt = prefix ? prefix + event : event;
    if (!this._events[evt]) return false;
    var listeners = this._events[evt],
        len = arguments.length,
        args,
        i;

    if (listeners.fn) {
      if (listeners.once) this.removeListener(event, listeners.fn, undefined, true);

      switch (len) {
        case 1:
          return listeners.fn.call(listeners.context), true;

        case 2:
          return listeners.fn.call(listeners.context, a1), true;

        case 3:
          return listeners.fn.call(listeners.context, a1, a2), true;

        case 4:
          return listeners.fn.call(listeners.context, a1, a2, a3), true;

        case 5:
          return listeners.fn.call(listeners.context, a1, a2, a3, a4), true;

        case 6:
          return listeners.fn.call(listeners.context, a1, a2, a3, a4, a5), true;
      }

      for (i = 1, args = new Array(len - 1); i < len; i++) {
        args[i - 1] = arguments[i];
      }

      listeners.fn.apply(listeners.context, args);
    } else {
      var length = listeners.length,
          j;

      for (i = 0; i < length; i++) {
        if (listeners[i].once) this.removeListener(event, listeners[i].fn, undefined, true);

        switch (len) {
          case 1:
            listeners[i].fn.call(listeners[i].context);
            break;

          case 2:
            listeners[i].fn.call(listeners[i].context, a1);
            break;

          case 3:
            listeners[i].fn.call(listeners[i].context, a1, a2);
            break;

          case 4:
            listeners[i].fn.call(listeners[i].context, a1, a2, a3);
            break;

          default:
            if (!args) for (j = 1, args = new Array(len - 1); j < len; j++) {
              args[j - 1] = arguments[j];
            }
            listeners[i].fn.apply(listeners[i].context, args);
        }
      }
    }

    return true;
  };
  /**
   * Add a listener for a given event.
   *
   * @param {(String|Symbol)} event The event name.
   * @param {Function} fn The listener function.
   * @param {*} [context=this] The context to invoke the listener with.
   * @returns {EventEmitter} `this`.
   * @public
   */


  EventEmitter.prototype.on = function on(event, fn, context) {
    return addListener(this, event, fn, context, false);
  };
  /**
   * Add a one-time listener for a given event.
   *
   * @param {(String|Symbol)} event The event name.
   * @param {Function} fn The listener function.
   * @param {*} [context=this] The context to invoke the listener with.
   * @returns {EventEmitter} `this`.
   * @public
   */


  EventEmitter.prototype.once = function once(event, fn, context) {
    return addListener(this, event, fn, context, true);
  };
  /**
   * Remove the listeners of a given event.
   *
   * @param {(String|Symbol)} event The event name.
   * @param {Function} fn Only remove the listeners that match this function.
   * @param {*} context Only remove the listeners that have this context.
   * @param {Boolean} once Only remove one-time listeners.
   * @returns {EventEmitter} `this`.
   * @public
   */


  EventEmitter.prototype.removeListener = function removeListener(event, fn, context, once) {
    var evt = prefix ? prefix + event : event;
    if (!this._events[evt]) return this;

    if (!fn) {
      clearEvent(this, evt);
      return this;
    }

    var listeners = this._events[evt];

    if (listeners.fn) {
      if (listeners.fn === fn && (!once || listeners.once) && (!context || listeners.context === context)) {
        clearEvent(this, evt);
      }
    } else {
      for (var i = 0, events = [], length = listeners.length; i < length; i++) {
        if (listeners[i].fn !== fn || once && !listeners[i].once || context && listeners[i].context !== context) {
          events.push(listeners[i]);
        }
      } //
      // Reset the array, or remove it completely if we have no more listeners.
      //


      if (events.length) this._events[evt] = events.length === 1 ? events[0] : events;else clearEvent(this, evt);
    }

    return this;
  };
  /**
   * Remove all listeners, or those of the specified event.
   *
   * @param {(String|Symbol)} [event] The event name.
   * @returns {EventEmitter} `this`.
   * @public
   */


  EventEmitter.prototype.removeAllListeners = function removeAllListeners(event) {
    var evt;

    if (event) {
      evt = prefix ? prefix + event : event;
      if (this._events[evt]) clearEvent(this, evt);
    } else {
      this._events = new Events();
      this._eventsCount = 0;
    }

    return this;
  }; //
  // Alias methods names because people roll like that.
  //


  EventEmitter.prototype.off = EventEmitter.prototype.removeListener;
  EventEmitter.prototype.addListener = EventEmitter.prototype.on; //
  // Expose the prefix.
  //

  EventEmitter.prefixed = prefix; //
  // Allow `EventEmitter` to be imported as module namespace.
  //

  EventEmitter.EventEmitter = EventEmitter; //
  // Expose the module.
  //

  {
    module.exports = EventEmitter;
  }
  });

  function _createSuper(Derived) { return function () { var Super = _getPrototypeOf(Derived), result; if (_isNativeReflectConstruct()) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

  function _isNativeReflectConstruct() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Date.prototype.toString.call(Reflect.construct(Date, [], function () {})); return true; } catch (e) { return false; } }
  var callbackPrefix$1 = '__midi';

  var leftPad = function leftPad(str, padString, length) {
    var buf = str;

    while (buf.length < length) {
      buf = padString + buf;
    }

    return buf;
  };

  var hexFormat = function hexFormat(n, d) {
    return '0x' + leftPad(n.toString(16).toUpperCase(), '0', d);
  };

  var MidiBus = /*#__PURE__*/function (_EventEmitter) {
    _inherits(MidiBus, _EventEmitter);

    var _super = _createSuper(MidiBus);

    _createClass$1(MidiBus, null, [{
      key: "create",
      value: function create(registry, device) {
        return new MidiBus(registry, device);
      }
    }]);

    function MidiBus(registry, device) {
      var _this;

      _classCallCheck$1(this, MidiBus);

      _this = _super.call(this);

      _defineProperty$1(_assertThisInitialized(_this), "registry", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "device", void 0);

      _this.registry = registry;
      _this.device = device;
      Object.keys(device.buttons).forEach(function (buttonName) {
        var button = device.buttons[buttonName];
        var def = button.def;

        _this.registry["".concat(callbackPrefix$1, "_").concat(hexFormat(def.status, 2), "_").concat(hexFormat(def.midino, 2))] = function (channel, control, value, status) {
          var message = {
            value: value,
            button: button,
            device: _this.device
          };

          _this.emit(def.name, message);
        };
      });
      return _this;
    }

    return MidiBus;
  }(eventemitter3);

  function _superPropBase(object, property) {
    while (!Object.prototype.hasOwnProperty.call(object, property)) {
      object = _getPrototypeOf(object);
      if (object === null) break;
    }

    return object;
  }

  function _get(target, property, receiver) {
    if (typeof Reflect !== "undefined" && Reflect.get) {
      _get = Reflect.get;
    } else {
      _get = function _get(target, property, receiver) {
        var base = _superPropBase(target, property);
        if (!base) return;
        var desc = Object.getOwnPropertyDescriptor(base, property);

        if (desc.get) {
          return desc.get.call(receiver);
        }

        return desc.value;
      };
    }

    return _get(target, property, receiver || target);
  }

  function _createSuper$1(Derived) { return function () { var Super = _getPrototypeOf(Derived), result; if (_isNativeReflectConstruct$1()) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

  function _isNativeReflectConstruct$1() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Date.prototype.toString.call(Reflect.construct(Date, [], function () {})); return true; } catch (e) { return false; } }

  var Component = /*#__PURE__*/function (_EventEmitter) {
    _inherits(Component, _EventEmitter);

    var _super = _createSuper$1(Component);

    function Component() {
      _classCallCheck$1(this, Component);

      return _super.apply(this, arguments);
    }

    _createClass$1(Component, [{
      key: "mount",
      value: function mount() {
        this.onMount();
        this.emit('mount', this);
      }
    }, {
      key: "unmount",
      value: function unmount() {
        this.onUnmount();
        this.emit('unmount', this);
      }
    }, {
      key: "onMount",
      value: function onMount() {}
    }, {
      key: "onUnmount",
      value: function onUnmount() {}
    }]);

    return Component;
  }(eventemitter3);

  function _createSuper$2(Derived) { return function () { var Super = _getPrototypeOf(Derived), result; if (_isNativeReflectConstruct$2()) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

  function _isNativeReflectConstruct$2() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Date.prototype.toString.call(Reflect.construct(Date, [], function () {})); return true; } catch (e) { return false; } }

  var MidiComponent = /*#__PURE__*/function (_Component) {
    _inherits(MidiComponent, _Component);

    var _super = _createSuper$2(MidiComponent);

    function MidiComponent(midibus) {
      var _this;

      _classCallCheck$1(this, MidiComponent);

      _this = _super.call(this);

      _defineProperty$1(_assertThisInitialized(_this), "midibus", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "device", void 0);

      _this.midibus = midibus;
      _this.device = midibus.device;
      return _this;
    }

    _createClass$1(MidiComponent, [{
      key: "onMount",
      value: function onMount() {
        _get(_getPrototypeOf(MidiComponent.prototype), "onMount", this).call(this);
      }
    }, {
      key: "onUnmount",
      value: function onUnmount() {
        _get(_getPrototypeOf(MidiComponent.prototype), "onUnmount", this).call(this);
      }
    }]);

    return MidiComponent;
  }(Component);

  function _createSuper$3(Derived) { return function () { var Super = _getPrototypeOf(Derived), result; if (_isNativeReflectConstruct$3()) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

  function _isNativeReflectConstruct$3() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Date.prototype.toString.call(Reflect.construct(Date, [], function () {})); return true; } catch (e) { return false; } }

  var MidiButtonComponent = /*#__PURE__*/function (_MidiComponent) {
    _inherits(MidiButtonComponent, _MidiComponent);

    var _super = _createSuper$3(MidiButtonComponent);

    function MidiButtonComponent(midibus, button) {
      var _this;

      _classCallCheck$1(this, MidiButtonComponent);

      _this = _super.call(this, midibus);

      _defineProperty$1(_assertThisInitialized(_this), "button", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "_cb", void 0);

      _this.midibus = midibus;
      _this.button = button;
      _this.device = midibus.device;

      _this._cb = function (data) {
        if (data.value) {
          _this.emit('attack', data);
        } else {
          _this.emit('release', data);
        }

        _this.emit('midi', data);
      };

      return _this;
    }

    _createClass$1(MidiButtonComponent, [{
      key: "onMount",
      value: function onMount() {
        _get(_getPrototypeOf(MidiButtonComponent.prototype), "onMount", this).call(this);

        this.midibus.on(this.button.def.name, this._cb);
      }
    }, {
      key: "onUnmount",
      value: function onUnmount() {
        this.midibus.removeListener(this.button.def.name, this._cb);

        _get(_getPrototypeOf(MidiButtonComponent.prototype), "onUnmount", this).call(this);
      }
    }]);

    return MidiButtonComponent;
  }(MidiComponent);

  function _createSuper$4(Derived) { return function () { var Super = _getPrototypeOf(Derived), result; if (_isNativeReflectConstruct$4()) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

  function _isNativeReflectConstruct$4() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Date.prototype.toString.call(Reflect.construct(Date, [], function () {})); return true; } catch (e) { return false; } }

  var autoscrolled = function autoscrolled(binding) {
    return function (timerBuilder) {
      var started;
      var minInterval = 32;
      var interval;
      var timer;
      binding.on('midi', function (data) {
        if (data.value) {
          interval = 250;
          started = timer.start(interval);
        } else {
          timer.end();
        }
      });
      binding.on('mount', function () {
        timer = timerBuilder(function () {
          binding.emit('scroll');

          if (interval > minInterval) {
            var current = Date.now(); // silence Flow with unsafe casts

            if (interval === 250 && current - started > 1500) {
              interval = 125;
              timer.restart(interval);
            } else if (interval === 125 && current - started > 3000) {
              interval = 63;
              timer.restart(interval);
            } else if (interval === 63 && current - started > 6000) {
              interval = minInterval;
              timer.restart(interval);
            }
          }
        });
      });
      binding.on('unmount', function () {
        timer.end();
      });
      return binding;
    };
  };

  var onScroll = function onScroll(control) {
    return function () {
      control.setValue(1);
    };
  };

  var onMidi = function onMidi(control) {
    return function (_ref) {
      var value = _ref.value,
          button = _ref.button,
          device = _ref.device;

      if (value) {
        control.setValue(1);
        button.sendColor(device.colors.hi_red);
      } else {
        button.sendColor(device.colors.hi_yellow);
      }
    };
  };

  var onMount = function onMount(_ref2) {
    var button = _ref2.button,
        device = _ref2.device;
    button.sendColor(device.colors.hi_yellow);
  };

  var onUnmount = function onUnmount(_ref3) {
    var button = _ref3.button,
        device = _ref3.device;
    button.sendColor(device.colors.black);
  };

  var PlaylistSidebar = /*#__PURE__*/function (_MidiComponent) {
    _inherits(PlaylistSidebar, _MidiComponent);

    var _super = _createSuper$4(PlaylistSidebar);

    function PlaylistSidebar(midibus, timerBuilder) {
      var _this;

      _classCallCheck$1(this, PlaylistSidebar);

      _this = _super.call(this, midibus);

      _defineProperty$1(_assertThisInitialized(_this), "buttons", void 0);

      var btns = [new MidiButtonComponent(midibus, _this.device.buttons.vol), new MidiButtonComponent(midibus, _this.device.buttons.pan), new MidiButtonComponent(midibus, _this.device.buttons.snda), new MidiButtonComponent(midibus, _this.device.buttons.sndb), new MidiButtonComponent(midibus, _this.device.buttons.stop)];
      var prevPlaylist = autoscrolled(btns[0])(timerBuilder);
      var nextPlaylist = autoscrolled(btns[1])(timerBuilder);
      var toggleItem = btns[2];
      var prevTrack = autoscrolled(btns[3])(timerBuilder);
      var nextTrack = autoscrolled(btns[4])(timerBuilder);
      prevPlaylist.on('scroll', onScroll(playListControl.SelectPrevPlaylist));
      prevPlaylist.on('midi', onMidi(playListControl.SelectPrevPlaylist));
      prevPlaylist.on('mount', onMount);
      prevPlaylist.on('unmount', onUnmount);
      nextPlaylist.on('scroll', onScroll(playListControl.SelectNextPlaylist));
      nextPlaylist.on('midi', onMidi(playListControl.SelectNextPlaylist));
      nextPlaylist.on('mount', onMount);
      nextPlaylist.on('unmount', onUnmount);
      prevTrack.on('scroll', onScroll(playListControl.SelectPrevTrack));
      prevTrack.on('midi', onMidi(playListControl.SelectPrevTrack));
      prevTrack.on('mount', onMount);
      prevTrack.on('unmount', onUnmount);
      nextTrack.on('scroll', onScroll(playListControl.SelectNextTrack));
      nextTrack.on('midi', onMidi(playListControl.SelectNextTrack));
      nextTrack.on('mount', onMount);
      nextTrack.on('unmount', onUnmount);
      toggleItem.on('midi', onMidi(playListControl.ToggleSelectedSidebarItem));
      toggleItem.on('mount', onMount);
      toggleItem.on('unmount', onUnmount);
      _this.buttons = btns;
      return _this;
    }

    _createClass$1(PlaylistSidebar, [{
      key: "onMount",
      value: function onMount() {
        this.buttons.forEach(function (button) {
          return button.mount();
        });
      }
    }, {
      key: "onUnmount",
      value: function onUnmount() {
        this.buttons.forEach(function (button) {
          return button.unmount();
        });
      }
    }]);

    return PlaylistSidebar;
  }(MidiComponent);

  function _createSuper$5(Derived) { return function () { var Super = _getPrototypeOf(Derived), result; if (_isNativeReflectConstruct$5()) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

  function _isNativeReflectConstruct$5() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Date.prototype.toString.call(Reflect.construct(Date, [], function () {})); return true; } catch (e) { return false; } }

  var ModifierSidebar = /*#__PURE__*/function (_MidiComponent) {
    _inherits(ModifierSidebar, _MidiComponent);

    var _super = _createSuper$5(ModifierSidebar);

    function ModifierSidebar(midibus) {
      var _this;

      _classCallCheck$1(this, ModifierSidebar);

      _this = _super.call(this, midibus);

      _defineProperty$1(_assertThisInitialized(_this), "shift", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "ctrl", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "state", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "listener", void 0);

      _this.shift = new MidiButtonComponent(_this.midibus, _this.device.buttons.solo);
      _this.ctrl = new MidiButtonComponent(_this.midibus, _this.device.buttons.arm);
      _this.state = {
        shift: false,
        ctrl: false
      };

      _this.listener = function (_ref) {
        var value = _ref.value,
            button = _ref.button,
            device = _ref.device;

        if (value) {
          button.sendColor(device.colors.hi_red);
        } else {
          button.sendColor(device.colors.black);
        }

        if (button.def.name === _this.device.buttons.solo.def.name) {
          _this.state.shift = !!value;

          _this.emit('shift', value);
        } else {
          _this.state.ctrl = !!value;

          _this.emit('ctrl', value);
        }
      };

      return _this;
    }

    _createClass$1(ModifierSidebar, [{
      key: "onMount",
      value: function onMount() {
        this.shift.mount();
        this.ctrl.mount();
        this.shift.on('midi', this.listener);
        this.ctrl.on('midi', this.listener);
      }
    }, {
      key: "onUnmount",
      value: function onUnmount() {
        this.shift.removeListener('midi', this.listener);
        this.ctrl.removeListener('midi', this.listener);
        this.shift.unmount();
        this.ctrl.unmount();
      }
    }, {
      key: "getState",
      value: function getState() {
        return this.state;
      }
    }]);

    return ModifierSidebar;
  }(MidiComponent);
  var modes = function modes(ctx, n, c, s, cs) {
    if (ctx.shift && ctx.ctrl) {
      cs && cs();
    } else if (ctx.shift) {
      s && s();
    } else if (ctx.ctrl) {
      c && c();
    } else {
      n && n();
    }
  };
  var retainAttackMode = function retainAttackMode(modifier, cb) {
    var state = {
      shift: false,
      ctrl: false
    };
    return function (data) {
      if (data.value) {
        state = modifier.getState();
      }

      for (var _len = arguments.length, rest = new Array(_len > 1 ? _len - 1 : 0), _key = 1; _key < _len; _key++) {
        rest[_key - 1] = arguments[_key];
      }

      return cb.apply(void 0, [state, data].concat(rest));
    };
  };

  function _arrayWithoutHoles(arr) {
    if (Array.isArray(arr)) return _arrayLikeToArray(arr);
  }

  function _iterableToArray(iter) {
    if (typeof Symbol !== "undefined" && Symbol.iterator in Object(iter)) return Array.from(iter);
  }

  function _nonIterableSpread() {
    throw new TypeError("Invalid attempt to spread non-iterable instance.\nIn order to be iterable, non-array objects must have a [Symbol.iterator]() method.");
  }

  function _toConsumableArray(arr) {
    return _arrayWithoutHoles(arr) || _iterableToArray(arr) || _unsupportedIterableToArray(arr) || _nonIterableSpread();
  }

  /** Detect free variable `global` from Node.js. */
  var freeGlobal$1 = (typeof global === "undefined" ? "undefined" : _typeof(global)) == 'object' && global && global.Object === Object && global;

  /** Detect free variable `self`. */

  var freeSelf$1 = (typeof self === "undefined" ? "undefined" : _typeof(self)) == 'object' && self && self.Object === Object && self;
  /** Used as a reference to the global object. */

  var root$1 = freeGlobal$1 || freeSelf$1 || Function('return this')();

  /** Built-in value references. */

  var _Symbol$1 = root$1.Symbol;

  /** Used for built-in method references. */

  var objectProto$9 = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$7 = objectProto$9.hasOwnProperty;
  /**
   * Used to resolve the
   * [`toStringTag`](http://ecma-international.org/ecma-262/7.0/#sec-object.prototype.tostring)
   * of values.
   */

  var nativeObjectToString$2 = objectProto$9.toString;
  /** Built-in value references. */

  var symToStringTag$2 = _Symbol$1 ? _Symbol$1.toStringTag : undefined;
  /**
   * A specialized version of `baseGetTag` which ignores `Symbol.toStringTag` values.
   *
   * @private
   * @param {*} value The value to query.
   * @returns {string} Returns the raw `toStringTag`.
   */

  function getRawTag$1(value) {
    var isOwn = hasOwnProperty$7.call(value, symToStringTag$2),
        tag = value[symToStringTag$2];

    try {
      value[symToStringTag$2] = undefined;
      var unmasked = true;
    } catch (e) {}

    var result = nativeObjectToString$2.call(value);

    if (unmasked) {
      if (isOwn) {
        value[symToStringTag$2] = tag;
      } else {
        delete value[symToStringTag$2];
      }
    }

    return result;
  }

  /** Used for built-in method references. */
  var objectProto$a = Object.prototype;
  /**
   * Used to resolve the
   * [`toStringTag`](http://ecma-international.org/ecma-262/7.0/#sec-object.prototype.tostring)
   * of values.
   */

  var nativeObjectToString$3 = objectProto$a.toString;
  /**
   * Converts `value` to a string using `Object.prototype.toString`.
   *
   * @private
   * @param {*} value The value to convert.
   * @returns {string} Returns the converted string.
   */

  function objectToString$1(value) {
    return nativeObjectToString$3.call(value);
  }

  /** `Object#toString` result references. */

  var nullTag$1 = '[object Null]',
      undefinedTag$1 = '[object Undefined]';
  /** Built-in value references. */

  var symToStringTag$3 = _Symbol$1 ? _Symbol$1.toStringTag : undefined;
  /**
   * The base implementation of `getTag` without fallbacks for buggy environments.
   *
   * @private
   * @param {*} value The value to query.
   * @returns {string} Returns the `toStringTag`.
   */

  function baseGetTag$1(value) {
    if (value == null) {
      return value === undefined ? undefinedTag$1 : nullTag$1;
    }

    return symToStringTag$3 && symToStringTag$3 in Object(value) ? getRawTag$1(value) : objectToString$1(value);
  }

  /**
   * Checks if `value` is the
   * [language type](http://www.ecma-international.org/ecma-262/7.0/#sec-ecmascript-language-types)
   * of `Object`. (e.g. arrays, functions, objects, regexes, `new Number(0)`, and `new String('')`)
   *
   * @static
   * @memberOf _
   * @since 0.1.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is an object, else `false`.
   * @example
   *
   * _.isObject({});
   * // => true
   *
   * _.isObject([1, 2, 3]);
   * // => true
   *
   * _.isObject(_.noop);
   * // => true
   *
   * _.isObject(null);
   * // => false
   */
  function isObject$1(value) {
    var type = _typeof(value);

    return value != null && (type == 'object' || type == 'function');
  }

  /** `Object#toString` result references. */

  var asyncTag$1 = '[object AsyncFunction]',
      funcTag$2 = '[object Function]',
      genTag$1 = '[object GeneratorFunction]',
      proxyTag$1 = '[object Proxy]';
  /**
   * Checks if `value` is classified as a `Function` object.
   *
   * @static
   * @memberOf _
   * @since 0.1.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a function, else `false`.
   * @example
   *
   * _.isFunction(_);
   * // => true
   *
   * _.isFunction(/abc/);
   * // => false
   */

  function isFunction$1(value) {
    if (!isObject$1(value)) {
      return false;
    } // The use of `Object#toString` avoids issues with the `typeof` operator
    // in Safari 9 which returns 'object' for typed arrays and other constructors.


    var tag = baseGetTag$1(value);
    return tag == funcTag$2 || tag == genTag$1 || tag == asyncTag$1 || tag == proxyTag$1;
  }

  /** Used to detect overreaching core-js shims. */

  var coreJsData$1 = root$1['__core-js_shared__'];

  /** Used to detect methods masquerading as native. */

  var maskSrcKey$1 = function () {
    var uid = /[^.]+$/.exec(coreJsData$1 && coreJsData$1.keys && coreJsData$1.keys.IE_PROTO || '');
    return uid ? 'Symbol(src)_1.' + uid : '';
  }();
  /**
   * Checks if `func` has its source masked.
   *
   * @private
   * @param {Function} func The function to check.
   * @returns {boolean} Returns `true` if `func` is masked, else `false`.
   */


  function isMasked$1(func) {
    return !!maskSrcKey$1 && maskSrcKey$1 in func;
  }

  /** Used for built-in method references. */
  var funcProto$2 = Function.prototype;
  /** Used to resolve the decompiled source of functions. */

  var funcToString$2 = funcProto$2.toString;
  /**
   * Converts `func` to its source code.
   *
   * @private
   * @param {Function} func The function to convert.
   * @returns {string} Returns the source code.
   */

  function toSource$1(func) {
    if (func != null) {
      try {
        return funcToString$2.call(func);
      } catch (e) {}

      try {
        return func + '';
      } catch (e) {}
    }

    return '';
  }

  /**
   * Used to match `RegExp`
   * [syntax characters](http://ecma-international.org/ecma-262/7.0/#sec-patterns).
   */

  var reRegExpChar$1 = /[\\^$.*+?()[\]{}|]/g;
  /** Used to detect host constructors (Safari). */

  var reIsHostCtor$1 = /^\[object .+?Constructor\]$/;
  /** Used for built-in method references. */

  var funcProto$3 = Function.prototype,
      objectProto$b = Object.prototype;
  /** Used to resolve the decompiled source of functions. */

  var funcToString$3 = funcProto$3.toString;
  /** Used to check objects for own properties. */

  var hasOwnProperty$8 = objectProto$b.hasOwnProperty;
  /** Used to detect if a method is native. */

  var reIsNative$1 = RegExp('^' + funcToString$3.call(hasOwnProperty$8).replace(reRegExpChar$1, '\\$&').replace(/hasOwnProperty|(function).*?(?=\\\()| for .+?(?=\\\])/g, '$1.*?') + '$');
  /**
   * The base implementation of `_.isNative` without bad shim checks.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a native function,
   *  else `false`.
   */

  function baseIsNative$1(value) {
    if (!isObject$1(value) || isMasked$1(value)) {
      return false;
    }

    var pattern = isFunction$1(value) ? reIsNative$1 : reIsHostCtor$1;
    return pattern.test(toSource$1(value));
  }

  /**
   * Gets the value at `key` of `object`.
   *
   * @private
   * @param {Object} [object] The object to query.
   * @param {string} key The key of the property to get.
   * @returns {*} Returns the property value.
   */
  function getValue$1(object, key) {
    return object == null ? undefined : object[key];
  }

  /**
   * Gets the native function at `key` of `object`.
   *
   * @private
   * @param {Object} object The object to query.
   * @param {string} key The key of the method to get.
   * @returns {*} Returns the function if it's native, else `undefined`.
   */

  function getNative$1(object, key) {
    var value = getValue$1(object, key);
    return baseIsNative$1(value) ? value : undefined;
  }

  var defineProperty$1 = function () {
    try {
      var func = getNative$1(Object, 'defineProperty');
      func({}, '', {});
      return func;
    } catch (e) {}
  }();

  /**
   * The base implementation of `assignValue` and `assignMergeValue` without
   * value checks.
   *
   * @private
   * @param {Object} object The object to modify.
   * @param {string} key The key of the property to assign.
   * @param {*} value The value to assign.
   */

  function baseAssignValue$1(object, key, value) {
    if (key == '__proto__' && defineProperty$1) {
      defineProperty$1(object, key, {
        'configurable': true,
        'enumerable': true,
        'value': value,
        'writable': true
      });
    } else {
      object[key] = value;
    }
  }

  /**
   * Performs a
   * [`SameValueZero`](http://ecma-international.org/ecma-262/7.0/#sec-samevaluezero)
   * comparison between two values to determine if they are equivalent.
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to compare.
   * @param {*} other The other value to compare.
   * @returns {boolean} Returns `true` if the values are equivalent, else `false`.
   * @example
   *
   * var object = { 'a': 1 };
   * var other = { 'a': 1 };
   *
   * _.eq(object, object);
   * // => true
   *
   * _.eq(object, other);
   * // => false
   *
   * _.eq('a', 'a');
   * // => true
   *
   * _.eq('a', Object('a'));
   * // => false
   *
   * _.eq(NaN, NaN);
   * // => true
   */
  function eq$1(value, other) {
    return value === other || value !== value && other !== other;
  }

  /** Used for built-in method references. */

  var objectProto$c = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$9 = objectProto$c.hasOwnProperty;
  /**
   * Assigns `value` to `key` of `object` if the existing value is not equivalent
   * using [`SameValueZero`](http://ecma-international.org/ecma-262/7.0/#sec-samevaluezero)
   * for equality comparisons.
   *
   * @private
   * @param {Object} object The object to modify.
   * @param {string} key The key of the property to assign.
   * @param {*} value The value to assign.
   */

  function assignValue$1(object, key, value) {
    var objValue = object[key];

    if (!(hasOwnProperty$9.call(object, key) && eq$1(objValue, value)) || value === undefined && !(key in object)) {
      baseAssignValue$1(object, key, value);
    }
  }

  /**
   * Copies properties of `source` to `object`.
   *
   * @private
   * @param {Object} source The object to copy properties from.
   * @param {Array} props The property identifiers to copy.
   * @param {Object} [object={}] The object to copy properties to.
   * @param {Function} [customizer] The function to customize copied values.
   * @returns {Object} Returns `object`.
   */

  function copyObject$1(source, props, object, customizer) {
    var isNew = !object;
    object || (object = {});
    var index = -1,
        length = props.length;

    while (++index < length) {
      var key = props[index];
      var newValue = customizer ? customizer(object[key], source[key], key, object, source) : undefined;

      if (newValue === undefined) {
        newValue = source[key];
      }

      if (isNew) {
        baseAssignValue$1(object, key, newValue);
      } else {
        assignValue$1(object, key, newValue);
      }
    }

    return object;
  }

  /**
   * This method returns the first argument it receives.
   *
   * @static
   * @since 0.1.0
   * @memberOf _
   * @category Util
   * @param {*} value Any value.
   * @returns {*} Returns `value`.
   * @example
   *
   * var object = { 'a': 1 };
   *
   * console.log(_.identity(object) === object);
   * // => true
   */
  function identity$1(value) {
    return value;
  }

  /**
   * A faster alternative to `Function#apply`, this function invokes `func`
   * with the `this` binding of `thisArg` and the arguments of `args`.
   *
   * @private
   * @param {Function} func The function to invoke.
   * @param {*} thisArg The `this` binding of `func`.
   * @param {Array} args The arguments to invoke `func` with.
   * @returns {*} Returns the result of `func`.
   */
  function apply$1(func, thisArg, args) {
    switch (args.length) {
      case 0:
        return func.call(thisArg);

      case 1:
        return func.call(thisArg, args[0]);

      case 2:
        return func.call(thisArg, args[0], args[1]);

      case 3:
        return func.call(thisArg, args[0], args[1], args[2]);
    }

    return func.apply(thisArg, args);
  }

  /* Built-in method references for those with the same name as other `lodash` methods. */

  var nativeMax$2 = Math.max;
  /**
   * A specialized version of `baseRest` which transforms the rest array.
   *
   * @private
   * @param {Function} func The function to apply a rest parameter to.
   * @param {number} [start=func.length-1] The start position of the rest parameter.
   * @param {Function} transform The rest array transform.
   * @returns {Function} Returns the new function.
   */

  function overRest$1(func, start, transform) {
    start = nativeMax$2(start === undefined ? func.length - 1 : start, 0);
    return function () {
      var args = arguments,
          index = -1,
          length = nativeMax$2(args.length - start, 0),
          array = Array(length);

      while (++index < length) {
        array[index] = args[start + index];
      }

      index = -1;
      var otherArgs = Array(start + 1);

      while (++index < start) {
        otherArgs[index] = args[index];
      }

      otherArgs[start] = transform(array);
      return apply$1(func, this, otherArgs);
    };
  }

  /**
   * Creates a function that returns `value`.
   *
   * @static
   * @memberOf _
   * @since 2.4.0
   * @category Util
   * @param {*} value The value to return from the new function.
   * @returns {Function} Returns the new constant function.
   * @example
   *
   * var objects = _.times(2, _.constant({ 'a': 1 }));
   *
   * console.log(objects);
   * // => [{ 'a': 1 }, { 'a': 1 }]
   *
   * console.log(objects[0] === objects[1]);
   * // => true
   */
  function constant$1(value) {
    return function () {
      return value;
    };
  }

  /**
   * The base implementation of `setToString` without support for hot loop shorting.
   *
   * @private
   * @param {Function} func The function to modify.
   * @param {Function} string The `toString` result.
   * @returns {Function} Returns `func`.
   */

  var baseSetToString$1 = !defineProperty$1 ? identity$1 : function (func, string) {
    return defineProperty$1(func, 'toString', {
      'configurable': true,
      'enumerable': false,
      'value': constant$1(string),
      'writable': true
    });
  };

  /** Used to detect hot functions by number of calls within a span of milliseconds. */
  var HOT_COUNT$1 = 800,
      HOT_SPAN$1 = 16;
  /* Built-in method references for those with the same name as other `lodash` methods. */

  var nativeNow$1 = Date.now;
  /**
   * Creates a function that'll short out and invoke `identity` instead
   * of `func` when it's called `HOT_COUNT` or more times in `HOT_SPAN`
   * milliseconds.
   *
   * @private
   * @param {Function} func The function to restrict.
   * @returns {Function} Returns the new shortable function.
   */

  function shortOut$1(func) {
    var count = 0,
        lastCalled = 0;
    return function () {
      var stamp = nativeNow$1(),
          remaining = HOT_SPAN$1 - (stamp - lastCalled);
      lastCalled = stamp;

      if (remaining > 0) {
        if (++count >= HOT_COUNT$1) {
          return arguments[0];
        }
      } else {
        count = 0;
      }

      return func.apply(undefined, arguments);
    };
  }

  /**
   * Sets the `toString` method of `func` to return `string`.
   *
   * @private
   * @param {Function} func The function to modify.
   * @param {Function} string The `toString` result.
   * @returns {Function} Returns `func`.
   */

  var setToString$1 = shortOut$1(baseSetToString$1);

  /**
   * The base implementation of `_.rest` which doesn't validate or coerce arguments.
   *
   * @private
   * @param {Function} func The function to apply a rest parameter to.
   * @param {number} [start=func.length-1] The start position of the rest parameter.
   * @returns {Function} Returns the new function.
   */

  function baseRest$1(func, start) {
    return setToString$1(overRest$1(func, start, identity$1), func + '');
  }

  /** Used as references for various `Number` constants. */
  var MAX_SAFE_INTEGER$2 = 9007199254740991;
  /**
   * Checks if `value` is a valid array-like length.
   *
   * **Note:** This method is loosely based on
   * [`ToLength`](http://ecma-international.org/ecma-262/7.0/#sec-tolength).
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a valid length, else `false`.
   * @example
   *
   * _.isLength(3);
   * // => true
   *
   * _.isLength(Number.MIN_VALUE);
   * // => false
   *
   * _.isLength(Infinity);
   * // => false
   *
   * _.isLength('3');
   * // => false
   */

  function isLength$1(value) {
    return typeof value == 'number' && value > -1 && value % 1 == 0 && value <= MAX_SAFE_INTEGER$2;
  }

  /**
   * Checks if `value` is array-like. A value is considered array-like if it's
   * not a function and has a `value.length` that's an integer greater than or
   * equal to `0` and less than or equal to `Number.MAX_SAFE_INTEGER`.
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is array-like, else `false`.
   * @example
   *
   * _.isArrayLike([1, 2, 3]);
   * // => true
   *
   * _.isArrayLike(document.body.children);
   * // => true
   *
   * _.isArrayLike('abc');
   * // => true
   *
   * _.isArrayLike(_.noop);
   * // => false
   */

  function isArrayLike$1(value) {
    return value != null && isLength$1(value.length) && !isFunction$1(value);
  }

  /** Used as references for various `Number` constants. */
  var MAX_SAFE_INTEGER$3 = 9007199254740991;
  /** Used to detect unsigned integer values. */

  var reIsUint$1 = /^(?:0|[1-9]\d*)$/;
  /**
   * Checks if `value` is a valid array-like index.
   *
   * @private
   * @param {*} value The value to check.
   * @param {number} [length=MAX_SAFE_INTEGER] The upper bounds of a valid index.
   * @returns {boolean} Returns `true` if `value` is a valid index, else `false`.
   */

  function isIndex$1(value, length) {
    var type = _typeof(value);

    length = length == null ? MAX_SAFE_INTEGER$3 : length;
    return !!length && (type == 'number' || type != 'symbol' && reIsUint$1.test(value)) && value > -1 && value % 1 == 0 && value < length;
  }

  /**
   * Checks if the given arguments are from an iteratee call.
   *
   * @private
   * @param {*} value The potential iteratee value argument.
   * @param {*} index The potential iteratee index or key argument.
   * @param {*} object The potential iteratee object argument.
   * @returns {boolean} Returns `true` if the arguments are from an iteratee call,
   *  else `false`.
   */

  function isIterateeCall$1(value, index, object) {
    if (!isObject$1(object)) {
      return false;
    }

    var type = _typeof(index);

    if (type == 'number' ? isArrayLike$1(object) && isIndex$1(index, object.length) : type == 'string' && index in object) {
      return eq$1(object[index], value);
    }

    return false;
  }

  /**
   * Creates a function like `_.assign`.
   *
   * @private
   * @param {Function} assigner The function to assign values.
   * @returns {Function} Returns the new assigner function.
   */

  function createAssigner$1(assigner) {
    return baseRest$1(function (object, sources) {
      var index = -1,
          length = sources.length,
          customizer = length > 1 ? sources[length - 1] : undefined,
          guard = length > 2 ? sources[2] : undefined;
      customizer = assigner.length > 3 && typeof customizer == 'function' ? (length--, customizer) : undefined;

      if (guard && isIterateeCall$1(sources[0], sources[1], guard)) {
        customizer = length < 3 ? undefined : customizer;
        length = 1;
      }

      object = Object(object);

      while (++index < length) {
        var source = sources[index];

        if (source) {
          assigner(object, source, index, customizer);
        }
      }

      return object;
    });
  }

  /** Used for built-in method references. */
  var objectProto$d = Object.prototype;
  /**
   * Checks if `value` is likely a prototype object.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a prototype, else `false`.
   */

  function isPrototype$1(value) {
    var Ctor = value && value.constructor,
        proto = typeof Ctor == 'function' && Ctor.prototype || objectProto$d;
    return value === proto;
  }

  /**
   * The base implementation of `_.times` without support for iteratee shorthands
   * or max array length checks.
   *
   * @private
   * @param {number} n The number of times to invoke `iteratee`.
   * @param {Function} iteratee The function invoked per iteration.
   * @returns {Array} Returns the array of results.
   */
  function baseTimes$1(n, iteratee) {
    var index = -1,
        result = Array(n);

    while (++index < n) {
      result[index] = iteratee(index);
    }

    return result;
  }

  /**
   * Checks if `value` is object-like. A value is object-like if it's not `null`
   * and has a `typeof` result of "object".
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is object-like, else `false`.
   * @example
   *
   * _.isObjectLike({});
   * // => true
   *
   * _.isObjectLike([1, 2, 3]);
   * // => true
   *
   * _.isObjectLike(_.noop);
   * // => false
   *
   * _.isObjectLike(null);
   * // => false
   */
  function isObjectLike$1(value) {
    return value != null && _typeof(value) == 'object';
  }

  /** `Object#toString` result references. */

  var argsTag$2 = '[object Arguments]';
  /**
   * The base implementation of `_.isArguments`.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is an `arguments` object,
   */

  function baseIsArguments$1(value) {
    return isObjectLike$1(value) && baseGetTag$1(value) == argsTag$2;
  }

  /** Used for built-in method references. */

  var objectProto$e = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$a = objectProto$e.hasOwnProperty;
  /** Built-in value references. */

  var propertyIsEnumerable$1 = objectProto$e.propertyIsEnumerable;
  /**
   * Checks if `value` is likely an `arguments` object.
   *
   * @static
   * @memberOf _
   * @since 0.1.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is an `arguments` object,
   *  else `false`.
   * @example
   *
   * _.isArguments(function() { return arguments; }());
   * // => true
   *
   * _.isArguments([1, 2, 3]);
   * // => false
   */

  var isArguments$1 = baseIsArguments$1(function () {
    return arguments;
  }()) ? baseIsArguments$1 : function (value) {
    return isObjectLike$1(value) && hasOwnProperty$a.call(value, 'callee') && !propertyIsEnumerable$1.call(value, 'callee');
  };

  /**
   * Checks if `value` is classified as an `Array` object.
   *
   * @static
   * @memberOf _
   * @since 0.1.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is an array, else `false`.
   * @example
   *
   * _.isArray([1, 2, 3]);
   * // => true
   *
   * _.isArray(document.body.children);
   * // => false
   *
   * _.isArray('abc');
   * // => false
   *
   * _.isArray(_.noop);
   * // => false
   */
  var isArray$1 = Array.isArray;

  /**
   * This method returns `false`.
   *
   * @static
   * @memberOf _
   * @since 4.13.0
   * @category Util
   * @returns {boolean} Returns `false`.
   * @example
   *
   * _.times(2, _.stubFalse);
   * // => [false, false]
   */
  function stubFalse$1() {
    return false;
  }

  /** Detect free variable `exports`. */

  var freeExports$2 = (typeof exports === "undefined" ? "undefined" : _typeof(exports)) == 'object' && exports && !exports.nodeType && exports;
  /** Detect free variable `module`. */

  var freeModule$2 = freeExports$2 && (typeof module === "undefined" ? "undefined" : _typeof(module)) == 'object' && module && !module.nodeType && module;
  /** Detect the popular CommonJS extension `module.exports`. */

  var moduleExports$2 = freeModule$2 && freeModule$2.exports === freeExports$2;
  /** Built-in value references. */

  var Buffer$1 = moduleExports$2 ? root$1.Buffer : undefined;
  /* Built-in method references for those with the same name as other `lodash` methods. */

  var nativeIsBuffer$1 = Buffer$1 ? Buffer$1.isBuffer : undefined;
  /**
   * Checks if `value` is a buffer.
   *
   * @static
   * @memberOf _
   * @since 4.3.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a buffer, else `false`.
   * @example
   *
   * _.isBuffer(new Buffer(2));
   * // => true
   *
   * _.isBuffer(new Uint8Array(2));
   * // => false
   */

  var isBuffer$1 = nativeIsBuffer$1 || stubFalse$1;

  /** `Object#toString` result references. */

  var argsTag$3 = '[object Arguments]',
      arrayTag$1 = '[object Array]',
      boolTag$1 = '[object Boolean]',
      dateTag$1 = '[object Date]',
      errorTag$1 = '[object Error]',
      funcTag$3 = '[object Function]',
      mapTag$1 = '[object Map]',
      numberTag$1 = '[object Number]',
      objectTag$1 = '[object Object]',
      regexpTag$1 = '[object RegExp]',
      setTag$1 = '[object Set]',
      stringTag$1 = '[object String]',
      weakMapTag$1 = '[object WeakMap]';
  var arrayBufferTag$1 = '[object ArrayBuffer]',
      dataViewTag$1 = '[object DataView]',
      float32Tag$1 = '[object Float32Array]',
      float64Tag$1 = '[object Float64Array]',
      int8Tag$1 = '[object Int8Array]',
      int16Tag$1 = '[object Int16Array]',
      int32Tag$1 = '[object Int32Array]',
      uint8Tag$1 = '[object Uint8Array]',
      uint8ClampedTag$1 = '[object Uint8ClampedArray]',
      uint16Tag$1 = '[object Uint16Array]',
      uint32Tag$1 = '[object Uint32Array]';
  /** Used to identify `toStringTag` values of typed arrays. */

  var typedArrayTags$1 = {};
  typedArrayTags$1[float32Tag$1] = typedArrayTags$1[float64Tag$1] = typedArrayTags$1[int8Tag$1] = typedArrayTags$1[int16Tag$1] = typedArrayTags$1[int32Tag$1] = typedArrayTags$1[uint8Tag$1] = typedArrayTags$1[uint8ClampedTag$1] = typedArrayTags$1[uint16Tag$1] = typedArrayTags$1[uint32Tag$1] = true;
  typedArrayTags$1[argsTag$3] = typedArrayTags$1[arrayTag$1] = typedArrayTags$1[arrayBufferTag$1] = typedArrayTags$1[boolTag$1] = typedArrayTags$1[dataViewTag$1] = typedArrayTags$1[dateTag$1] = typedArrayTags$1[errorTag$1] = typedArrayTags$1[funcTag$3] = typedArrayTags$1[mapTag$1] = typedArrayTags$1[numberTag$1] = typedArrayTags$1[objectTag$1] = typedArrayTags$1[regexpTag$1] = typedArrayTags$1[setTag$1] = typedArrayTags$1[stringTag$1] = typedArrayTags$1[weakMapTag$1] = false;
  /**
   * The base implementation of `_.isTypedArray` without Node.js optimizations.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a typed array, else `false`.
   */

  function baseIsTypedArray$1(value) {
    return isObjectLike$1(value) && isLength$1(value.length) && !!typedArrayTags$1[baseGetTag$1(value)];
  }

  /**
   * The base implementation of `_.unary` without support for storing metadata.
   *
   * @private
   * @param {Function} func The function to cap arguments for.
   * @returns {Function} Returns the new capped function.
   */
  function baseUnary$1(func) {
    return function (value) {
      return func(value);
    };
  }

  /** Detect free variable `exports`. */

  var freeExports$3 = (typeof exports === "undefined" ? "undefined" : _typeof(exports)) == 'object' && exports && !exports.nodeType && exports;
  /** Detect free variable `module`. */

  var freeModule$3 = freeExports$3 && (typeof module === "undefined" ? "undefined" : _typeof(module)) == 'object' && module && !module.nodeType && module;
  /** Detect the popular CommonJS extension `module.exports`. */

  var moduleExports$3 = freeModule$3 && freeModule$3.exports === freeExports$3;
  /** Detect free variable `process` from Node.js. */

  var freeProcess$1 = moduleExports$3 && freeGlobal$1.process;
  /** Used to access faster Node.js helpers. */

  var nodeUtil$1 = function () {
    try {
      // Use `util.types` for Node.js 10+.
      var types = freeModule$3 && freeModule$3.require && freeModule$3.require('util').types;

      if (types) {
        return types;
      } // Legacy `process.binding('util')` for Node.js < 10.


      return freeProcess$1 && freeProcess$1.binding && freeProcess$1.binding('util');
    } catch (e) {}
  }();

  /* Node.js helper references. */

  var nodeIsTypedArray$1 = nodeUtil$1 && nodeUtil$1.isTypedArray;
  /**
   * Checks if `value` is classified as a typed array.
   *
   * @static
   * @memberOf _
   * @since 3.0.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a typed array, else `false`.
   * @example
   *
   * _.isTypedArray(new Uint8Array);
   * // => true
   *
   * _.isTypedArray([]);
   * // => false
   */

  var isTypedArray$1 = nodeIsTypedArray$1 ? baseUnary$1(nodeIsTypedArray$1) : baseIsTypedArray$1;

  /** Used for built-in method references. */

  var objectProto$f = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$b = objectProto$f.hasOwnProperty;
  /**
   * Creates an array of the enumerable property names of the array-like `value`.
   *
   * @private
   * @param {*} value The value to query.
   * @param {boolean} inherited Specify returning inherited property names.
   * @returns {Array} Returns the array of property names.
   */

  function arrayLikeKeys$1(value, inherited) {
    var isArr = isArray$1(value),
        isArg = !isArr && isArguments$1(value),
        isBuff = !isArr && !isArg && isBuffer$1(value),
        isType = !isArr && !isArg && !isBuff && isTypedArray$1(value),
        skipIndexes = isArr || isArg || isBuff || isType,
        result = skipIndexes ? baseTimes$1(value.length, String) : [],
        length = result.length;

    for (var key in value) {
      if ((inherited || hasOwnProperty$b.call(value, key)) && !(skipIndexes && ( // Safari 9 has enumerable `arguments.length` in strict mode.
      key == 'length' || // Node.js 0.10 has enumerable non-index properties on buffers.
      isBuff && (key == 'offset' || key == 'parent') || // PhantomJS 2 has enumerable non-index properties on typed arrays.
      isType && (key == 'buffer' || key == 'byteLength' || key == 'byteOffset') || // Skip index properties.
      isIndex$1(key, length)))) {
        result.push(key);
      }
    }

    return result;
  }

  /**
   * Creates a unary function that invokes `func` with its argument transformed.
   *
   * @private
   * @param {Function} func The function to wrap.
   * @param {Function} transform The argument transform.
   * @returns {Function} Returns the new function.
   */
  function overArg$1(func, transform) {
    return function (arg) {
      return func(transform(arg));
    };
  }

  /* Built-in method references for those with the same name as other `lodash` methods. */

  var nativeKeys$1 = overArg$1(Object.keys, Object);

  /** Used for built-in method references. */

  var objectProto$g = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$c = objectProto$g.hasOwnProperty;
  /**
   * The base implementation of `_.keys` which doesn't treat sparse arrays as dense.
   *
   * @private
   * @param {Object} object The object to query.
   * @returns {Array} Returns the array of property names.
   */

  function baseKeys$1(object) {
    if (!isPrototype$1(object)) {
      return nativeKeys$1(object);
    }

    var result = [];

    for (var key in Object(object)) {
      if (hasOwnProperty$c.call(object, key) && key != 'constructor') {
        result.push(key);
      }
    }

    return result;
  }

  /**
   * Creates an array of the own enumerable property names of `object`.
   *
   * **Note:** Non-object values are coerced to objects. See the
   * [ES spec](http://ecma-international.org/ecma-262/7.0/#sec-object.keys)
   * for more details.
   *
   * @static
   * @since 0.1.0
   * @memberOf _
   * @category Object
   * @param {Object} object The object to query.
   * @returns {Array} Returns the array of property names.
   * @example
   *
   * function Foo() {
   *   this.a = 1;
   *   this.b = 2;
   * }
   *
   * Foo.prototype.c = 3;
   *
   * _.keys(new Foo);
   * // => ['a', 'b'] (iteration order is not guaranteed)
   *
   * _.keys('hi');
   * // => ['0', '1']
   */

  function keys$1(object) {
    return isArrayLike$1(object) ? arrayLikeKeys$1(object) : baseKeys$1(object);
  }

  /** Used for built-in method references. */

  var objectProto$h = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$d = objectProto$h.hasOwnProperty;
  /**
   * Assigns own enumerable string keyed properties of source objects to the
   * destination object. Source objects are applied from left to right.
   * Subsequent sources overwrite property assignments of previous sources.
   *
   * **Note:** This method mutates `object` and is loosely based on
   * [`Object.assign`](https://mdn.io/Object/assign).
   *
   * @static
   * @memberOf _
   * @since 0.10.0
   * @category Object
   * @param {Object} object The destination object.
   * @param {...Object} [sources] The source objects.
   * @returns {Object} Returns `object`.
   * @see _.assignIn
   * @example
   *
   * function Foo() {
   *   this.a = 1;
   * }
   *
   * function Bar() {
   *   this.c = 3;
   * }
   *
   * Foo.prototype.b = 2;
   * Bar.prototype.d = 4;
   *
   * _.assign({ 'a': 0 }, new Foo, new Bar);
   * // => { 'a': 1, 'c': 3 }
   */

  var assign$1 = createAssigner$1(function (object, source) {
    if (isPrototype$1(source) || isArrayLike$1(source)) {
      copyObject$1(source, keys$1(source), object);
      return;
    }

    for (var key in source) {
      if (hasOwnProperty$d.call(source, key)) {
        assignValue$1(object, key, source[key]);
      }
    }
  });

  /**
   * The base implementation of `_.findIndex` and `_.findLastIndex` without
   * support for iteratee shorthands.
   *
   * @private
   * @param {Array} array The array to inspect.
   * @param {Function} predicate The function invoked per iteration.
   * @param {number} fromIndex The index to search from.
   * @param {boolean} [fromRight] Specify iterating from right to left.
   * @returns {number} Returns the index of the matched value, else `-1`.
   */
  function baseFindIndex(array, predicate, fromIndex, fromRight) {
    var length = array.length,
        index = fromIndex + (fromRight ? 1 : -1);

    while (fromRight ? index-- : ++index < length) {
      if (predicate(array[index], index, array)) {
        return index;
      }
    }

    return -1;
  }

  /**
   * Removes all key-value entries from the list cache.
   *
   * @private
   * @name clear
   * @memberOf ListCache
   */
  function listCacheClear() {
    this.__data__ = [];
    this.size = 0;
  }

  /**
   * Gets the index at which the `key` is found in `array` of key-value pairs.
   *
   * @private
   * @param {Array} array The array to inspect.
   * @param {*} key The key to search for.
   * @returns {number} Returns the index of the matched value, else `-1`.
   */

  function assocIndexOf(array, key) {
    var length = array.length;

    while (length--) {
      if (eq$1(array[length][0], key)) {
        return length;
      }
    }

    return -1;
  }

  /** Used for built-in method references. */

  var arrayProto = Array.prototype;
  /** Built-in value references. */

  var splice = arrayProto.splice;
  /**
   * Removes `key` and its value from the list cache.
   *
   * @private
   * @name delete
   * @memberOf ListCache
   * @param {string} key The key of the value to remove.
   * @returns {boolean} Returns `true` if the entry was removed, else `false`.
   */

  function listCacheDelete(key) {
    var data = this.__data__,
        index = assocIndexOf(data, key);

    if (index < 0) {
      return false;
    }

    var lastIndex = data.length - 1;

    if (index == lastIndex) {
      data.pop();
    } else {
      splice.call(data, index, 1);
    }

    --this.size;
    return true;
  }

  /**
   * Gets the list cache value for `key`.
   *
   * @private
   * @name get
   * @memberOf ListCache
   * @param {string} key The key of the value to get.
   * @returns {*} Returns the entry value.
   */

  function listCacheGet(key) {
    var data = this.__data__,
        index = assocIndexOf(data, key);
    return index < 0 ? undefined : data[index][1];
  }

  /**
   * Checks if a list cache value for `key` exists.
   *
   * @private
   * @name has
   * @memberOf ListCache
   * @param {string} key The key of the entry to check.
   * @returns {boolean} Returns `true` if an entry for `key` exists, else `false`.
   */

  function listCacheHas(key) {
    return assocIndexOf(this.__data__, key) > -1;
  }

  /**
   * Sets the list cache `key` to `value`.
   *
   * @private
   * @name set
   * @memberOf ListCache
   * @param {string} key The key of the value to set.
   * @param {*} value The value to set.
   * @returns {Object} Returns the list cache instance.
   */

  function listCacheSet(key, value) {
    var data = this.__data__,
        index = assocIndexOf(data, key);

    if (index < 0) {
      ++this.size;
      data.push([key, value]);
    } else {
      data[index][1] = value;
    }

    return this;
  }

  /**
   * Creates an list cache object.
   *
   * @private
   * @constructor
   * @param {Array} [entries] The key-value pairs to cache.
   */

  function ListCache(entries) {
    var index = -1,
        length = entries == null ? 0 : entries.length;
    this.clear();

    while (++index < length) {
      var entry = entries[index];
      this.set(entry[0], entry[1]);
    }
  } // Add methods to `ListCache`.


  ListCache.prototype.clear = listCacheClear;
  ListCache.prototype['delete'] = listCacheDelete;
  ListCache.prototype.get = listCacheGet;
  ListCache.prototype.has = listCacheHas;
  ListCache.prototype.set = listCacheSet;

  /**
   * Removes all key-value entries from the stack.
   *
   * @private
   * @name clear
   * @memberOf Stack
   */

  function stackClear() {
    this.__data__ = new ListCache();
    this.size = 0;
  }

  /**
   * Removes `key` and its value from the stack.
   *
   * @private
   * @name delete
   * @memberOf Stack
   * @param {string} key The key of the value to remove.
   * @returns {boolean} Returns `true` if the entry was removed, else `false`.
   */
  function stackDelete(key) {
    var data = this.__data__,
        result = data['delete'](key);
    this.size = data.size;
    return result;
  }

  /**
   * Gets the stack value for `key`.
   *
   * @private
   * @name get
   * @memberOf Stack
   * @param {string} key The key of the value to get.
   * @returns {*} Returns the entry value.
   */
  function stackGet(key) {
    return this.__data__.get(key);
  }

  /**
   * Checks if a stack value for `key` exists.
   *
   * @private
   * @name has
   * @memberOf Stack
   * @param {string} key The key of the entry to check.
   * @returns {boolean} Returns `true` if an entry for `key` exists, else `false`.
   */
  function stackHas(key) {
    return this.__data__.has(key);
  }

  /* Built-in method references that are verified to be native. */

  var Map = getNative$1(root$1, 'Map');

  /* Built-in method references that are verified to be native. */

  var nativeCreate = getNative$1(Object, 'create');

  /**
   * Removes all key-value entries from the hash.
   *
   * @private
   * @name clear
   * @memberOf Hash
   */

  function hashClear() {
    this.__data__ = nativeCreate ? nativeCreate(null) : {};
    this.size = 0;
  }

  /**
   * Removes `key` and its value from the hash.
   *
   * @private
   * @name delete
   * @memberOf Hash
   * @param {Object} hash The hash to modify.
   * @param {string} key The key of the value to remove.
   * @returns {boolean} Returns `true` if the entry was removed, else `false`.
   */
  function hashDelete(key) {
    var result = this.has(key) && delete this.__data__[key];
    this.size -= result ? 1 : 0;
    return result;
  }

  /** Used to stand-in for `undefined` hash values. */

  var HASH_UNDEFINED = '__lodash_hash_undefined__';
  /** Used for built-in method references. */

  var objectProto$i = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$e = objectProto$i.hasOwnProperty;
  /**
   * Gets the hash value for `key`.
   *
   * @private
   * @name get
   * @memberOf Hash
   * @param {string} key The key of the value to get.
   * @returns {*} Returns the entry value.
   */

  function hashGet(key) {
    var data = this.__data__;

    if (nativeCreate) {
      var result = data[key];
      return result === HASH_UNDEFINED ? undefined : result;
    }

    return hasOwnProperty$e.call(data, key) ? data[key] : undefined;
  }

  /** Used for built-in method references. */

  var objectProto$j = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$f = objectProto$j.hasOwnProperty;
  /**
   * Checks if a hash value for `key` exists.
   *
   * @private
   * @name has
   * @memberOf Hash
   * @param {string} key The key of the entry to check.
   * @returns {boolean} Returns `true` if an entry for `key` exists, else `false`.
   */

  function hashHas(key) {
    var data = this.__data__;
    return nativeCreate ? data[key] !== undefined : hasOwnProperty$f.call(data, key);
  }

  /** Used to stand-in for `undefined` hash values. */

  var HASH_UNDEFINED$1 = '__lodash_hash_undefined__';
  /**
   * Sets the hash `key` to `value`.
   *
   * @private
   * @name set
   * @memberOf Hash
   * @param {string} key The key of the value to set.
   * @param {*} value The value to set.
   * @returns {Object} Returns the hash instance.
   */

  function hashSet(key, value) {
    var data = this.__data__;
    this.size += this.has(key) ? 0 : 1;
    data[key] = nativeCreate && value === undefined ? HASH_UNDEFINED$1 : value;
    return this;
  }

  /**
   * Creates a hash object.
   *
   * @private
   * @constructor
   * @param {Array} [entries] The key-value pairs to cache.
   */

  function Hash(entries) {
    var index = -1,
        length = entries == null ? 0 : entries.length;
    this.clear();

    while (++index < length) {
      var entry = entries[index];
      this.set(entry[0], entry[1]);
    }
  } // Add methods to `Hash`.


  Hash.prototype.clear = hashClear;
  Hash.prototype['delete'] = hashDelete;
  Hash.prototype.get = hashGet;
  Hash.prototype.has = hashHas;
  Hash.prototype.set = hashSet;

  /**
   * Removes all key-value entries from the map.
   *
   * @private
   * @name clear
   * @memberOf MapCache
   */

  function mapCacheClear() {
    this.size = 0;
    this.__data__ = {
      'hash': new Hash(),
      'map': new (Map || ListCache)(),
      'string': new Hash()
    };
  }

  /**
   * Checks if `value` is suitable for use as unique object key.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is suitable, else `false`.
   */
  function isKeyable(value) {
    var type = _typeof(value);

    return type == 'string' || type == 'number' || type == 'symbol' || type == 'boolean' ? value !== '__proto__' : value === null;
  }

  /**
   * Gets the data for `map`.
   *
   * @private
   * @param {Object} map The map to query.
   * @param {string} key The reference key.
   * @returns {*} Returns the map data.
   */

  function getMapData(map, key) {
    var data = map.__data__;
    return isKeyable(key) ? data[typeof key == 'string' ? 'string' : 'hash'] : data.map;
  }

  /**
   * Removes `key` and its value from the map.
   *
   * @private
   * @name delete
   * @memberOf MapCache
   * @param {string} key The key of the value to remove.
   * @returns {boolean} Returns `true` if the entry was removed, else `false`.
   */

  function mapCacheDelete(key) {
    var result = getMapData(this, key)['delete'](key);
    this.size -= result ? 1 : 0;
    return result;
  }

  /**
   * Gets the map value for `key`.
   *
   * @private
   * @name get
   * @memberOf MapCache
   * @param {string} key The key of the value to get.
   * @returns {*} Returns the entry value.
   */

  function mapCacheGet(key) {
    return getMapData(this, key).get(key);
  }

  /**
   * Checks if a map value for `key` exists.
   *
   * @private
   * @name has
   * @memberOf MapCache
   * @param {string} key The key of the entry to check.
   * @returns {boolean} Returns `true` if an entry for `key` exists, else `false`.
   */

  function mapCacheHas(key) {
    return getMapData(this, key).has(key);
  }

  /**
   * Sets the map `key` to `value`.
   *
   * @private
   * @name set
   * @memberOf MapCache
   * @param {string} key The key of the value to set.
   * @param {*} value The value to set.
   * @returns {Object} Returns the map cache instance.
   */

  function mapCacheSet(key, value) {
    var data = getMapData(this, key),
        size = data.size;
    data.set(key, value);
    this.size += data.size == size ? 0 : 1;
    return this;
  }

  /**
   * Creates a map cache object to store key-value pairs.
   *
   * @private
   * @constructor
   * @param {Array} [entries] The key-value pairs to cache.
   */

  function MapCache(entries) {
    var index = -1,
        length = entries == null ? 0 : entries.length;
    this.clear();

    while (++index < length) {
      var entry = entries[index];
      this.set(entry[0], entry[1]);
    }
  } // Add methods to `MapCache`.


  MapCache.prototype.clear = mapCacheClear;
  MapCache.prototype['delete'] = mapCacheDelete;
  MapCache.prototype.get = mapCacheGet;
  MapCache.prototype.has = mapCacheHas;
  MapCache.prototype.set = mapCacheSet;

  /** Used as the size to enable large array optimizations. */

  var LARGE_ARRAY_SIZE = 200;
  /**
   * Sets the stack `key` to `value`.
   *
   * @private
   * @name set
   * @memberOf Stack
   * @param {string} key The key of the value to set.
   * @param {*} value The value to set.
   * @returns {Object} Returns the stack cache instance.
   */

  function stackSet(key, value) {
    var data = this.__data__;

    if (data instanceof ListCache) {
      var pairs = data.__data__;

      if (!Map || pairs.length < LARGE_ARRAY_SIZE - 1) {
        pairs.push([key, value]);
        this.size = ++data.size;
        return this;
      }

      data = this.__data__ = new MapCache(pairs);
    }

    data.set(key, value);
    this.size = data.size;
    return this;
  }

  /**
   * Creates a stack cache object to store key-value pairs.
   *
   * @private
   * @constructor
   * @param {Array} [entries] The key-value pairs to cache.
   */

  function Stack(entries) {
    var data = this.__data__ = new ListCache(entries);
    this.size = data.size;
  } // Add methods to `Stack`.


  Stack.prototype.clear = stackClear;
  Stack.prototype['delete'] = stackDelete;
  Stack.prototype.get = stackGet;
  Stack.prototype.has = stackHas;
  Stack.prototype.set = stackSet;

  /** Used to stand-in for `undefined` hash values. */
  var HASH_UNDEFINED$2 = '__lodash_hash_undefined__';
  /**
   * Adds `value` to the array cache.
   *
   * @private
   * @name add
   * @memberOf SetCache
   * @alias push
   * @param {*} value The value to cache.
   * @returns {Object} Returns the cache instance.
   */

  function setCacheAdd(value) {
    this.__data__.set(value, HASH_UNDEFINED$2);

    return this;
  }

  /**
   * Checks if `value` is in the array cache.
   *
   * @private
   * @name has
   * @memberOf SetCache
   * @param {*} value The value to search for.
   * @returns {number} Returns `true` if `value` is found, else `false`.
   */
  function setCacheHas(value) {
    return this.__data__.has(value);
  }

  /**
   *
   * Creates an array cache object to store unique values.
   *
   * @private
   * @constructor
   * @param {Array} [values] The values to cache.
   */

  function SetCache(values) {
    var index = -1,
        length = values == null ? 0 : values.length;
    this.__data__ = new MapCache();

    while (++index < length) {
      this.add(values[index]);
    }
  } // Add methods to `SetCache`.


  SetCache.prototype.add = SetCache.prototype.push = setCacheAdd;
  SetCache.prototype.has = setCacheHas;

  /**
   * A specialized version of `_.some` for arrays without support for iteratee
   * shorthands.
   *
   * @private
   * @param {Array} [array] The array to iterate over.
   * @param {Function} predicate The function invoked per iteration.
   * @returns {boolean} Returns `true` if any element passes the predicate check,
   *  else `false`.
   */
  function arraySome(array, predicate) {
    var index = -1,
        length = array == null ? 0 : array.length;

    while (++index < length) {
      if (predicate(array[index], index, array)) {
        return true;
      }
    }

    return false;
  }

  /**
   * Checks if a `cache` value for `key` exists.
   *
   * @private
   * @param {Object} cache The cache to query.
   * @param {string} key The key of the entry to check.
   * @returns {boolean} Returns `true` if an entry for `key` exists, else `false`.
   */
  function cacheHas(cache, key) {
    return cache.has(key);
  }

  /** Used to compose bitmasks for value comparisons. */

  var COMPARE_PARTIAL_FLAG = 1,
      COMPARE_UNORDERED_FLAG = 2;
  /**
   * A specialized version of `baseIsEqualDeep` for arrays with support for
   * partial deep comparisons.
   *
   * @private
   * @param {Array} array The array to compare.
   * @param {Array} other The other array to compare.
   * @param {number} bitmask The bitmask flags. See `baseIsEqual` for more details.
   * @param {Function} customizer The function to customize comparisons.
   * @param {Function} equalFunc The function to determine equivalents of values.
   * @param {Object} stack Tracks traversed `array` and `other` objects.
   * @returns {boolean} Returns `true` if the arrays are equivalent, else `false`.
   */

  function equalArrays(array, other, bitmask, customizer, equalFunc, stack) {
    var isPartial = bitmask & COMPARE_PARTIAL_FLAG,
        arrLength = array.length,
        othLength = other.length;

    if (arrLength != othLength && !(isPartial && othLength > arrLength)) {
      return false;
    } // Assume cyclic values are equal.


    var stacked = stack.get(array);

    if (stacked && stack.get(other)) {
      return stacked == other;
    }

    var index = -1,
        result = true,
        seen = bitmask & COMPARE_UNORDERED_FLAG ? new SetCache() : undefined;
    stack.set(array, other);
    stack.set(other, array); // Ignore non-index properties.

    while (++index < arrLength) {
      var arrValue = array[index],
          othValue = other[index];

      if (customizer) {
        var compared = isPartial ? customizer(othValue, arrValue, index, other, array, stack) : customizer(arrValue, othValue, index, array, other, stack);
      }

      if (compared !== undefined) {
        if (compared) {
          continue;
        }

        result = false;
        break;
      } // Recursively compare arrays (susceptible to call stack limits).


      if (seen) {
        if (!arraySome(other, function (othValue, othIndex) {
          if (!cacheHas(seen, othIndex) && (arrValue === othValue || equalFunc(arrValue, othValue, bitmask, customizer, stack))) {
            return seen.push(othIndex);
          }
        })) {
          result = false;
          break;
        }
      } else if (!(arrValue === othValue || equalFunc(arrValue, othValue, bitmask, customizer, stack))) {
        result = false;
        break;
      }
    }

    stack['delete'](array);
    stack['delete'](other);
    return result;
  }

  /** Built-in value references. */

  var Uint8Array = root$1.Uint8Array;

  /**
   * Converts `map` to its key-value pairs.
   *
   * @private
   * @param {Object} map The map to convert.
   * @returns {Array} Returns the key-value pairs.
   */
  function mapToArray(map) {
    var index = -1,
        result = Array(map.size);
    map.forEach(function (value, key) {
      result[++index] = [key, value];
    });
    return result;
  }

  /**
   * Converts `set` to an array of its values.
   *
   * @private
   * @param {Object} set The set to convert.
   * @returns {Array} Returns the values.
   */
  function setToArray(set) {
    var index = -1,
        result = Array(set.size);
    set.forEach(function (value) {
      result[++index] = value;
    });
    return result;
  }

  /** Used to compose bitmasks for value comparisons. */

  var COMPARE_PARTIAL_FLAG$1 = 1,
      COMPARE_UNORDERED_FLAG$1 = 2;
  /** `Object#toString` result references. */

  var boolTag$2 = '[object Boolean]',
      dateTag$2 = '[object Date]',
      errorTag$2 = '[object Error]',
      mapTag$2 = '[object Map]',
      numberTag$2 = '[object Number]',
      regexpTag$2 = '[object RegExp]',
      setTag$2 = '[object Set]',
      stringTag$2 = '[object String]',
      symbolTag$1 = '[object Symbol]';
  var arrayBufferTag$2 = '[object ArrayBuffer]',
      dataViewTag$2 = '[object DataView]';
  /** Used to convert symbols to primitives and strings. */

  var symbolProto = _Symbol$1 ? _Symbol$1.prototype : undefined,
      symbolValueOf = symbolProto ? symbolProto.valueOf : undefined;
  /**
   * A specialized version of `baseIsEqualDeep` for comparing objects of
   * the same `toStringTag`.
   *
   * **Note:** This function only supports comparing values with tags of
   * `Boolean`, `Date`, `Error`, `Number`, `RegExp`, or `String`.
   *
   * @private
   * @param {Object} object The object to compare.
   * @param {Object} other The other object to compare.
   * @param {string} tag The `toStringTag` of the objects to compare.
   * @param {number} bitmask The bitmask flags. See `baseIsEqual` for more details.
   * @param {Function} customizer The function to customize comparisons.
   * @param {Function} equalFunc The function to determine equivalents of values.
   * @param {Object} stack Tracks traversed `object` and `other` objects.
   * @returns {boolean} Returns `true` if the objects are equivalent, else `false`.
   */

  function equalByTag(object, other, tag, bitmask, customizer, equalFunc, stack) {
    switch (tag) {
      case dataViewTag$2:
        if (object.byteLength != other.byteLength || object.byteOffset != other.byteOffset) {
          return false;
        }

        object = object.buffer;
        other = other.buffer;

      case arrayBufferTag$2:
        if (object.byteLength != other.byteLength || !equalFunc(new Uint8Array(object), new Uint8Array(other))) {
          return false;
        }

        return true;

      case boolTag$2:
      case dateTag$2:
      case numberTag$2:
        // Coerce booleans to `1` or `0` and dates to milliseconds.
        // Invalid dates are coerced to `NaN`.
        return eq$1(+object, +other);

      case errorTag$2:
        return object.name == other.name && object.message == other.message;

      case regexpTag$2:
      case stringTag$2:
        // Coerce regexes to strings and treat strings, primitives and objects,
        // as equal. See http://www.ecma-international.org/ecma-262/7.0/#sec-regexp.prototype.tostring
        // for more details.
        return object == other + '';

      case mapTag$2:
        var convert = mapToArray;

      case setTag$2:
        var isPartial = bitmask & COMPARE_PARTIAL_FLAG$1;
        convert || (convert = setToArray);

        if (object.size != other.size && !isPartial) {
          return false;
        } // Assume cyclic values are equal.


        var stacked = stack.get(object);

        if (stacked) {
          return stacked == other;
        }

        bitmask |= COMPARE_UNORDERED_FLAG$1; // Recursively compare objects (susceptible to call stack limits).

        stack.set(object, other);
        var result = equalArrays(convert(object), convert(other), bitmask, customizer, equalFunc, stack);
        stack['delete'](object);
        return result;

      case symbolTag$1:
        if (symbolValueOf) {
          return symbolValueOf.call(object) == symbolValueOf.call(other);
        }

    }

    return false;
  }

  /**
   * Appends the elements of `values` to `array`.
   *
   * @private
   * @param {Array} array The array to modify.
   * @param {Array} values The values to append.
   * @returns {Array} Returns `array`.
   */
  function arrayPush(array, values) {
    var index = -1,
        length = values.length,
        offset = array.length;

    while (++index < length) {
      array[offset + index] = values[index];
    }

    return array;
  }

  /**
   * The base implementation of `getAllKeys` and `getAllKeysIn` which uses
   * `keysFunc` and `symbolsFunc` to get the enumerable property names and
   * symbols of `object`.
   *
   * @private
   * @param {Object} object The object to query.
   * @param {Function} keysFunc The function to get the keys of `object`.
   * @param {Function} symbolsFunc The function to get the symbols of `object`.
   * @returns {Array} Returns the array of property names and symbols.
   */

  function baseGetAllKeys(object, keysFunc, symbolsFunc) {
    var result = keysFunc(object);
    return isArray$1(object) ? result : arrayPush(result, symbolsFunc(object));
  }

  /**
   * A specialized version of `_.filter` for arrays without support for
   * iteratee shorthands.
   *
   * @private
   * @param {Array} [array] The array to iterate over.
   * @param {Function} predicate The function invoked per iteration.
   * @returns {Array} Returns the new filtered array.
   */
  function arrayFilter(array, predicate) {
    var index = -1,
        length = array == null ? 0 : array.length,
        resIndex = 0,
        result = [];

    while (++index < length) {
      var value = array[index];

      if (predicate(value, index, array)) {
        result[resIndex++] = value;
      }
    }

    return result;
  }

  /**
   * This method returns a new empty array.
   *
   * @static
   * @memberOf _
   * @since 4.13.0
   * @category Util
   * @returns {Array} Returns the new empty array.
   * @example
   *
   * var arrays = _.times(2, _.stubArray);
   *
   * console.log(arrays);
   * // => [[], []]
   *
   * console.log(arrays[0] === arrays[1]);
   * // => false
   */
  function stubArray() {
    return [];
  }

  /** Used for built-in method references. */

  var objectProto$k = Object.prototype;
  /** Built-in value references. */

  var propertyIsEnumerable$2 = objectProto$k.propertyIsEnumerable;
  /* Built-in method references for those with the same name as other `lodash` methods. */

  var nativeGetSymbols = Object.getOwnPropertySymbols;
  /**
   * Creates an array of the own enumerable symbols of `object`.
   *
   * @private
   * @param {Object} object The object to query.
   * @returns {Array} Returns the array of symbols.
   */

  var getSymbols = !nativeGetSymbols ? stubArray : function (object) {
    if (object == null) {
      return [];
    }

    object = Object(object);
    return arrayFilter(nativeGetSymbols(object), function (symbol) {
      return propertyIsEnumerable$2.call(object, symbol);
    });
  };

  /**
   * Creates an array of own enumerable property names and symbols of `object`.
   *
   * @private
   * @param {Object} object The object to query.
   * @returns {Array} Returns the array of property names and symbols.
   */

  function getAllKeys(object) {
    return baseGetAllKeys(object, keys$1, getSymbols);
  }

  /** Used to compose bitmasks for value comparisons. */

  var COMPARE_PARTIAL_FLAG$2 = 1;
  /** Used for built-in method references. */

  var objectProto$l = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$g = objectProto$l.hasOwnProperty;
  /**
   * A specialized version of `baseIsEqualDeep` for objects with support for
   * partial deep comparisons.
   *
   * @private
   * @param {Object} object The object to compare.
   * @param {Object} other The other object to compare.
   * @param {number} bitmask The bitmask flags. See `baseIsEqual` for more details.
   * @param {Function} customizer The function to customize comparisons.
   * @param {Function} equalFunc The function to determine equivalents of values.
   * @param {Object} stack Tracks traversed `object` and `other` objects.
   * @returns {boolean} Returns `true` if the objects are equivalent, else `false`.
   */

  function equalObjects(object, other, bitmask, customizer, equalFunc, stack) {
    var isPartial = bitmask & COMPARE_PARTIAL_FLAG$2,
        objProps = getAllKeys(object),
        objLength = objProps.length,
        othProps = getAllKeys(other),
        othLength = othProps.length;

    if (objLength != othLength && !isPartial) {
      return false;
    }

    var index = objLength;

    while (index--) {
      var key = objProps[index];

      if (!(isPartial ? key in other : hasOwnProperty$g.call(other, key))) {
        return false;
      }
    } // Assume cyclic values are equal.


    var stacked = stack.get(object);

    if (stacked && stack.get(other)) {
      return stacked == other;
    }

    var result = true;
    stack.set(object, other);
    stack.set(other, object);
    var skipCtor = isPartial;

    while (++index < objLength) {
      key = objProps[index];
      var objValue = object[key],
          othValue = other[key];

      if (customizer) {
        var compared = isPartial ? customizer(othValue, objValue, key, other, object, stack) : customizer(objValue, othValue, key, object, other, stack);
      } // Recursively compare objects (susceptible to call stack limits).


      if (!(compared === undefined ? objValue === othValue || equalFunc(objValue, othValue, bitmask, customizer, stack) : compared)) {
        result = false;
        break;
      }

      skipCtor || (skipCtor = key == 'constructor');
    }

    if (result && !skipCtor) {
      var objCtor = object.constructor,
          othCtor = other.constructor; // Non `Object` object instances with different constructors are not equal.

      if (objCtor != othCtor && 'constructor' in object && 'constructor' in other && !(typeof objCtor == 'function' && objCtor instanceof objCtor && typeof othCtor == 'function' && othCtor instanceof othCtor)) {
        result = false;
      }
    }

    stack['delete'](object);
    stack['delete'](other);
    return result;
  }

  /* Built-in method references that are verified to be native. */

  var DataView = getNative$1(root$1, 'DataView');

  /* Built-in method references that are verified to be native. */

  var Promise = getNative$1(root$1, 'Promise');

  /* Built-in method references that are verified to be native. */

  var Set = getNative$1(root$1, 'Set');

  /* Built-in method references that are verified to be native. */

  var WeakMap = getNative$1(root$1, 'WeakMap');

  /** `Object#toString` result references. */

  var mapTag$3 = '[object Map]',
      objectTag$2 = '[object Object]',
      promiseTag = '[object Promise]',
      setTag$3 = '[object Set]',
      weakMapTag$2 = '[object WeakMap]';
  var dataViewTag$3 = '[object DataView]';
  /** Used to detect maps, sets, and weakmaps. */

  var dataViewCtorString = toSource$1(DataView),
      mapCtorString = toSource$1(Map),
      promiseCtorString = toSource$1(Promise),
      setCtorString = toSource$1(Set),
      weakMapCtorString = toSource$1(WeakMap);
  /**
   * Gets the `toStringTag` of `value`.
   *
   * @private
   * @param {*} value The value to query.
   * @returns {string} Returns the `toStringTag`.
   */

  var getTag = baseGetTag$1; // Fallback for data views, maps, sets, and weak maps in IE 11 and promises in Node.js < 6.

  if (DataView && getTag(new DataView(new ArrayBuffer(1))) != dataViewTag$3 || Map && getTag(new Map()) != mapTag$3 || Promise && getTag(Promise.resolve()) != promiseTag || Set && getTag(new Set()) != setTag$3 || WeakMap && getTag(new WeakMap()) != weakMapTag$2) {
    getTag = function getTag(value) {
      var result = baseGetTag$1(value),
          Ctor = result == objectTag$2 ? value.constructor : undefined,
          ctorString = Ctor ? toSource$1(Ctor) : '';

      if (ctorString) {
        switch (ctorString) {
          case dataViewCtorString:
            return dataViewTag$3;

          case mapCtorString:
            return mapTag$3;

          case promiseCtorString:
            return promiseTag;

          case setCtorString:
            return setTag$3;

          case weakMapCtorString:
            return weakMapTag$2;
        }
      }

      return result;
    };
  }

  var getTag$1 = getTag;

  /** Used to compose bitmasks for value comparisons. */

  var COMPARE_PARTIAL_FLAG$3 = 1;
  /** `Object#toString` result references. */

  var argsTag$4 = '[object Arguments]',
      arrayTag$2 = '[object Array]',
      objectTag$3 = '[object Object]';
  /** Used for built-in method references. */

  var objectProto$m = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$h = objectProto$m.hasOwnProperty;
  /**
   * A specialized version of `baseIsEqual` for arrays and objects which performs
   * deep comparisons and tracks traversed objects enabling objects with circular
   * references to be compared.
   *
   * @private
   * @param {Object} object The object to compare.
   * @param {Object} other The other object to compare.
   * @param {number} bitmask The bitmask flags. See `baseIsEqual` for more details.
   * @param {Function} customizer The function to customize comparisons.
   * @param {Function} equalFunc The function to determine equivalents of values.
   * @param {Object} [stack] Tracks traversed `object` and `other` objects.
   * @returns {boolean} Returns `true` if the objects are equivalent, else `false`.
   */

  function baseIsEqualDeep(object, other, bitmask, customizer, equalFunc, stack) {
    var objIsArr = isArray$1(object),
        othIsArr = isArray$1(other),
        objTag = objIsArr ? arrayTag$2 : getTag$1(object),
        othTag = othIsArr ? arrayTag$2 : getTag$1(other);
    objTag = objTag == argsTag$4 ? objectTag$3 : objTag;
    othTag = othTag == argsTag$4 ? objectTag$3 : othTag;
    var objIsObj = objTag == objectTag$3,
        othIsObj = othTag == objectTag$3,
        isSameTag = objTag == othTag;

    if (isSameTag && isBuffer$1(object)) {
      if (!isBuffer$1(other)) {
        return false;
      }

      objIsArr = true;
      objIsObj = false;
    }

    if (isSameTag && !objIsObj) {
      stack || (stack = new Stack());
      return objIsArr || isTypedArray$1(object) ? equalArrays(object, other, bitmask, customizer, equalFunc, stack) : equalByTag(object, other, objTag, bitmask, customizer, equalFunc, stack);
    }

    if (!(bitmask & COMPARE_PARTIAL_FLAG$3)) {
      var objIsWrapped = objIsObj && hasOwnProperty$h.call(object, '__wrapped__'),
          othIsWrapped = othIsObj && hasOwnProperty$h.call(other, '__wrapped__');

      if (objIsWrapped || othIsWrapped) {
        var objUnwrapped = objIsWrapped ? object.value() : object,
            othUnwrapped = othIsWrapped ? other.value() : other;
        stack || (stack = new Stack());
        return equalFunc(objUnwrapped, othUnwrapped, bitmask, customizer, stack);
      }
    }

    if (!isSameTag) {
      return false;
    }

    stack || (stack = new Stack());
    return equalObjects(object, other, bitmask, customizer, equalFunc, stack);
  }

  /**
   * The base implementation of `_.isEqual` which supports partial comparisons
   * and tracks traversed objects.
   *
   * @private
   * @param {*} value The value to compare.
   * @param {*} other The other value to compare.
   * @param {boolean} bitmask The bitmask flags.
   *  1 - Unordered comparison
   *  2 - Partial comparison
   * @param {Function} [customizer] The function to customize comparisons.
   * @param {Object} [stack] Tracks traversed `value` and `other` objects.
   * @returns {boolean} Returns `true` if the values are equivalent, else `false`.
   */

  function baseIsEqual(value, other, bitmask, customizer, stack) {
    if (value === other) {
      return true;
    }

    if (value == null || other == null || !isObjectLike$1(value) && !isObjectLike$1(other)) {
      return value !== value && other !== other;
    }

    return baseIsEqualDeep(value, other, bitmask, customizer, baseIsEqual, stack);
  }

  /** Used to compose bitmasks for value comparisons. */

  var COMPARE_PARTIAL_FLAG$4 = 1,
      COMPARE_UNORDERED_FLAG$2 = 2;
  /**
   * The base implementation of `_.isMatch` without support for iteratee shorthands.
   *
   * @private
   * @param {Object} object The object to inspect.
   * @param {Object} source The object of property values to match.
   * @param {Array} matchData The property names, values, and compare flags to match.
   * @param {Function} [customizer] The function to customize comparisons.
   * @returns {boolean} Returns `true` if `object` is a match, else `false`.
   */

  function baseIsMatch(object, source, matchData, customizer) {
    var index = matchData.length,
        length = index,
        noCustomizer = !customizer;

    if (object == null) {
      return !length;
    }

    object = Object(object);

    while (index--) {
      var data = matchData[index];

      if (noCustomizer && data[2] ? data[1] !== object[data[0]] : !(data[0] in object)) {
        return false;
      }
    }

    while (++index < length) {
      data = matchData[index];
      var key = data[0],
          objValue = object[key],
          srcValue = data[1];

      if (noCustomizer && data[2]) {
        if (objValue === undefined && !(key in object)) {
          return false;
        }
      } else {
        var stack = new Stack();

        if (customizer) {
          var result = customizer(objValue, srcValue, key, object, source, stack);
        }

        if (!(result === undefined ? baseIsEqual(srcValue, objValue, COMPARE_PARTIAL_FLAG$4 | COMPARE_UNORDERED_FLAG$2, customizer, stack) : result)) {
          return false;
        }
      }
    }

    return true;
  }

  /**
   * Checks if `value` is suitable for strict equality comparisons, i.e. `===`.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` if suitable for strict
   *  equality comparisons, else `false`.
   */

  function isStrictComparable(value) {
    return value === value && !isObject$1(value);
  }

  /**
   * Gets the property names, values, and compare flags of `object`.
   *
   * @private
   * @param {Object} object The object to query.
   * @returns {Array} Returns the match data of `object`.
   */

  function getMatchData(object) {
    var result = keys$1(object),
        length = result.length;

    while (length--) {
      var key = result[length],
          value = object[key];
      result[length] = [key, value, isStrictComparable(value)];
    }

    return result;
  }

  /**
   * A specialized version of `matchesProperty` for source values suitable
   * for strict equality comparisons, i.e. `===`.
   *
   * @private
   * @param {string} key The key of the property to get.
   * @param {*} srcValue The value to match.
   * @returns {Function} Returns the new spec function.
   */
  function matchesStrictComparable(key, srcValue) {
    return function (object) {
      if (object == null) {
        return false;
      }

      return object[key] === srcValue && (srcValue !== undefined || key in Object(object));
    };
  }

  /**
   * The base implementation of `_.matches` which doesn't clone `source`.
   *
   * @private
   * @param {Object} source The object of property values to match.
   * @returns {Function} Returns the new spec function.
   */

  function baseMatches(source) {
    var matchData = getMatchData(source);

    if (matchData.length == 1 && matchData[0][2]) {
      return matchesStrictComparable(matchData[0][0], matchData[0][1]);
    }

    return function (object) {
      return object === source || baseIsMatch(object, source, matchData);
    };
  }

  /** `Object#toString` result references. */

  var symbolTag$2 = '[object Symbol]';
  /**
   * Checks if `value` is classified as a `Symbol` primitive or object.
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a symbol, else `false`.
   * @example
   *
   * _.isSymbol(Symbol.iterator);
   * // => true
   *
   * _.isSymbol('abc');
   * // => false
   */

  function isSymbol$1(value) {
    return _typeof(value) == 'symbol' || isObjectLike$1(value) && baseGetTag$1(value) == symbolTag$2;
  }

  /** Used to match property names within property paths. */

  var reIsDeepProp = /\.|\[(?:[^[\]]*|(["'])(?:(?!\1)[^\\]|\\.)*?\1)\]/,
      reIsPlainProp = /^\w*$/;
  /**
   * Checks if `value` is a property name and not a property path.
   *
   * @private
   * @param {*} value The value to check.
   * @param {Object} [object] The object to query keys on.
   * @returns {boolean} Returns `true` if `value` is a property name, else `false`.
   */

  function isKey(value, object) {
    if (isArray$1(value)) {
      return false;
    }

    var type = _typeof(value);

    if (type == 'number' || type == 'symbol' || type == 'boolean' || value == null || isSymbol$1(value)) {
      return true;
    }

    return reIsPlainProp.test(value) || !reIsDeepProp.test(value) || object != null && value in Object(object);
  }

  /** Error message constants. */

  var FUNC_ERROR_TEXT = 'Expected a function';
  /**
   * Creates a function that memoizes the result of `func`. If `resolver` is
   * provided, it determines the cache key for storing the result based on the
   * arguments provided to the memoized function. By default, the first argument
   * provided to the memoized function is used as the map cache key. The `func`
   * is invoked with the `this` binding of the memoized function.
   *
   * **Note:** The cache is exposed as the `cache` property on the memoized
   * function. Its creation may be customized by replacing the `_.memoize.Cache`
   * constructor with one whose instances implement the
   * [`Map`](http://ecma-international.org/ecma-262/7.0/#sec-properties-of-the-map-prototype-object)
   * method interface of `clear`, `delete`, `get`, `has`, and `set`.
   *
   * @static
   * @memberOf _
   * @since 0.1.0
   * @category Function
   * @param {Function} func The function to have its output memoized.
   * @param {Function} [resolver] The function to resolve the cache key.
   * @returns {Function} Returns the new memoized function.
   * @example
   *
   * var object = { 'a': 1, 'b': 2 };
   * var other = { 'c': 3, 'd': 4 };
   *
   * var values = _.memoize(_.values);
   * values(object);
   * // => [1, 2]
   *
   * values(other);
   * // => [3, 4]
   *
   * object.a = 2;
   * values(object);
   * // => [1, 2]
   *
   * // Modify the result cache.
   * values.cache.set(object, ['a', 'b']);
   * values(object);
   * // => ['a', 'b']
   *
   * // Replace `_.memoize.Cache`.
   * _.memoize.Cache = WeakMap;
   */

  function memoize(func, resolver) {
    if (typeof func != 'function' || resolver != null && typeof resolver != 'function') {
      throw new TypeError(FUNC_ERROR_TEXT);
    }

    var memoized = function memoized() {
      var args = arguments,
          key = resolver ? resolver.apply(this, args) : args[0],
          cache = memoized.cache;

      if (cache.has(key)) {
        return cache.get(key);
      }

      var result = func.apply(this, args);
      memoized.cache = cache.set(key, result) || cache;
      return result;
    };

    memoized.cache = new (memoize.Cache || MapCache)();
    return memoized;
  } // Expose `MapCache`.


  memoize.Cache = MapCache;

  /** Used as the maximum memoize cache size. */

  var MAX_MEMOIZE_SIZE = 500;
  /**
   * A specialized version of `_.memoize` which clears the memoized function's
   * cache when it exceeds `MAX_MEMOIZE_SIZE`.
   *
   * @private
   * @param {Function} func The function to have its output memoized.
   * @returns {Function} Returns the new memoized function.
   */

  function memoizeCapped(func) {
    var result = memoize(func, function (key) {
      if (cache.size === MAX_MEMOIZE_SIZE) {
        cache.clear();
      }

      return key;
    });
    var cache = result.cache;
    return result;
  }

  /** Used to match property names within property paths. */

  var rePropName = /[^.[\]]+|\[(?:(-?\d+(?:\.\d+)?)|(["'])((?:(?!\2)[^\\]|\\.)*?)\2)\]|(?=(?:\.|\[\])(?:\.|\[\]|$))/g;
  /** Used to match backslashes in property paths. */

  var reEscapeChar = /\\(\\)?/g;
  /**
   * Converts `string` to a property path array.
   *
   * @private
   * @param {string} string The string to convert.
   * @returns {Array} Returns the property path array.
   */

  var stringToPath = memoizeCapped(function (string) {
    var result = [];

    if (string.charCodeAt(0) === 46
    /* . */
    ) {
        result.push('');
      }

    string.replace(rePropName, function (match, number, quote, subString) {
      result.push(quote ? subString.replace(reEscapeChar, '$1') : number || match);
    });
    return result;
  });

  /**
   * A specialized version of `_.map` for arrays without support for iteratee
   * shorthands.
   *
   * @private
   * @param {Array} [array] The array to iterate over.
   * @param {Function} iteratee The function invoked per iteration.
   * @returns {Array} Returns the new mapped array.
   */
  function arrayMap(array, iteratee) {
    var index = -1,
        length = array == null ? 0 : array.length,
        result = Array(length);

    while (++index < length) {
      result[index] = iteratee(array[index], index, array);
    }

    return result;
  }

  /** Used as references for various `Number` constants. */

  var INFINITY$1 = 1 / 0;
  /** Used to convert symbols to primitives and strings. */

  var symbolProto$1 = _Symbol$1 ? _Symbol$1.prototype : undefined,
      symbolToString = symbolProto$1 ? symbolProto$1.toString : undefined;
  /**
   * The base implementation of `_.toString` which doesn't convert nullish
   * values to empty strings.
   *
   * @private
   * @param {*} value The value to process.
   * @returns {string} Returns the string.
   */

  function baseToString(value) {
    // Exit early for strings to avoid a performance hit in some environments.
    if (typeof value == 'string') {
      return value;
    }

    if (isArray$1(value)) {
      // Recursively convert values (susceptible to call stack limits).
      return arrayMap(value, baseToString) + '';
    }

    if (isSymbol$1(value)) {
      return symbolToString ? symbolToString.call(value) : '';
    }

    var result = value + '';
    return result == '0' && 1 / value == -INFINITY$1 ? '-0' : result;
  }

  /**
   * Converts `value` to a string. An empty string is returned for `null`
   * and `undefined` values. The sign of `-0` is preserved.
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to convert.
   * @returns {string} Returns the converted string.
   * @example
   *
   * _.toString(null);
   * // => ''
   *
   * _.toString(-0);
   * // => '-0'
   *
   * _.toString([1, 2, 3]);
   * // => '1,2,3'
   */

  function toString(value) {
    return value == null ? '' : baseToString(value);
  }

  /**
   * Casts `value` to a path array if it's not one.
   *
   * @private
   * @param {*} value The value to inspect.
   * @param {Object} [object] The object to query keys on.
   * @returns {Array} Returns the cast property path array.
   */

  function castPath(value, object) {
    if (isArray$1(value)) {
      return value;
    }

    return isKey(value, object) ? [value] : stringToPath(toString(value));
  }

  /** Used as references for various `Number` constants. */

  var INFINITY$2 = 1 / 0;
  /**
   * Converts `value` to a string key if it's not a string or symbol.
   *
   * @private
   * @param {*} value The value to inspect.
   * @returns {string|symbol} Returns the key.
   */

  function toKey(value) {
    if (typeof value == 'string' || isSymbol$1(value)) {
      return value;
    }

    var result = value + '';
    return result == '0' && 1 / value == -INFINITY$2 ? '-0' : result;
  }

  /**
   * The base implementation of `_.get` without support for default values.
   *
   * @private
   * @param {Object} object The object to query.
   * @param {Array|string} path The path of the property to get.
   * @returns {*} Returns the resolved value.
   */

  function baseGet(object, path) {
    path = castPath(path, object);
    var index = 0,
        length = path.length;

    while (object != null && index < length) {
      object = object[toKey(path[index++])];
    }

    return index && index == length ? object : undefined;
  }

  /**
   * Gets the value at `path` of `object`. If the resolved value is
   * `undefined`, the `defaultValue` is returned in its place.
   *
   * @static
   * @memberOf _
   * @since 3.7.0
   * @category Object
   * @param {Object} object The object to query.
   * @param {Array|string} path The path of the property to get.
   * @param {*} [defaultValue] The value returned for `undefined` resolved values.
   * @returns {*} Returns the resolved value.
   * @example
   *
   * var object = { 'a': [{ 'b': { 'c': 3 } }] };
   *
   * _.get(object, 'a[0].b.c');
   * // => 3
   *
   * _.get(object, ['a', '0', 'b', 'c']);
   * // => 3
   *
   * _.get(object, 'a.b.c', 'default');
   * // => 'default'
   */

  function get(object, path, defaultValue) {
    var result = object == null ? undefined : baseGet(object, path);
    return result === undefined ? defaultValue : result;
  }

  /**
   * The base implementation of `_.hasIn` without support for deep paths.
   *
   * @private
   * @param {Object} [object] The object to query.
   * @param {Array|string} key The key to check.
   * @returns {boolean} Returns `true` if `key` exists, else `false`.
   */
  function baseHasIn(object, key) {
    return object != null && key in Object(object);
  }

  /**
   * Checks if `path` exists on `object`.
   *
   * @private
   * @param {Object} object The object to query.
   * @param {Array|string} path The path to check.
   * @param {Function} hasFunc The function to check properties.
   * @returns {boolean} Returns `true` if `path` exists, else `false`.
   */

  function hasPath(object, path, hasFunc) {
    path = castPath(path, object);
    var index = -1,
        length = path.length,
        result = false;

    while (++index < length) {
      var key = toKey(path[index]);

      if (!(result = object != null && hasFunc(object, key))) {
        break;
      }

      object = object[key];
    }

    if (result || ++index != length) {
      return result;
    }

    length = object == null ? 0 : object.length;
    return !!length && isLength$1(length) && isIndex$1(key, length) && (isArray$1(object) || isArguments$1(object));
  }

  /**
   * Checks if `path` is a direct or inherited property of `object`.
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Object
   * @param {Object} object The object to query.
   * @param {Array|string} path The path to check.
   * @returns {boolean} Returns `true` if `path` exists, else `false`.
   * @example
   *
   * var object = _.create({ 'a': _.create({ 'b': 2 }) });
   *
   * _.hasIn(object, 'a');
   * // => true
   *
   * _.hasIn(object, 'a.b');
   * // => true
   *
   * _.hasIn(object, ['a', 'b']);
   * // => true
   *
   * _.hasIn(object, 'b');
   * // => false
   */

  function hasIn(object, path) {
    return object != null && hasPath(object, path, baseHasIn);
  }

  /** Used to compose bitmasks for value comparisons. */

  var COMPARE_PARTIAL_FLAG$5 = 1,
      COMPARE_UNORDERED_FLAG$3 = 2;
  /**
   * The base implementation of `_.matchesProperty` which doesn't clone `srcValue`.
   *
   * @private
   * @param {string} path The path of the property to get.
   * @param {*} srcValue The value to match.
   * @returns {Function} Returns the new spec function.
   */

  function baseMatchesProperty(path, srcValue) {
    if (isKey(path) && isStrictComparable(srcValue)) {
      return matchesStrictComparable(toKey(path), srcValue);
    }

    return function (object) {
      var objValue = get(object, path);
      return objValue === undefined && objValue === srcValue ? hasIn(object, path) : baseIsEqual(srcValue, objValue, COMPARE_PARTIAL_FLAG$5 | COMPARE_UNORDERED_FLAG$3);
    };
  }

  /**
   * The base implementation of `_.property` without support for deep paths.
   *
   * @private
   * @param {string} key The key of the property to get.
   * @returns {Function} Returns the new accessor function.
   */
  function baseProperty(key) {
    return function (object) {
      return object == null ? undefined : object[key];
    };
  }

  /**
   * A specialized version of `baseProperty` which supports deep paths.
   *
   * @private
   * @param {Array|string} path The path of the property to get.
   * @returns {Function} Returns the new accessor function.
   */

  function basePropertyDeep(path) {
    return function (object) {
      return baseGet(object, path);
    };
  }

  /**
   * Creates a function that returns the value at `path` of a given object.
   *
   * @static
   * @memberOf _
   * @since 2.4.0
   * @category Util
   * @param {Array|string} path The path of the property to get.
   * @returns {Function} Returns the new accessor function.
   * @example
   *
   * var objects = [
   *   { 'a': { 'b': 2 } },
   *   { 'a': { 'b': 1 } }
   * ];
   *
   * _.map(objects, _.property('a.b'));
   * // => [2, 1]
   *
   * _.map(_.sortBy(objects, _.property(['a', 'b'])), 'a.b');
   * // => [1, 2]
   */

  function property(path) {
    return isKey(path) ? baseProperty(toKey(path)) : basePropertyDeep(path);
  }

  /**
   * The base implementation of `_.iteratee`.
   *
   * @private
   * @param {*} [value=_.identity] The value to convert to an iteratee.
   * @returns {Function} Returns the iteratee.
   */

  function baseIteratee(value) {
    // Don't store the `typeof` result in a variable to avoid a JIT bug in Safari 9.
    // See https://bugs.webkit.org/show_bug.cgi?id=156034 for more details.
    if (typeof value == 'function') {
      return value;
    }

    if (value == null) {
      return identity$1;
    }

    if (_typeof(value) == 'object') {
      return isArray$1(value) ? baseMatchesProperty(value[0], value[1]) : baseMatches(value);
    }

    return property(value);
  }

  /** Used as references for various `Number` constants. */

  var NAN$1 = 0 / 0;
  /** Used to match leading and trailing whitespace. */

  var reTrim$1 = /^\s+|\s+$/g;
  /** Used to detect bad signed hexadecimal string values. */

  var reIsBadHex$1 = /^[-+]0x[0-9a-f]+$/i;
  /** Used to detect binary string values. */

  var reIsBinary$1 = /^0b[01]+$/i;
  /** Used to detect octal string values. */

  var reIsOctal$1 = /^0o[0-7]+$/i;
  /** Built-in method references without a dependency on `root`. */

  var freeParseInt$1 = parseInt;
  /**
   * Converts `value` to a number.
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to process.
   * @returns {number} Returns the number.
   * @example
   *
   * _.toNumber(3.2);
   * // => 3.2
   *
   * _.toNumber(Number.MIN_VALUE);
   * // => 5e-324
   *
   * _.toNumber(Infinity);
   * // => Infinity
   *
   * _.toNumber('3.2');
   * // => 3.2
   */

  function toNumber$1(value) {
    if (typeof value == 'number') {
      return value;
    }

    if (isSymbol$1(value)) {
      return NAN$1;
    }

    if (isObject$1(value)) {
      var other = typeof value.valueOf == 'function' ? value.valueOf() : value;
      value = isObject$1(other) ? other + '' : other;
    }

    if (typeof value != 'string') {
      return value === 0 ? value : +value;
    }

    value = value.replace(reTrim$1, '');
    var isBinary = reIsBinary$1.test(value);
    return isBinary || reIsOctal$1.test(value) ? freeParseInt$1(value.slice(2), isBinary ? 2 : 8) : reIsBadHex$1.test(value) ? NAN$1 : +value;
  }

  /** Used as references for various `Number` constants. */

  var INFINITY$3 = 1 / 0,
      MAX_INTEGER$1 = 1.7976931348623157e+308;
  /**
   * Converts `value` to a finite number.
   *
   * @static
   * @memberOf _
   * @since 4.12.0
   * @category Lang
   * @param {*} value The value to convert.
   * @returns {number} Returns the converted number.
   * @example
   *
   * _.toFinite(3.2);
   * // => 3.2
   *
   * _.toFinite(Number.MIN_VALUE);
   * // => 5e-324
   *
   * _.toFinite(Infinity);
   * // => 1.7976931348623157e+308
   *
   * _.toFinite('3.2');
   * // => 3.2
   */

  function toFinite$1(value) {
    if (!value) {
      return value === 0 ? value : 0;
    }

    value = toNumber$1(value);

    if (value === INFINITY$3 || value === -INFINITY$3) {
      var sign = value < 0 ? -1 : 1;
      return sign * MAX_INTEGER$1;
    }

    return value === value ? value : 0;
  }

  /**
   * Converts `value` to an integer.
   *
   * **Note:** This method is loosely based on
   * [`ToInteger`](http://www.ecma-international.org/ecma-262/7.0/#sec-tointeger).
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to convert.
   * @returns {number} Returns the converted integer.
   * @example
   *
   * _.toInteger(3.2);
   * // => 3
   *
   * _.toInteger(Number.MIN_VALUE);
   * // => 0
   *
   * _.toInteger(Infinity);
   * // => 1.7976931348623157e+308
   *
   * _.toInteger('3.2');
   * // => 3
   */

  function toInteger(value) {
    var result = toFinite$1(value),
        remainder = result % 1;
    return result === result ? remainder ? result - remainder : result : 0;
  }

  /* Built-in method references for those with the same name as other `lodash` methods. */

  var nativeMax$3 = Math.max;
  /**
   * This method is like `_.find` except that it returns the index of the first
   * element `predicate` returns truthy for instead of the element itself.
   *
   * @static
   * @memberOf _
   * @since 1.1.0
   * @category Array
   * @param {Array} array The array to inspect.
   * @param {Function} [predicate=_.identity] The function invoked per iteration.
   * @param {number} [fromIndex=0] The index to search from.
   * @returns {number} Returns the index of the found element, else `-1`.
   * @example
   *
   * var users = [
   *   { 'user': 'barney',  'active': false },
   *   { 'user': 'fred',    'active': false },
   *   { 'user': 'pebbles', 'active': true }
   * ];
   *
   * _.findIndex(users, function(o) { return o.user == 'barney'; });
   * // => 0
   *
   * // The `_.matches` iteratee shorthand.
   * _.findIndex(users, { 'user': 'fred', 'active': false });
   * // => 1
   *
   * // The `_.matchesProperty` iteratee shorthand.
   * _.findIndex(users, ['active', false]);
   * // => 0
   *
   * // The `_.property` iteratee shorthand.
   * _.findIndex(users, 'active');
   * // => 2
   */

  function findIndex(array, predicate, fromIndex) {
    var length = array == null ? 0 : array.length;

    if (!length) {
      return -1;
    }

    var index = fromIndex == null ? 0 : toInteger(fromIndex);

    if (index < 0) {
      index = nativeMax$3(length + index, 0);
    }

    return baseFindIndex(array, baseIteratee(predicate), index);
  }

  var play = (function (gridPosition) {
    return function (deck) {
      return function (modifier) {
        return function (device) {
          return {
            bindings: {
              playIndicator: {
                type: 'control',
                target: deck.play_indicator,
                update: function update(_ref, _ref2) {
                  var value = _ref.value;
                  var bindings = _ref2.bindings;

                  if (value) {
                    bindings.play.button.sendColor(device.colors.hi_red);
                  } else if (!value) {
                    bindings.play.button.sendColor(device.colors.black);
                  }
                }
              },
              play: {
                type: 'button',
                target: gridPosition,
                attack: function attack() {
                  modes(modifier.getState(), function () {
                    return deck.play.setValue(Number(!deck.play.getValue()));
                  }, function () {
                    return deck.start_play.setValue(1);
                  }, function () {
                    return deck.start_stop.setValue(1);
                  });
                }
              }
            }
          };
        };
      };
    };
  });

  var sync = (function (gridPosition) {
    return function (deck) {
      return function (modifier) {
        return function (device) {
          return {
            bindings: {
              sync: {
                type: 'button',
                target: gridPosition,
                attack: function attack(message, _ref) {
                  var bindings = _ref.bindings;
                  modes(modifier.getState(), function () {
                    if (bindings.syncMode.getValue()) {
                      deck.sync_enabled.setValue(0);
                    } else {
                      deck.sync_enabled.setValue(1);
                    }
                  }, function () {
                    if (bindings.syncMode.getValue() === 2) {
                      deck.sync_leader.setValue(0);
                    } else {
                      deck.sync_leader.setValue(1);
                    }
                  });
                }
              },
              syncMode: {
                type: 'control',
                target: deck.sync_mode,
                update: function update(_ref2, _ref3) {
                  var value = _ref2.value;
                  var bindings = _ref3.bindings;

                  if (value === 0) {
                    bindings.sync.button.sendColor(device.colors.black);
                  } else if (value === 1) {
                    bindings.sync.button.sendColor(device.colors.hi_orange);
                  } else if (value === 2) {
                    bindings.sync.button.sendColor(device.colors.hi_red);
                  }
                }
              }
            }
          };
        };
      };
    };
  });

  var nudge = (function (gridPosition) {
    return function (deck) {
      return function (modifier) {
        return function (device) {
          var rateEpsilon = 1e-3;

          var getDirection = function getDirection(rate) {
            if (rate < -rateEpsilon) {
              return 'up';
            } else if (rate > rateEpsilon) {
              return 'down';
            } else {
              return '';
            }
          };

          var onNudgeMidi = function onNudgeMidi(dir) {
            return function (modifier) {
              return retainAttackMode(modifier, function (mode, _ref, _ref2) {
                var value = _ref.value;
                var bindings = _ref2.bindings,
                    state = _ref2.state;

                if (value) {
                  state[dir] = true;

                  if (state.down && state.up) {
                    deck.rate.setValue(0);
                  } else {
                    modes(mode, function () {
                      bindings[dir].button.sendColor(device.colors.hi_yellow); // TODO: remove unsafe cast once flow supports https://github.com/facebook/flow/issues/3637

                      deck["rate_temp_".concat(dir)].setValue(1);
                    }, function () {
                      bindings[dir].button.sendColor(device.colors.hi_red); // TODO: remove unsafe cast once flow supports https://github.com/facebook/flow/issues/3637

                      deck["rate_perm_".concat(dir)].setValue(1);
                    }, function () {
                      bindings[dir].button.sendColor(device.colors.lo_yellow); // TODO: remove unsafe cast once flow supports https://github.com/facebook/flow/issues/3637

                      deck["rate_temp_".concat(dir, "_small")].setValue(1);
                    }, function () {
                      bindings[dir].button.sendColor(device.colors.lo_red); // TODO: remove unsafe cast once flow supports https://github.com/facebook/flow/issues/3637

                      deck["rate_perm_".concat(dir, "_small")].setValue(1);
                    });
                  }
                } else {
                  state[dir] = false;

                  if (getDirection(bindings.rate.getValue()) === dir) {
                    bindings[dir].button.sendColor(device.colors.lo_orange);
                  } else {
                    bindings[dir].button.sendColor(device.colors.black);
                  }

                  modes(mode, // TODO: remove unsafe cast once flow supports https://github.com/facebook/flow/issues/3637
                  function () {
                    return deck["rate_temp_".concat(dir)].setValue(0);
                  }, undefined, // TODO: remove unsafe cast once flow supports https://github.com/facebook/flow/issues/3637
                  function () {
                    return deck["rate_temp_".concat(dir, "_small")].setValue(0);
                  });
                }
              });
            };
          };

          var onRate = function onRate(_ref3, _ref4) {
            var value = _ref3.value;
            var state = _ref4.state,
                bindings = _ref4.bindings;
            var up = device.colors.black;
            var down = device.colors.black;
            var rate = getDirection(value);

            if (rate === 'down') {
              down = device.colors.lo_orange;
            } else if (rate === 'up') {
              up = device.colors.lo_orange;
            }

            if (!state.down) {
              bindings.down.button.sendColor(down);
            }

            if (!state.up) {
              bindings.up.button.sendColor(up);
            }
          };

          return {
            bindings: {
              down: {
                type: 'button',
                target: gridPosition,
                midi: onNudgeMidi('down')(modifier)
              },
              up: {
                type: 'button',
                target: [gridPosition[0] + 1, gridPosition[1]],
                midi: onNudgeMidi('up')(modifier)
              },
              rate: {
                type: 'control',
                target: deck.rate,
                update: onRate
              }
            },
            state: {
              up: false,
              down: false
            }
          };
        };
      };
    };
  });

  var cue = (function (gridPosition) {
    return function (deck) {
      return function (modifier) {
        return function (device) {
          return {
            bindings: {
              cue: {
                type: 'button',
                target: gridPosition,
                midi: retainAttackMode(modifier, function (mode, _ref) {
                  var value = _ref.value;
                  modes(mode, function () {
                    if (value) {
                      deck.cue_default.setValue(1);
                    } else {
                      deck.cue_default.setValue(0);
                    }
                  }, function () {
                    return value && deck.cue_set.setValue(1);
                  });
                })
              },
              cueIndicator: {
                type: 'control',
                target: deck.cue_indicator,
                update: function update(_ref2, _ref3) {
                  var value = _ref2.value;
                  var bindings = _ref3.bindings;

                  if (value) {
                    bindings.cue.button.sendColor(device.colors.hi_red);
                  } else if (!value) {
                    bindings.cue.button.sendColor(device.colors.black);
                  }
                }
              }
            }
          };
        };
      };
    };
  });

  function _createSuper$6(Derived) { return function () { var Super = _getPrototypeOf(Derived), result; if (_isNativeReflectConstruct$6()) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

  function _isNativeReflectConstruct$6() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Date.prototype.toString.call(Reflect.construct(Date, [], function () {})); return true; } catch (e) { return false; } }

  var Bpm = /*#__PURE__*/function (_EventEmitter) {
    _inherits(Bpm, _EventEmitter);

    var _super = _createSuper$6(Bpm);

    function Bpm(max) {
      var _this;

      _classCallCheck$1(this, Bpm);

      _this = _super.call(this);

      _defineProperty$1(_assertThisInitialized(_this), "tapTime", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "taps", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "max", void 0);

      if (max == null) {
        max = 8;
      }

      _this.tapTime = 0;
      _this.taps = [];
      _this.max = max;
      return _this;
    }

    _createClass$1(Bpm, [{
      key: "reset",
      value: function reset() {
        this.taps = [];
      }
    }, {
      key: "tap",
      value: function tap() {
        var now = Date.now();
        var tapDelta = now - this.tapTime;
        this.tapTime = now;

        if (tapDelta > 2000) {
          // reset if longer than two seconds between taps
          this.taps = [];
        } else {
          this.taps.push(60000 / tapDelta);
          if (this.taps.length > this.max) this.taps.shift(); // Keep the last n samples for averaging

          var sum = 0;
          this.taps.forEach(function (v) {
            sum += v;
          });
          var avg = sum / this.taps.length;
          this.emit('tap', avg);
        }
      }
    }]);

    return Bpm;
  }(eventemitter3);

  var tap = (function (gridPosition) {
    return function (deck) {
      return function (modifier) {
        return function (device) {
          var tempoBpm = new Bpm();
          tempoBpm.on('tap', function (avg) {
            deck.bpm.setValue(avg);
          });
          return {
            bindings: {
              tap: {
                type: 'button',
                target: gridPosition,
                attack: function attack() {
                  modes(modifier.getState(), function () {
                    tempoBpm.tap();
                  }, function () {
                    deck.bpm_tap.setValue(1);
                  }, function () {
                    deck.beats_translate_curpos.setValue(1);
                  }, function () {
                    deck.beats_translate_match_alignment.setValue(1);
                  });
                }
              },
              beat: {
                type: 'control',
                target: deck.beat_active,
                update: function update(_ref, _ref2) {
                  var value = _ref.value;
                  var bindings = _ref2.bindings;

                  if (value) {
                    bindings.tap.button.sendColor(device.colors.hi_red);
                  } else {
                    bindings.tap.button.sendColor(device.colors.black);
                  }
                }
              }
            }
          };
        };
      };
    };
  });

  var grid = (function (gridPosition) {
    return function (deck) {
      return function (modifier) {
        return function (device) {
          var onGrid = function onGrid(dir) {
            return function (_ref, _ref2) {
              var value = _ref.value;
              var bindings = _ref2.bindings,
                  state = _ref2.state;

              if (!value) {
                bindings[dir].button.sendColor(device.colors.black);
              } else {
                modes(modifier.getState(), function () {
                  bindings[dir].button.sendColor(device.colors.hi_yellow);
                  state[dir].normal.setValue(1);
                }, function () {
                  bindings[dir].button.sendColor(device.colors.hi_amber);
                  state[dir].ctrl.setValue(1);
                });
              }
            };
          };

          return {
            bindings: {
              back: {
                type: 'button',
                target: gridPosition,
                midi: onGrid('back')
              },
              forth: {
                type: 'button',
                target: [gridPosition[0] + 1, gridPosition[1]],
                midi: onGrid('forth')
              }
            },
            state: {
              back: {
                normal: deck.beats_translate_earlier,
                ctrl: deck.beats_adjust_slower
              },
              forth: {
                normal: deck.beats_translate_later,
                ctrl: deck.beats_adjust_faster
              }
            }
          };
        };
      };
    };
  });

  var pfl = (function (gridPosition) {
    return function (deck) {
      return function (modifier) {
        return function (device) {
          return function (device) {
            return {
              bindings: {
                pfl: {
                  type: 'control',
                  target: deck.pfl,
                  update: function update(_ref, _ref2) {
                    var value = _ref.value;
                    var bindings = _ref2.bindings;
                    return value ? bindings.button.button.sendColor(device.colors.hi_green) : bindings.button.button.sendColor(device.colors.black);
                  }
                },
                button: {
                  type: 'button',
                  target: gridPosition,
                  attack: function attack(message, _ref3) {
                    var bindings = _ref3.bindings;
                    return modes(modifier.getState(), function () {
                      return bindings.pfl.setValue(Number(!bindings.pfl.getValue()));
                    });
                  }
                }
              }
            };
          };
        };
      };
    };
  });

  var quantize = (function (gridPosition) {
    return function (deck) {
      return function (modifier) {
        return function (device) {
          return {
            bindings: {
              quantize: {
                type: 'control',
                target: deck.quantize,
                update: function update(_ref, _ref2) {
                  var value = _ref.value;
                  var bindings = _ref2.bindings;
                  return value ? bindings.button.button.sendColor(device.colors.hi_orange) : bindings.button.button.sendColor(device.colors.black);
                }
              },
              button: {
                type: 'button',
                target: gridPosition,
                attack: function attack(message, _ref3) {
                  var bindings = _ref3.bindings;
                  return modes(modifier.getState(), function () {
                    return bindings.quantize.setValue(Number(!bindings.quantize.getValue()));
                  });
                }
              }
            }
          };
        };
      };
    };
  });

  var keyshift = (function (shifts, d) {
    return function (gridPosition) {
      return function (deck) {
        return function (modifier) {
          return function (device) {
            var bindings = {};

            var temporaryChange = function temporaryChange(i, value, bindings, state) {
              if (value) {
                var base = state.on === -1 ? deck.key.getValue() : state.base;

                if (state.on !== -1) {
                  bindings[state.on].button.sendColor(device.colors["lo_".concat(state.color[state.set])]);
                }

                bindings[i].button.sendColor(device.colors["hi_".concat(state.color[state.set])]);
                deck.key.setValue((base + shifts[i][state.set]) % 12 + 12);
                state.on = i;
                state.base = base;
              } else {
                if (state.on === i) {
                  bindings[i].button.sendColor(device.colors["lo_".concat(state.color[state.set])]);
                  deck.key.setValue(state.base);
                  state.on = -1;
                }
              }
            };

            var onMidi = function onMidi(i) {
              return function (modifier) {
                return retainAttackMode(modifier, function (mode, _ref, _ref2) {
                  var value = _ref.value;
                  var bindings = _ref2.bindings,
                      state = _ref2.state;
                  modes(mode, function () {
                    return temporaryChange(i, value, bindings, state);
                  }, function () {
                    if (value) {
                      if (state.set === 1) {
                        state.set = 0;

                        for (var _i = 0; _i < shifts.length; ++_i) {
                          bindings[_i].button.sendColor(device.colors["lo_".concat(state.color[state.set])]);
                        }
                      }
                    }
                  }, function () {
                    if (value) {
                      if (state.set === 0) {
                        state.set = 1;

                        for (var _i2 = 0; _i2 < shifts.length; ++_i2) {
                          bindings[_i2].button.sendColor(device.colors["lo_".concat(state.color[state.set])]);
                        }
                      }
                    }
                  });
                });
              };
            };

            shifts.forEach(function (s, i) {
              var dx = i % d;
              var dy = ~~(i / d);
              var position = [gridPosition[0] + dx, gridPosition[1] + dy];
              bindings[i] = {
                type: 'button',
                target: position,
                midi: onMidi(i)(modifier),
                mount: function mount(_, _ref3) {
                  var bindings = _ref3.bindings,
                      state = _ref3.state;
                  bindings[i].button.sendColor(device.colors["lo_".concat(state.color[state.set])]);
                }
              };
            });
            return {
              bindings: bindings,
              state: {
                on: -1,
                base: null,
                set: 0,
                color: ['green', 'red']
              }
            };
          };
        };
      };
    };
  });

  /* Built-in method references for those with the same name as other `lodash` methods. */
  var nativeCeil$1 = Math.ceil,
      nativeMax$4 = Math.max;
  /**
   * The base implementation of `_.range` and `_.rangeRight` which doesn't
   * coerce arguments.
   *
   * @private
   * @param {number} start The start of the range.
   * @param {number} end The end of the range.
   * @param {number} step The value to increment or decrement by.
   * @param {boolean} [fromRight] Specify iterating from right to left.
   * @returns {Array} Returns the range of numbers.
   */

  function baseRange$1(start, end, step, fromRight) {
    var index = -1,
        length = nativeMax$4(nativeCeil$1((end - start) / (step || 1)), 0),
        result = Array(length);

    while (length--) {
      result[fromRight ? length : ++index] = start;
      start += step;
    }

    return result;
  }

  /**
   * Creates a `_.range` or `_.rangeRight` function.
   *
   * @private
   * @param {boolean} [fromRight] Specify iterating from right to left.
   * @returns {Function} Returns the new range function.
   */

  function createRange$1(fromRight) {
    return function (start, end, step) {
      if (step && typeof step != 'number' && isIterateeCall$1(start, end, step)) {
        end = step = undefined;
      } // Ensure the sign of `-0` is preserved.


      start = toFinite$1(start);

      if (end === undefined) {
        end = start;
        start = 0;
      } else {
        end = toFinite$1(end);
      }

      step = step === undefined ? start < end ? 1 : -1 : toFinite$1(step);
      return baseRange$1(start, end, step, fromRight);
    };
  }

  /**
   * Creates an array of numbers (positive and/or negative) progressing from
   * `start` up to, but not including, `end`. A step of `-1` is used if a negative
   * `start` is specified without an `end` or `step`. If `end` is not specified,
   * it's set to `start` with `start` then set to `0`.
   *
   * **Note:** JavaScript follows the IEEE-754 standard for resolving
   * floating-point values which can produce unexpected results.
   *
   * @static
   * @since 0.1.0
   * @memberOf _
   * @category Util
   * @param {number} [start=0] The start of the range.
   * @param {number} end The end of the range.
   * @param {number} [step=1] The value to increment or decrement by.
   * @returns {Array} Returns the range of numbers.
   * @see _.inRange, _.rangeRight
   * @example
   *
   * _.range(4);
   * // => [0, 1, 2, 3]
   *
   * _.range(-4);
   * // => [0, -1, -2, -3]
   *
   * _.range(1, 5);
   * // => [1, 2, 3, 4]
   *
   * _.range(0, 20, 5);
   * // => [0, 5, 10, 15]
   *
   * _.range(0, -4, -1);
   * // => [0, -1, -2, -3]
   *
   * _.range(1, 4, 0);
   * // => [1, 1, 1]
   *
   * _.range(0);
   * // => []
   */

  var range$1 = createRange$1();

  var hotcue = (function (n, d) {
    var s = arguments.length > 2 && arguments[2] !== undefined ? arguments[2] : 0;
    return function (gridPosition) {
      return function (deck) {
        return function (modifier) {
          return function (device) {
            var onHotcueMidi = function onHotcueMidi(i) {
              return function (_ref, _ref2) {
                var value = _ref.value;
                var bindings = _ref2.bindings;
                modes(modifier.getState(), function () {
                  if (value) {
                    deck.hotcues[1 + i + s].activate.setValue(1);
                  } else {
                    deck.hotcues[1 + i + s].activate.setValue(0);
                  }
                }, function () {
                  if (value) {
                    if (bindings["".concat(i, ".enabled")].getValue()) {
                      deck.hotcues[1 + i + s].clear.setValue(1);
                    } else {
                      deck.hotcues[1 + i + s].set.setValue(1);
                    }
                  }
                });
              };
            };

            var onHotcueEnabled = function onHotcueEnabled(i) {
              return function (_ref3, _ref4) {
                var value = _ref3.value;
                var bindings = _ref4.bindings;

                if (value) {
                  bindings["".concat(i, ".btn")].button.sendColor(device.colors.lo_yellow);
                } else {
                  bindings["".concat(i, ".btn")].button.sendColor(device.colors.black);
                }
              };
            };

            var bindings = {};
            range$1(n).map(function (i) {
              var dx = i % d;
              var dy = ~~(i / d);
              bindings["".concat(i, ".btn")] = {
                type: 'button',
                target: [gridPosition[0] + dx, gridPosition[1] + dy],
                midi: onHotcueMidi(i)
              };
              bindings["".concat(i, ".enabled")] = {
                type: 'control',
                target: deck.hotcues[1 + i + s].enabled,
                update: onHotcueEnabled(i)
              };
            });
            return {
              bindings: bindings
            };
          };
        };
      };
    };
  });

  var load = (function (gridPosition) {
    return function (deck) {
      return function (modifier) {
        return function (device) {
          var onStateChanged = function onStateChanged(loaded, playing, bindings) {
            if (loaded && playing) {
              bindings.button.button.sendColor(device.colors.lo_red);
            } else if (loaded) {
              bindings.button.button.sendColor(device.colors.lo_yellow);
            } else {
              bindings.button.button.sendColor(device.colors.lo_green);
            }
          };

          return {
            bindings: {
              samples: {
                type: 'control',
                target: deck.track_samples,
                update: function update(_ref, _ref2) {
                  var value = _ref.value;
                  var bindings = _ref2.bindings;
                  return onStateChanged(value, bindings.play.getValue(), bindings);
                }
              },
              play: {
                type: 'control',
                target: deck.play,
                update: function update(_ref3, _ref4) {
                  var value = _ref3.value;
                  var bindings = _ref4.bindings;
                  return onStateChanged(bindings.samples.getValue(), value, bindings);
                }
              },
              button: {
                type: 'button',
                target: gridPosition,
                attack: function attack(message, _ref5) {
                  var bindings = _ref5.bindings;
                  modes(modifier.getState(), function () {
                    if (!bindings.samples.getValue()) {
                      deck.LoadSelectedTrack.setValue(1);
                    }
                  }, function () {
                    return deck.LoadSelectedTrack.setValue(1);
                  }, function () {
                    return deck.eject.setValue(1);
                  });
                }
              }
            }
          };
        };
      };
    };
  });

  var key = (function (gridPosition) {
    return function (deck) {
      return function (modifier) {
        return function (device) {
          return {
            bindings: {
              button: {
                type: 'button',
                target: gridPosition,
                attack: function attack(message, _ref) {
                  var bindings = _ref.bindings;
                  modes(modifier.getState(), function () {
                    bindings.keylock.setValue(Number(!bindings.keylock.getValue()));
                  }, function () {
                    deck.key.setValue(deck.key.getValue() - 1);
                  }, function () {
                    deck.key.setValue(deck.key.getValue() + 1);
                  }, function () {
                    deck.reset_key.setValue(1);
                  });
                }
              },
              keylock: {
                type: 'control',
                target: deck.keylock,
                update: function update(_ref2, _ref3) {
                  var value = _ref2.value;
                  var bindings = _ref3.bindings;

                  if (value) {
                    bindings.button.button.sendColor(device.colors.hi_red);
                  } else {
                    bindings.button.button.sendColor(device.colors.black);
                  }
                }
              }
            }
          };
        };
      };
    };
  });

  /** Built-in value references. */

  var spreadableSymbol = _Symbol$1 ? _Symbol$1.isConcatSpreadable : undefined;
  /**
   * Checks if `value` is a flattenable `arguments` object or array.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is flattenable, else `false`.
   */

  function isFlattenable(value) {
    return isArray$1(value) || isArguments$1(value) || !!(spreadableSymbol && value && value[spreadableSymbol]);
  }

  /**
   * The base implementation of `_.flatten` with support for restricting flattening.
   *
   * @private
   * @param {Array} array The array to flatten.
   * @param {number} depth The maximum recursion depth.
   * @param {boolean} [predicate=isFlattenable] The function invoked per iteration.
   * @param {boolean} [isStrict] Restrict to values that pass `predicate` checks.
   * @param {Array} [result=[]] The initial result value.
   * @returns {Array} Returns the new flattened array.
   */

  function baseFlatten(array, depth, predicate, isStrict, result) {
    var index = -1,
        length = array.length;
    predicate || (predicate = isFlattenable);
    result || (result = []);

    while (++index < length) {
      var value = array[index];

      if (depth > 0 && predicate(value)) {
        if (depth > 1) {
          // Recursively flatten arrays (susceptible to call stack limits).
          baseFlatten(value, depth - 1, predicate, isStrict, result);
        } else {
          arrayPush(result, value);
        }
      } else if (!isStrict) {
        result[result.length] = value;
      }
    }

    return result;
  }

  /**
   * Creates a base function for methods like `_.forIn` and `_.forOwn`.
   *
   * @private
   * @param {boolean} [fromRight] Specify iterating from right to left.
   * @returns {Function} Returns the new base function.
   */
  function createBaseFor(fromRight) {
    return function (object, iteratee, keysFunc) {
      var index = -1,
          iterable = Object(object),
          props = keysFunc(object),
          length = props.length;

      while (length--) {
        var key = props[fromRight ? length : ++index];

        if (iteratee(iterable[key], key, iterable) === false) {
          break;
        }
      }

      return object;
    };
  }

  /**
   * The base implementation of `baseForOwn` which iterates over `object`
   * properties returned by `keysFunc` and invokes `iteratee` for each property.
   * Iteratee functions may exit iteration early by explicitly returning `false`.
   *
   * @private
   * @param {Object} object The object to iterate over.
   * @param {Function} iteratee The function invoked per iteration.
   * @param {Function} keysFunc The function to get the keys of `object`.
   * @returns {Object} Returns `object`.
   */

  var baseFor = createBaseFor();

  /**
   * The base implementation of `_.forOwn` without support for iteratee shorthands.
   *
   * @private
   * @param {Object} object The object to iterate over.
   * @param {Function} iteratee The function invoked per iteration.
   * @returns {Object} Returns `object`.
   */

  function baseForOwn(object, iteratee) {
    return object && baseFor(object, iteratee, keys$1);
  }

  /**
   * Creates a `baseEach` or `baseEachRight` function.
   *
   * @private
   * @param {Function} eachFunc The function to iterate over a collection.
   * @param {boolean} [fromRight] Specify iterating from right to left.
   * @returns {Function} Returns the new base function.
   */

  function createBaseEach(eachFunc, fromRight) {
    return function (collection, iteratee) {
      if (collection == null) {
        return collection;
      }

      if (!isArrayLike$1(collection)) {
        return eachFunc(collection, iteratee);
      }

      var length = collection.length,
          index = fromRight ? length : -1,
          iterable = Object(collection);

      while (fromRight ? index-- : ++index < length) {
        if (iteratee(iterable[index], index, iterable) === false) {
          break;
        }
      }

      return collection;
    };
  }

  /**
   * The base implementation of `_.forEach` without support for iteratee shorthands.
   *
   * @private
   * @param {Array|Object} collection The collection to iterate over.
   * @param {Function} iteratee The function invoked per iteration.
   * @returns {Array|Object} Returns `collection`.
   */

  var baseEach = createBaseEach(baseForOwn);

  /**
   * The base implementation of `_.map` without support for iteratee shorthands.
   *
   * @private
   * @param {Array|Object} collection The collection to iterate over.
   * @param {Function} iteratee The function invoked per iteration.
   * @returns {Array} Returns the new mapped array.
   */

  function baseMap(collection, iteratee) {
    var index = -1,
        result = isArrayLike$1(collection) ? Array(collection.length) : [];
    baseEach(collection, function (value, key, collection) {
      result[++index] = iteratee(value, key, collection);
    });
    return result;
  }

  /**
   * Creates an array of values by running each element in `collection` thru
   * `iteratee`. The iteratee is invoked with three arguments:
   * (value, index|key, collection).
   *
   * Many lodash methods are guarded to work as iteratees for methods like
   * `_.every`, `_.filter`, `_.map`, `_.mapValues`, `_.reject`, and `_.some`.
   *
   * The guarded methods are:
   * `ary`, `chunk`, `curry`, `curryRight`, `drop`, `dropRight`, `every`,
   * `fill`, `invert`, `parseInt`, `random`, `range`, `rangeRight`, `repeat`,
   * `sampleSize`, `slice`, `some`, `sortBy`, `split`, `take`, `takeRight`,
   * `template`, `trim`, `trimEnd`, `trimStart`, and `words`
   *
   * @static
   * @memberOf _
   * @since 0.1.0
   * @category Collection
   * @param {Array|Object} collection The collection to iterate over.
   * @param {Function} [iteratee=_.identity] The function invoked per iteration.
   * @returns {Array} Returns the new mapped array.
   * @example
   *
   * function square(n) {
   *   return n * n;
   * }
   *
   * _.map([4, 8], square);
   * // => [16, 64]
   *
   * _.map({ 'a': 4, 'b': 8 }, square);
   * // => [16, 64] (iteration order is not guaranteed)
   *
   * var users = [
   *   { 'user': 'barney' },
   *   { 'user': 'fred' }
   * ];
   *
   * // The `_.property` iteratee shorthand.
   * _.map(users, 'user');
   * // => ['barney', 'fred']
   */

  function map(collection, iteratee) {
    var func = isArray$1(collection) ? arrayMap : baseMap;
    return func(collection, baseIteratee(iteratee));
  }

  /**
   * Creates a flattened array of values by running each element in `collection`
   * thru `iteratee` and flattening the mapped results. The iteratee is invoked
   * with three arguments: (value, index|key, collection).
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Collection
   * @param {Array|Object} collection The collection to iterate over.
   * @param {Function} [iteratee=_.identity] The function invoked per iteration.
   * @returns {Array} Returns the new flattened array.
   * @example
   *
   * function duplicate(n) {
   *   return [n, n];
   * }
   *
   * _.flatMap([1, 2], duplicate);
   * // => [1, 1, 2, 2]
   */

  function flatMap(collection, iteratee) {
    return baseFlatten(map(collection, iteratee), 1);
  }

  var beatjump = (function (jumps, vertical) {
    return function (gridPosition) {
      return function (deck) {
        return function (modifier) {
          return function (device) {
            var bindings = {};

            var onMidi = function onMidi(k, j, d) {
              return function (modifier) {
                return retainAttackMode(modifier, function (mode, _ref, _ref2) {
                  var value = _ref.value;
                  var bindings = _ref2.bindings,
                      state = _ref2.state;
                  modes(mode, function () {
                    if (!state.mode) {
                      if (value) {
                        deck.beatjump.setValue(j[state.set] * d);
                      }
                    } else {
                      if (value) {
                        var currentJump = j[state.set] * d;
                        deck.beatjump.setValue(currentJump);

                        if (state.pressing != null) {
                          bindings[state.pressing].button.sendColor(device.colors["lo_".concat(state.color[state.set])]);
                        }

                        bindings[k].button.sendColor(device.colors["hi_".concat(state.color[state.set])]);
                        state.pressing = k;
                        state.diff = state.diff + currentJump;
                      } else {
                        if (state.pressing === k) {
                          bindings[k].button.sendColor(device.colors["lo_".concat(state.color[state.set])]);
                          state.pressing = null;
                          deck.beatjump.setValue(-state.diff);
                          state.diff = 0;
                        }
                      }
                    }
                  }, function () {
                    if (value) {
                      if (state.set === 1) {
                        state.set = 0;
                        var prefix = state.mode ? 'lo' : 'hi';

                        for (var b = 0; b < spec.length; ++b) {
                          bindings[b].button.sendColor(device.colors["".concat(prefix, "_").concat(state.color[state.set])]);
                        }
                      }
                    }
                  }, function () {
                    if (value) {
                      if (state.set === 0) {
                        state.set = 1;
                        var prefix = state.mode ? 'lo' : 'hi';

                        for (var b = 0; b < spec.length; ++b) {
                          bindings[b].button.sendColor(device.colors["".concat(prefix, "_").concat(state.color[state.set])]);
                        }
                      }
                    }
                  }, function () {
                    if (value) {
                      state.mode = !state.mode;
                      var prefix = state.mode ? 'lo' : 'hi';

                      for (var b = 0; b < spec.length; ++b) {
                        bindings[b].button.sendColor(device.colors["".concat(prefix, "_").concat(state.color[state.set])]);
                      }
                    }
                  });
                });
              };
            };

            var onMount = function onMount(k) {
              return function (_, _ref3) {
                var bindings = _ref3.bindings,
                    state = _ref3.state;
                var prefix = state.mode ? 'lo' : 'hi';
                bindings[k].button.sendColor(device.colors["".concat(prefix, "_").concat(state.color[state.set])]);
              };
            };

            var spec = flatMap(jumps, function (j, i) {
              return [[j, -1], [j, 1]];
            });
            spec.forEach(function (_ref4, i) {
              var _ref5 = _slicedToArray(_ref4, 2),
                  jump = _ref5[0],
                  dir = _ref5[1];

              bindings[i] = {
                type: 'button',
                target: vertical ? [gridPosition[0] + i % 2, gridPosition[1] + ~~(i / 2)] : [gridPosition[0] + ~~(i / 2), gridPosition[1] + i % 2],
                midi: onMidi(i, jump, dir)(modifier),
                mount: onMount(i)
              };
            });
            return {
              bindings: bindings,
              state: {
                mode: false,
                pressing: 0,
                diff: 0,
                set: 0,
                color: ['green', 'red']
              }
            };
          };
        };
      };
    };
  });

  var beatloop = (function (loops, d) {
    return function (gridPosition) {
      return function (deck) {
        return function (modifier) {
          return function (device) {
            var bindings = {};

            var onAttack = function onAttack(l) {
              return function (modifier) {
                return function () {
                  modes(modifier.getState(), function () {
                    return deck.beatloops[l].toggle.setValue(1);
                  });
                };
              };
            };

            var onUpdate = function onUpdate(i) {
              return function (_ref, _ref2) {
                var value = _ref.value;
                var bindings = _ref2.bindings;

                if (value) {
                  bindings[i].button.sendColor(device.colors.hi_red);
                } else {
                  bindings[i].button.sendColor(device.colors.lo_red);
                }
              };
            };

            loops.forEach(function (loop, i) {
              var dx = i % d;
              var dy = ~~(i / d);
              bindings[i] = {
                type: 'button',
                target: [gridPosition[0] + dx, gridPosition[1] + dy],
                attack: onAttack(loop)(modifier)
              };
              bindings["".concat(loop, ".enabled")] = {
                type: 'control',
                target: deck.beatloops[loop].enabled,
                update: onUpdate(i)
              };
            });
            return {
              bindings: bindings
            };
          };
        };
      };
    };
  });

  var loopjump = function loopjump(jumps) {
    return function (gridPosition) {
      return function (deck) {
        return function (modifier) {
          return function (device) {
            var bindings = {};

            var onMidi = function onMidi(k, j, d) {
              return function (modifier) {
                return retainAttackMode(modifier, function (mode, _ref, _ref2) {
                  var value = _ref.value;
                  var bindings = _ref2.bindings,
                      state = _ref2.state;
                  modes(mode, function () {
                    if (!state.mode) {
                      if (value) {
                        deck.loop_move.setValue(j[state.set] * d);
                      }
                    } else {
                      if (value) {
                        var currentJump = j[state.set] * d;
                        deck.loop_move.setValue(currentJump);

                        if (state.pressing != null) {
                          bindings[state.pressing].button.sendColor(device.colors["lo_".concat(state.color[state.set])]);
                        }

                        bindings[k].button.sendColor(device.colors["hi_".concat(state.color[state.set])]);
                        state.pressing = k;
                        state.diff = state.diff + currentJump;
                      } else {
                        if (state.pressing === k) {
                          bindings[k].button.sendColor(device.colors["lo_".concat(state.color[state.set])]);
                          state.pressing = null;
                          deck.loop_move.setValue(-state.diff);
                          state.diff = 0;
                        }
                      }
                    }
                  }, function () {
                    if (value) {
                      if (state.set === 1) {
                        state.set = 0;
                        var prefix = state.mode ? 'lo' : 'hi';

                        for (var b = 0; b < spec.length; ++b) {
                          bindings[b].button.sendColor(device.colors["".concat(prefix, "_").concat(state.color[state.set])]);
                        }
                      }
                    }
                  }, function () {
                    if (value) {
                      if (state.set === 0) {
                        state.set = 1;
                        var prefix = state.mode ? 'lo' : 'hi';

                        for (var b = 0; b < spec.length; ++b) {
                          bindings[b].button.sendColor(device.colors["".concat(prefix, "_").concat(state.color[state.set])]);
                        }
                      }
                    }
                  }, function () {
                    if (value) {
                      state.mode = !state.mode;
                      var prefix = state.mode ? 'lo' : 'hi';

                      for (var b = 0; b < spec.length; ++b) {
                        bindings[b].button.sendColor(device.colors["".concat(prefix, "_").concat(state.color[state.set])]);
                      }
                    }
                  });
                });
              };
            };

            var onMount = function onMount(k) {
              return function (_, _ref3) {
                var bindings = _ref3.bindings,
                    state = _ref3.state;
                var prefix = state.mode ? 'lo' : 'hi';
                bindings[k].button.sendColor(device.colors["".concat(prefix, "_").concat(state.color[state.set])]);
              };
            };

            var spec = flatMap(jumps, function (j, i) {
              return [[j, 1], [j, -1]];
            }); // FIXME: flatMap is incorrectly typed see https://github.com/flow-typed/flow-typed/issues/2463

            spec.forEach(function (_ref4, i) {
              var _ref5 = _slicedToArray(_ref4, 2),
                  jump = _ref5[0],
                  dir = _ref5[1];

              bindings[i] = {
                type: 'button',
                target: [gridPosition[0] + i % 2, gridPosition[1] + ~~(i / 2)],
                midi: onMidi(i, jump, dir)(modifier),
                mount: onMount(i)
              };
            });
            return {
              bindings: bindings,
              state: {
                mode: false,
                pressing: 0,
                diff: 0,
                set: 0,
                color: ['green', 'red']
              }
            };
          };
        };
      };
    };
  };
  var loopjumpSmall = function loopjumpSmall(amount) {
    return function (button) {
      return function (deck) {
        return function (modifier) {
          return function (device) {
            var onAttack = function onAttack(dir) {
              return function () {
                modes(modifier.getState(), function () {
                  return deck.loop_move.setValue(dir * amount);
                });
              };
            };

            return {
              bindings: {
                back: {
                  type: 'button',
                  target: button,
                  attack: onAttack(-1),
                  mount: function mount(dk, _ref6) {
                    var bindings = _ref6.bindings;
                    bindings.back.button.sendColor(device.colors.hi_yellow);
                  }
                },
                forth: {
                  type: 'button',
                  target: [button[0] + 1, button[1]],
                  attack: onAttack(1),
                  mount: function mount(dk, _ref7) {
                    var bindings = _ref7.bindings;
                    bindings.forth.button.sendColor(device.colors.hi_yellow);
                  }
                }
              }
            };
          };
        };
      };
    };
  };

  var loopMultiply = (function (gridPosition) {
    return function (deck) {
      return function (_) {
        return function (device) {
          var onMount = function onMount(k) {
            return function (dk, _ref) {
              var bindings = _ref.bindings;
              bindings[k].button.sendColor(device.colors.lo_yellow);
            };
          };

          var onAttack = function onAttack(k) {
            return function () {
              // TODO: remove unsafe cast once flow supports https://github.com/facebook/flow/issues/3637
              deck["loop_".concat(k)].setValue(1);
            };
          };

          return {
            bindings: {
              halve: {
                type: 'button',
                target: gridPosition,
                mount: onMount('halve'),
                attack: onAttack('halve')
              },
              "double": {
                type: 'button',
                target: [gridPosition[0] + 1, gridPosition[1]],
                mount: onMount('double'),
                attack: onAttack('double')
              }
            }
          };
        };
      };
    };
  });

  var reloop = (function (gridPosition) {
    return function (deck) {
      return function (modifier) {
        return function (device) {
          return {
            bindings: {
              button: {
                type: 'button',
                target: gridPosition,
                attack: function attack() {
                  modes(modifier.getState(), function () {
                    return deck.reloop_exit.setValue(1);
                  }, function () {
                    return deck.reloop_andstop.setValue(1);
                  });
                }
              },
              control: {
                type: 'control',
                target: deck.loop_enabled,
                update: function update(_ref, _ref2) {
                  var value = _ref.value;
                  var bindings = _ref2.bindings;

                  if (value) {
                    bindings.button.button.sendColor(device.colors.hi_green);
                  } else {
                    bindings.button.button.sendColor(device.colors.lo_green);
                  }
                }
              }
            }
          };
        };
      };
    };
  });

  var SMALL_SAMPLES = 125;
  var loopIo = (function (gridPosition) {
    return function (deck) {
      return function (modifier) {
        return function (device) {
          var loopName = {
            "in": 'loop_in',
            out: 'loop_out'
          };
          var loopPosName = {
            "in": 'loop_start_position',
            out: 'loop_end_position'
          };

          var onMidi = function onMidi(dir) {
            return function (_ref, _ref2) {
              var value = _ref.value;
              var bindings = _ref2.bindings;
              modes(modifier.getState(), function () {
                if (value) {
                  var ctrl = loopName[dir];
                  deck[ctrl].setValue(1);
                  deck[ctrl].setValue(0);
                }
              }, function () {
                if (value) {
                  var ctrl = loopPosName[dir];
                  deck[ctrl].setValue(deck[ctrl].getValue() - SMALL_SAMPLES);
                }
              }, function () {
                if (value) {
                  var ctrl = loopPosName[dir];
                  deck[ctrl].setValue(deck[ctrl].getValue() + SMALL_SAMPLES);
                }
              });
            };
          };

          return {
            bindings: {
              "in": {
                type: 'button',
                target: gridPosition,
                midi: onMidi('in')
              },
              out: {
                type: 'button',
                target: [gridPosition[0] + 1, gridPosition[1]],
                midi: onMidi('out')
              }
            }
          };
        };
      };
    };
  });

  var slip = (function (gridPosition) {
    return function (deck) {
      return function (modifier) {
        return function (device) {
          var onMidi = function onMidi(modifier) {
            return retainAttackMode(modifier, function (mode, _ref, _ref2) {
              var value = _ref.value;
              var bindings = _ref2.bindings,
                  state = _ref2.state;
              modes(mode, function () {
                if (value) {
                  bindings.control.setValue(Number(!bindings.control.getValue()));
                } else {
                  if (state.mode) {
                    bindings.control.setValue(Number(!bindings.control.getValue()));
                  }
                }
              }, function () {
                if (value) {
                  state.mode = !state.mode;
                  var color = state.mode ? 'orange' : 'red';
                  bindings.button.button.sendColor(device.colors["lo_".concat(color)]);
                }
              });
            });
          };

          return {
            bindings: {
              control: {
                type: 'control',
                target: deck.slip_enabled,
                update: function update(_ref3, _ref4) {
                  var value = _ref3.value;
                  var bindings = _ref4.bindings,
                      state = _ref4.state;
                  var color = state.mode ? 'orange' : 'red';

                  if (value) {
                    bindings.button.button.sendColor(device.colors["hi_".concat(color)]);
                  } else {
                    bindings.button.button.sendColor(device.colors["lo_".concat(color)]);
                  }
                }
              },
              button: {
                type: 'button',
                target: gridPosition,
                midi: onMidi(modifier),
                mount: function mount(dk, _ref5) {
                  var bindings = _ref5.bindings,
                      state = _ref5.state;
                  var color = state.mode ? 'orange' : 'red';
                  bindings.button.button.sendColor(device.colors["lo_".concat(color)]);
                }
              }
            },
            state: {
              mode: 1
            }
          };
        };
      };
    };
  });

  var Grande = {
    play: play([0, 0]),
    sync: sync([1, 0]),
    nudge: nudge([2, 0]),
    cue: cue([0, 1]),
    tap: tap([1, 1]),
    grid: grid([2, 1]),
    pfl: pfl([0, 2]),
    quantize: quantize([1, 2]),
    keyshift: keyshift([[1, 1], [2, 2], [3, 3], [5, 4], [7, 5], [8, 6], [10, 7], [12, 8]], 2)([2, 2]),
    load: load([0, 3]),
    key: key([1, 3]),
    hotcue: hotcue(8, 2)([0, 4]),
    beatjump: beatjump([[0.25, 1], [0.33, 2], [0.5, 4], [0.75, 8], [1, 16], [2, 32]])([2, 6]),
    beatloop: beatloop([0.5, 1, 2, 4, 8, 16, 32, 64], 2)([4, 2]),
    loopjump: loopjump([[0.5, 8], [1, 16], [2, 32], [4, 64]])([6, 2]),
    loopjumpSmall: loopjumpSmall(0.03125)([6, 1]),
    loopMultiply: loopMultiply([4, 1]),
    reloop: reloop([4, 0]),
    loopIo: loopIo([5, 0]),
    slip: slip([7, 0])
  };

  var Juggler = {
    play: play([0, 0]),
    load: load([1, 0]),
    beatjump: beatjump([[0.5, 4], [1, 16], [2, 32], [4, 64]], true)([2, 0]),
    loopjump: loopjump([[1, 16], [4, 64]])([0, 1]),
    reloop: reloop([0, 3]),
    loopMultiply: loopMultiply([0, 4]),
    hotcue: hotcue(8, 2)([2, 4]),
    beatloop: beatloop([0.5, 1, 2, 4, 8, 16], 2)([0, 5])
  };

  var Sampler = {
    hotcue: hotcue(16, 4)([0, 0])
  };

  var Short = {
    play: play([0, 0]),
    sync: sync([1, 0]),
    nudge: nudge([2, 0]),
    cue: cue([0, 1]),
    tap: tap([1, 1]),
    grid: grid([2, 1]),
    pfl: pfl([0, 2]),
    quantize: quantize([1, 2]),
    loopIo: loopIo([2, 2]),
    load: load([0, 3]),
    key: key([1, 3]),
    reloop: reloop([2, 3]),
    slip: slip([3, 3])
  };

  var Tall = {
    play: play([0, 0]),
    sync: sync([1, 0]),
    nudge: nudge([2, 0]),
    cue: cue([0, 1]),
    tap: tap([1, 1]),
    grid: grid([2, 1]),
    pfl: pfl([0, 2]),
    quantize: quantize([1, 2]),
    loopIo: loopIo([2, 2]),
    load: load([0, 3]),
    key: key([1, 3]),
    reloop: reloop([2, 3]),
    slip: slip([3, 3]),
    hotcue: hotcue(4, 2)([0, 4]),
    loopMultiply: loopMultiply([2, 4]),
    beatloop: beatloop([0.5, 1, 2, 4, 8, 16], 2)([2, 5]),
    beatjump: beatjump([[1, 16], [2, 32]])([0, 6])
  };

  function _createSuper$7(Derived) { return function () { var Super = _getPrototypeOf(Derived), result; if (_isNativeReflectConstruct$7()) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

  function _isNativeReflectConstruct$7() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Date.prototype.toString.call(Reflect.construct(Date, [], function () {})); return true; } catch (e) { return false; } }
  var makePresetFromPartialTemplate = function makePresetFromPartialTemplate(id, partialTemplate, offset) {
    return function (deck) {
      return function (controlComponentBuilder) {
        return function (midibus) {
          return function (modifier) {
            var template = {};
            Object.keys(partialTemplate).forEach(function (k) {
              assign$1(template, _defineProperty$1({}, k, partialTemplate[k](deck)(modifier)(midibus.device)));
            });
            return new Preset(midibus, controlComponentBuilder, modifier, id, template, offset);
          };
        };
      };
    };
  };
  var Preset = /*#__PURE__*/function (_MidiComponent) {
    _inherits(Preset, _MidiComponent);

    var _super = _createSuper$7(Preset);

    function Preset(midibus, controlComponentBuilder, modifier, id, template, offset) {
      var _this;

      _classCallCheck$1(this, Preset);

      _this = _super.call(this, midibus);

      _defineProperty$1(_assertThisInitialized(_this), "preset", void 0);

      var controlBindings = {};
      var controlListeners = {};
      var buttonBindings = {};
      var buttonListeners = {};
      Object.keys(template).forEach(function (tk) {
        if (template[tk] && template[tk].bindings) {
          var bindings = template[tk].bindings;
          var instance = {
            state: template[tk].state,
            bindings: {}
          };
          Object.keys(bindings).forEach(function (bk) {
            if (bindings[bk]) {
              var binding = bindings[bk];

              if (binding.type === 'control') {
                var name = "".concat(binding.target.def.group).concat(binding.target.def.name);

                if (!controlBindings[name]) {
                  controlBindings[name] = controlComponentBuilder("".concat(id, ".").concat(tk, ".").concat(bk))(binding.target);
                }

                instance.bindings[bk] = controlBindings[name];
                controlListeners[name] = controlListeners[name] || {};
                ['update', 'mount', 'unmount'].forEach(function (action) {
                  if (typeof binding[action] === 'function') {
                    appendListener(action, controlListeners[name], function (data) {
                      return binding[action](data, instance, modifier);
                    });
                  }
                });
              } else if (binding.type === 'button') {
                var position = tr(binding.target, offset);

                var _name = nameOf(position[0], position[1]);

                if (!buttonBindings[_name]) {
                  buttonBindings[_name] = new MidiButtonComponent(_this.midibus, _this.device.buttons[_name]);
                }

                instance.bindings[bk] = buttonBindings[_name];
                buttonListeners[_name] = buttonListeners[_name] || {};
                ['attack', 'release', 'midi', 'mount', 'unmount'].forEach(function (action) {
                  if (typeof binding[action] === 'function') {
                    appendListener(action, buttonListeners[_name], function (data) {
                      return binding[action](data, instance);
                    });
                  }
                });

                if (typeof binding.unmount !== 'function') {
                  appendListener('unmount', buttonListeners[_name], function (data) {
                    instance.bindings[bk].button.sendColor(this.device.colors.black);
                  });
                }
              }
            }
          });
        }
      });
      _this.preset = {
        controlBindings: controlBindings,
        controlListeners: controlListeners,
        buttonBindings: buttonBindings,
        buttonListeners: buttonListeners
      };
      return _this;
    }

    _createClass$1(Preset, [{
      key: "onMount",
      value: function onMount() {
        var _this$preset = this.preset,
            controlBindings = _this$preset.controlBindings,
            buttonBindings = _this$preset.buttonBindings,
            controlListeners = _this$preset.controlListeners,
            buttonListeners = _this$preset.buttonListeners;
        addListeners(controlBindings, controlListeners);
        addListeners(buttonBindings, buttonListeners);
        Object.keys(controlBindings).forEach(function (k) {
          return controlBindings[k].mount();
        });
        Object.keys(buttonBindings).forEach(function (k) {
          return buttonBindings[k].mount();
        });
      }
    }, {
      key: "onUnmount",
      value: function onUnmount() {
        var _this$preset2 = this.preset,
            controlBindings = _this$preset2.controlBindings,
            buttonBindings = _this$preset2.buttonBindings,
            controlListeners = _this$preset2.controlListeners,
            buttonListeners = _this$preset2.buttonListeners;
        Object.keys(controlBindings).forEach(function (k) {
          return controlBindings[k].unmount();
        });
        Object.keys(buttonBindings).forEach(function (k) {
          return buttonBindings[k].unmount();
        });
        removeListeners(controlBindings, controlListeners);
        removeListeners(buttonBindings, buttonListeners);
      }
    }]);

    return Preset;
  }(MidiComponent);

  var tr = function tr(a, b) {
    return [a[0] + b[0], a[1] + b[1]];
  };

  var nameOf = function nameOf(x, y) {
    return "".concat(7 - y, ",").concat(x);
  };

  var appendListener = function appendListener(type, bindings, binding) {
    if (bindings[type] && Array.isArray(bindings[type])) {
      bindings[type].push(binding);
    } else if (bindings[type]) {
      var first = bindings[type];
      bindings[type] = [first, binding];
    } else {
      bindings[type] = binding;
    }
  };

  var addListeners = function addListeners(tgt, bindings) {
    Object.keys(bindings).forEach(function (binding) {
      if (tgt[binding]) {
        Object.keys(bindings[binding]).forEach(function (k) {
          if (Array.isArray(bindings[binding][k])) {
            bindings[binding][k].forEach(function (f) {
              tgt[binding].on(k, f);
            });
          } else {
            tgt[binding].on(k, bindings[binding][k]);
          }
        });
      }
    });
  };

  var removeListeners = function removeListeners(tgt, bindings) {
    Object.keys(bindings).forEach(function (binding) {
      if (tgt[binding]) {
        Object.keys(bindings[binding]).forEach(function (k) {
          if (Array.isArray(bindings[binding][k])) {
            bindings[binding][k].forEach(function (f) {
              tgt[binding].removeListener(k, f);
            });
          } else {
            tgt[binding].removeListener(k, bindings[binding][k]);
          }
        });
      }
    });
  };

  function _createSuper$8(Derived) { return function () { var Super = _getPrototypeOf(Derived), result; if (_isNativeReflectConstruct$8()) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

  function _isNativeReflectConstruct$8() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Date.prototype.toString.call(Reflect.construct(Date, [], function () {})); return true; } catch (e) { return false; } }
  var initialChannels = [0, 1];

  var onMidi$1 = function onMidi(selectorBar, channel, modifier) {
    return retainAttackMode(modifier, function (mode, _ref) {
      var value = _ref.value;
      var selected = selectorBar.getChord();
      modes(mode, function () {
        if (!value && selected.length) {
          var diff = reorganize(selectorBar.getLayout(), selected);
          selectorBar.updateLayout(diff);
          selectorBar.removeChord();
        } else if (value) {
          selectorBar.addToChord(channel);
        }
      }, function () {
        if (value) {
          if (selected.length) selectorBar.removeChord();
          var diff = cycle(channel, selectorBar.getLayout(), 1);
          selectorBar.updateLayout(diff);
        }
      }, function () {
        if (value) {
          if (selected.length) selectorBar.removeChord();
          var diff = cycle(channel, selectorBar.getLayout(), -1);
          selectorBar.updateLayout(diff);
        }
      });
    });
  };

  var SelectorBar = /*#__PURE__*/function (_MidiComponent) {
    _inherits(SelectorBar, _MidiComponent);

    var _super = _createSuper$8(SelectorBar);

    function SelectorBar(midibus, controlComponentBuilder, modifier, id) {
      var _this;

      _classCallCheck$1(this, SelectorBar);

      _this = _super.call(this, midibus);

      _defineProperty$1(_assertThisInitialized(_this), "id", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "bindings", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "controlComponentBuilder", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "modifier", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "chord", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "layout", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "mountedPresets", void 0);

      _this.id = id;
      _this.bindings = SelectorBar.buttons.map(function (v, i) {
        var binding = new MidiButtonComponent(_this.midibus, _this.device.buttons[v]);
        return [binding, onMidi$1(_assertThisInitialized(_this), i, modifier)];
      });
      _this.controlComponentBuilder = controlComponentBuilder;
      _this.modifier = modifier;
      _this.chord = [];
      _this.layout = {};
      _this.mountedPresets = {};
      return _this;
    }

    _createClass$1(SelectorBar, [{
      key: "getLayout",
      value: function getLayout() {
        var res = [];

        for (var k in this.layout) {
          res.push(this.layout[k]);
        }

        return res;
      }
    }, {
      key: "updateLayout",
      value: function updateLayout(diff) {
        var _this2 = this;

        var removedChannels = diff[0].map(function (block) {
          return block.channel;
        });
        removedChannels.forEach(function (ch) {
          delete _this2.layout[String(ch)];

          _this2.bindings[ch][0].button.sendColor(_this2.device.colors.black);

          _this2.mountedPresets[ch].unmount();
        });
        var addedBlocks = diff[1];
        addedBlocks.forEach(function (block) {
          _this2.layout[String(block.channel)] = block;

          if (block.index) {
            _this2.bindings[block.channel][0].button.sendColor(_this2.device.colors.hi_orange);
          } else {
            _this2.bindings[block.channel][0].button.sendColor(_this2.device.colors.hi_green);
          }

          _this2.mountedPresets[block.channel] = makePresetFromPartialTemplate("".concat(_this2.id, ".deck.").concat(block.channel), cycled[block.size][block.index], block.offset)(channelControls[block.channel])(_this2.controlComponentBuilder)(_this2.midibus)(_this2.modifier);

          _this2.mountedPresets[block.channel].mount();
        });
      }
    }, {
      key: "removeChord",
      value: function removeChord() {
        var _this3 = this;

        var layout = this.getLayout();
        this.chord.forEach(function (ch) {
          var found = findIndex(layout, function (b) {
            return b.channel === ch;
          });

          if (found === -1) {
            _this3.bindings[ch][0].button.sendColor(_this3.device.colors.black);
          } else {
            var block = layout[found];

            if (block.index) {
              _this3.bindings[ch][0].button.sendColor(_this3.device.colors.hi_orange);
            } else {
              _this3.bindings[ch][0].button.sendColor(_this3.device.colors.hi_green);
            }
          }

          _this3.chord = [];
        });
      }
    }, {
      key: "addToChord",
      value: function addToChord(channel) {
        if (this.chord.length === 4) {
          var rem = this.chord.shift();
          var found = findIndex(this.layout, function (b) {
            return b.channel === rem;
          }); // FIXME: badly typed

          if (found === -1) {
            this.bindings[rem][0].button.sendColor(this.device.colors.black);
          } else {
            var layout = this.layout[String(found)];

            if (layout.index) {
              this.bindings[rem][0].button.sendColor(this.device.colors.hi_orange);
            } else {
              this.bindings[rem][0].button.sendColor(this.device.colors.hi_green);
            }
          }
        }

        this.chord.push(channel);
        this.bindings[channel][0].button.sendColor(this.device.colors.hi_red);
      }
    }, {
      key: "getChord",
      value: function getChord() {
        return this.chord;
      }
    }, {
      key: "onMount",
      value: function onMount() {
        this.bindings.forEach(function (_ref2) {
          var _ref3 = _slicedToArray(_ref2, 2),
              binding = _ref3[0],
              midi = _ref3[1];

          binding.mount();
          binding.on('midi', midi);
        });
      }
    }, {
      key: "onUnmount",
      value: function onUnmount() {
        this.bindings.forEach(function (_ref4) {
          var _ref5 = _slicedToArray(_ref4, 2),
              binding = _ref5[0],
              midi = _ref5[1];

          binding.removeListener('midi', midi);
          binding.unmount();
        });
      }
    }]);

    return SelectorBar;
  }(MidiComponent);

  _defineProperty$1(SelectorBar, "buttons", ['up', 'down', 'left', 'right', 'session', 'user1', 'user2', 'mixer']);

  _defineProperty$1(SelectorBar, "channels", [0, 1, 2, 3, 4, 5, 6, 7]);

  var Layout = /*#__PURE__*/function (_MidiComponent2) {
    _inherits(Layout, _MidiComponent2);

    var _super2 = _createSuper$8(Layout);

    function Layout(midibus, controlComponentBuilder, modifier, id) {
      var _this4;

      _classCallCheck$1(this, Layout);

      _this4 = _super2.call(this, midibus);

      _defineProperty$1(_assertThisInitialized(_this4), "selectorBar", void 0);

      _this4.selectorBar = new SelectorBar(midibus, controlComponentBuilder, modifier, "".concat(id, ".selectorBar"));
      return _this4;
    }

    _createClass$1(Layout, [{
      key: "onMount",
      value: function onMount() {
        this.selectorBar.mount();
        var diff = reorganize([], initialChannels);
        this.selectorBar.updateLayout(diff);
      }
    }, {
      key: "onUnmount",
      value: function onUnmount() {
        var diff = reorganize(this.selectorBar.getLayout(), []);
        this.selectorBar.updateLayout(diff);
        this.selectorBar.unmount();
      }
    }]);

    return Layout;
  }(MidiComponent);
  var offsets = [[0, 0], [4, 0], [0, 4], [4, 4]];
  var presets = {
    grande: [Grande],
    tall: [Tall, Juggler],
    "short": [Short, Sampler]
  };
  var cycled = {
    grande: [].concat(_toConsumableArray(presets.grande), _toConsumableArray(presets.tall), _toConsumableArray(presets["short"])),
    tall: [].concat(_toConsumableArray(presets.tall), _toConsumableArray(presets["short"])),
    "short": presets["short"]
  };

  var blockEquals = function blockEquals(a, b) {
    return a.offset === b.offset && a.size === b.size && a.channel === b.channel && a.index === b.index;
  };

  var reorganize = function reorganize(current, selectedChannels) {
    var next = function (chs) {
      switch (chs.length) {
        case 0:
          return [];

        case 1:
          return [{
            offset: offsets[0],
            size: 'grande',
            channel: chs[0],
            index: 0
          }];

        case 2:
          return [{
            offset: offsets[0],
            size: 'tall',
            channel: chs[0],
            index: 0
          }, {
            offset: offsets[1],
            size: 'tall',
            channel: chs[1],
            index: 0
          }];

        case 3:
          return [{
            offset: offsets[0],
            size: 'tall',
            channel: chs[0],
            index: 0
          }, {
            offset: offsets[1],
            size: 'short',
            channel: chs[1],
            index: 0
          }, {
            offset: offsets[3],
            size: 'short',
            channel: chs[2],
            index: 0
          }];

        default:
          return [{
            offset: offsets[2],
            size: 'short',
            channel: chs[0],
            index: 0
          }, {
            offset: offsets[3],
            size: 'short',
            channel: chs[1],
            index: 0
          }, {
            offset: offsets[2],
            size: 'short',
            channel: chs[2],
            index: 0
          }, {
            offset: offsets[3],
            size: 'short',
            channel: chs[3],
            index: 0
          }];
      }
    }(selectedChannels);

    return current.reduce(function (diff, block) {
      var _diff = _slicedToArray(diff, 2),
          neg = _diff[0],
          pos = _diff[1];

      var matched = findIndex(pos, function (b) {
        return blockEquals(block, b);
      });
      return matched === -1 ? [neg.concat([block]), pos] : [neg, pos.slice(0, matched).concat(pos.slice(matched + 1, pos.length))];
    }, [[], next]);
  };

  var posMod = function posMod(x, n) {
    return (x % n + n) % n;
  };

  var cycle = function cycle(channel, current, dir) {
    var matched = findIndex(current, function (block) {
      return block.channel === channel;
    });

    if (matched === -1) {
      return [[], []];
    }

    var nextIndex = posMod(current[matched].index + dir, cycled[current[matched].size].length);

    if (nextIndex === current[matched].index) {
      return [[], []];
    }

    return [[current[matched]], [assign$1({}, current[matched], {
      index: nextIndex
    })]];
  };

  function _createSuper$9(Derived) { return function () { var Super = _getPrototypeOf(Derived), result; if (_isNativeReflectConstruct$9()) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

  function _isNativeReflectConstruct$9() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Date.prototype.toString.call(Reflect.construct(Date, [], function () {})); return true; } catch (e) { return false; } }

  var Screen = /*#__PURE__*/function (_MidiComponent) {
    _inherits(Screen, _MidiComponent);

    var _super = _createSuper$9(Screen);

    function Screen(midibus, timerBuilder, controlComponentBuilder, id) {
      var _this;

      _classCallCheck$1(this, Screen);

      _this = _super.call(this, midibus);

      _defineProperty$1(_assertThisInitialized(_this), "modifier", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "playListSidebar", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "layout", void 0);

      _this.modifier = new ModifierSidebar(midibus);
      _this.playListSidebar = new PlaylistSidebar(midibus, timerBuilder);
      _this.layout = new Layout(midibus, controlComponentBuilder, _this.modifier, "".concat(id, ".layout"));
      return _this;
    }

    _createClass$1(Screen, [{
      key: "onMount",
      value: function onMount() {
        this.modifier.mount();
        this.playListSidebar.mount();
        this.layout.mount();
      }
    }, {
      key: "onUnmount",
      value: function onUnmount() {
        this.layout.unmount();
        this.playListSidebar.unmount();
        this.modifier.unmount();
      }
    }]);

    return Screen;
  }(MidiComponent);

  function _createSuper$a(Derived) { return function () { var Super = _getPrototypeOf(Derived), result; if (_isNativeReflectConstruct$a()) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

  function _isNativeReflectConstruct$a() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Date.prototype.toString.call(Reflect.construct(Date, [], function () {})); return true; } catch (e) { return false; } }
  var makeControlComponent = function makeControlComponent(controlBus) {
    return function (id) {
      return function (control) {
        return new ControlComponent(controlBus, id, control);
      };
    };
  };

  var ControlComponent = /*#__PURE__*/function (_Component) {
    _inherits(ControlComponent, _Component);

    var _super = _createSuper$a(ControlComponent);

    function ControlComponent(controlBus, id, control) {
      var _this;

      _classCallCheck$1(this, ControlComponent);

      _this = _super.call(this);

      _defineProperty$1(_assertThisInitialized(_this), "value", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "id", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "controlBus", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "control", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "_handle", void 0);

      _this.value = null;
      _this.id = id;
      _this.controlBus = controlBus;
      _this.control = control;
      _this._handle = null;
      return _this;
    }

    _createClass$1(ControlComponent, [{
      key: "onMount",
      value: function onMount() {
        var _this2 = this;

        if (!this._handle) {
          this._handle = this.controlBus.connect(this.id, this.control.def, function (data) {
            _this2.value = data.value;

            _this2.emit('update', data);
          });
          this.value = this.control.getValue();
          this.emit('update', this);
        }
      }
    }, {
      key: "onUnmount",
      value: function onUnmount() {
        if (this._handle) {
          this.controlBus.disconnect(this._handle);
          this._handle = null;
        }
      }
    }, {
      key: "setValue",
      value: function setValue(value) {
        this.control.setValue(value);
        this.value = this.control.getValue();
      }
    }, {
      key: "toggleValue",
      value: function toggleValue() {
        this.setValue(Number(!this.getValue()));
      }
    }, {
      key: "getValue",
      value: function getValue() {
        if (!this._handle) {
          this.value = this.control.getValue();
        }

        return this.value;
      }
    }]);

    return ControlComponent;
  }(Component);

  function _createSuper$b(Derived) { return function () { var Super = _getPrototypeOf(Derived), result; if (_isNativeReflectConstruct$b()) { var NewTarget = _getPrototypeOf(this).constructor; result = Reflect.construct(Super, arguments, NewTarget); } else { result = Super.apply(this, arguments); } return _possibleConstructorReturn(this, result); }; }

  function _isNativeReflectConstruct$b() { if (typeof Reflect === "undefined" || !Reflect.construct) return false; if (Reflect.construct.sham) return false; if (typeof Proxy === "function") return true; try { Date.prototype.toString.call(Reflect.construct(Date, [], function () {})); return true; } catch (e) { return false; } }
  var LaunchpadMidiButton = /*#__PURE__*/function () {
    function LaunchpadMidiButton(def) {
      _classCallCheck$1(this, LaunchpadMidiButton);

      _defineProperty$1(this, "def", void 0);

      this.def = def;
    }

    _createClass$1(LaunchpadMidiButton, [{
      key: "sendColor",
      value: function sendColor(value) {
        midi_1.sendShortMsg(this.def.status, this.def.midino, value);
      }
    }]);

    return LaunchpadMidiButton;
  }();

  var Global = /*#__PURE__*/function (_Component) {
    _inherits(Global, _Component);

    var _super = _createSuper$b(Global);

    function Global(name, device) {
      var _this;

      _classCallCheck$1(this, Global);

      _this = _super.call(this);

      _defineProperty$1(_assertThisInitialized(_this), "screen", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "device", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "name", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "init", void 0);

      _defineProperty$1(_assertThisInitialized(_this), "shutdown", void 0);

      _this.name = name;
      _this.device = device;
      var timerBuilder = makeTimer(name, _assertThisInitialized(_this));
      var controlComponentBuilder = makeControlComponent(ControlBus.create(name, _assertThisInitialized(_this)));
      var midibus = MidiBus.create(_assertThisInitialized(_this), device);
      _this.screen = new Screen(midibus, timerBuilder, controlComponentBuilder, 'main');

      _this.init = function () {
        _this.onMount();
      };

      _this.shutdown = function () {
        _this.onUnmount();
      };

      return _this;
    }

    _createClass$1(Global, [{
      key: "onMount",
      value: function onMount() {
        this.device.init();
        this.screen.mount();
      }
    }, {
      key: "onUnmount",
      value: function onUnmount() {
        this.screen.unmount();
        this.device.shutdown();
      }
    }]);

    return Global;
  }(Component);

  function create(name, device) {
    return new Global(name, device);
  }

  function _typeof2(obj) { if (typeof Symbol === "function" && typeof Symbol.iterator === "symbol") { _typeof2 = function _typeof2(obj) { return typeof obj; }; } else { _typeof2 = function _typeof2(obj) { return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : typeof obj; }; } return _typeof2(obj); }

  function _typeof$1(obj) {
    if (typeof Symbol === "function" && _typeof2(Symbol.iterator) === "symbol") {
      _typeof$1 = function _typeof(obj) {
        return _typeof2(obj);
      };
    } else {
      _typeof$1 = function _typeof(obj) {
        return obj && typeof Symbol === "function" && obj.constructor === Symbol && obj !== Symbol.prototype ? "symbol" : _typeof2(obj);
      };
    }

    return _typeof$1(obj);
  }

  /** Detect free variable `global` from Node.js. */
  var freeGlobal$2 = (typeof global === "undefined" ? "undefined" : _typeof$1(global)) == 'object' && global && global.Object === Object && global;

  /** Detect free variable `self`. */

  var freeSelf$2 = (typeof self === "undefined" ? "undefined" : _typeof$1(self)) == 'object' && self && self.Object === Object && self;
  /** Used as a reference to the global object. */

  var root$2 = freeGlobal$2 || freeSelf$2 || Function('return this')();

  /** Built-in value references. */

  var _Symbol$2 = root$2.Symbol;

  /** Used for built-in method references. */

  var objectProto$n = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$i = objectProto$n.hasOwnProperty;
  /**
   * Used to resolve the
   * [`toStringTag`](http://ecma-international.org/ecma-262/7.0/#sec-object.prototype.tostring)
   * of values.
   */

  var nativeObjectToString$4 = objectProto$n.toString;
  /** Built-in value references. */

  var symToStringTag$4 = _Symbol$2 ? _Symbol$2.toStringTag : undefined;
  /**
   * A specialized version of `baseGetTag` which ignores `Symbol.toStringTag` values.
   *
   * @private
   * @param {*} value The value to query.
   * @returns {string} Returns the raw `toStringTag`.
   */

  function getRawTag$2(value) {
    var isOwn = hasOwnProperty$i.call(value, symToStringTag$4),
        tag = value[symToStringTag$4];

    try {
      value[symToStringTag$4] = undefined;
      var unmasked = true;
    } catch (e) {}

    var result = nativeObjectToString$4.call(value);

    if (unmasked) {
      if (isOwn) {
        value[symToStringTag$4] = tag;
      } else {
        delete value[symToStringTag$4];
      }
    }

    return result;
  }

  /** Used for built-in method references. */
  var objectProto$o = Object.prototype;
  /**
   * Used to resolve the
   * [`toStringTag`](http://ecma-international.org/ecma-262/7.0/#sec-object.prototype.tostring)
   * of values.
   */

  var nativeObjectToString$5 = objectProto$o.toString;
  /**
   * Converts `value` to a string using `Object.prototype.toString`.
   *
   * @private
   * @param {*} value The value to convert.
   * @returns {string} Returns the converted string.
   */

  function objectToString$2(value) {
    return nativeObjectToString$5.call(value);
  }

  /** `Object#toString` result references. */

  var nullTag$2 = '[object Null]',
      undefinedTag$2 = '[object Undefined]';
  /** Built-in value references. */

  var symToStringTag$5 = _Symbol$2 ? _Symbol$2.toStringTag : undefined;
  /**
   * The base implementation of `getTag` without fallbacks for buggy environments.
   *
   * @private
   * @param {*} value The value to query.
   * @returns {string} Returns the `toStringTag`.
   */

  function baseGetTag$2(value) {
    if (value == null) {
      return value === undefined ? undefinedTag$2 : nullTag$2;
    }

    return symToStringTag$5 && symToStringTag$5 in Object(value) ? getRawTag$2(value) : objectToString$2(value);
  }

  /**
   * Checks if `value` is the
   * [language type](http://www.ecma-international.org/ecma-262/7.0/#sec-ecmascript-language-types)
   * of `Object`. (e.g. arrays, functions, objects, regexes, `new Number(0)`, and `new String('')`)
   *
   * @static
   * @memberOf _
   * @since 0.1.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is an object, else `false`.
   * @example
   *
   * _.isObject({});
   * // => true
   *
   * _.isObject([1, 2, 3]);
   * // => true
   *
   * _.isObject(_.noop);
   * // => true
   *
   * _.isObject(null);
   * // => false
   */
  function isObject$2(value) {
    var type = _typeof$1(value);

    return value != null && (type == 'object' || type == 'function');
  }

  /** `Object#toString` result references. */

  var asyncTag$2 = '[object AsyncFunction]',
      funcTag$4 = '[object Function]',
      genTag$2 = '[object GeneratorFunction]',
      proxyTag$2 = '[object Proxy]';
  /**
   * Checks if `value` is classified as a `Function` object.
   *
   * @static
   * @memberOf _
   * @since 0.1.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a function, else `false`.
   * @example
   *
   * _.isFunction(_);
   * // => true
   *
   * _.isFunction(/abc/);
   * // => false
   */

  function isFunction$2(value) {
    if (!isObject$2(value)) {
      return false;
    } // The use of `Object#toString` avoids issues with the `typeof` operator
    // in Safari 9 which returns 'object' for typed arrays and other constructors.


    var tag = baseGetTag$2(value);
    return tag == funcTag$4 || tag == genTag$2 || tag == asyncTag$2 || tag == proxyTag$2;
  }

  /** Used to detect overreaching core-js shims. */

  var coreJsData$2 = root$2['__core-js_shared__'];

  /** Used to detect methods masquerading as native. */

  var maskSrcKey$2 = function () {
    var uid = /[^.]+$/.exec(coreJsData$2 && coreJsData$2.keys && coreJsData$2.keys.IE_PROTO || '');
    return uid ? 'Symbol(src)_1.' + uid : '';
  }();
  /**
   * Checks if `func` has its source masked.
   *
   * @private
   * @param {Function} func The function to check.
   * @returns {boolean} Returns `true` if `func` is masked, else `false`.
   */


  function isMasked$2(func) {
    return !!maskSrcKey$2 && maskSrcKey$2 in func;
  }

  /** Used for built-in method references. */
  var funcProto$4 = Function.prototype;
  /** Used to resolve the decompiled source of functions. */

  var funcToString$4 = funcProto$4.toString;
  /**
   * Converts `func` to its source code.
   *
   * @private
   * @param {Function} func The function to convert.
   * @returns {string} Returns the source code.
   */

  function toSource$2(func) {
    if (func != null) {
      try {
        return funcToString$4.call(func);
      } catch (e) {}

      try {
        return func + '';
      } catch (e) {}
    }

    return '';
  }

  /**
   * Used to match `RegExp`
   * [syntax characters](http://ecma-international.org/ecma-262/7.0/#sec-patterns).
   */

  var reRegExpChar$2 = /[\\^$.*+?()[\]{}|]/g;
  /** Used to detect host constructors (Safari). */

  var reIsHostCtor$2 = /^\[object .+?Constructor\]$/;
  /** Used for built-in method references. */

  var funcProto$5 = Function.prototype,
      objectProto$p = Object.prototype;
  /** Used to resolve the decompiled source of functions. */

  var funcToString$5 = funcProto$5.toString;
  /** Used to check objects for own properties. */

  var hasOwnProperty$j = objectProto$p.hasOwnProperty;
  /** Used to detect if a method is native. */

  var reIsNative$2 = RegExp('^' + funcToString$5.call(hasOwnProperty$j).replace(reRegExpChar$2, '\\$&').replace(/hasOwnProperty|(function).*?(?=\\\()| for .+?(?=\\\])/g, '$1.*?') + '$');
  /**
   * The base implementation of `_.isNative` without bad shim checks.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a native function,
   *  else `false`.
   */

  function baseIsNative$2(value) {
    if (!isObject$2(value) || isMasked$2(value)) {
      return false;
    }

    var pattern = isFunction$2(value) ? reIsNative$2 : reIsHostCtor$2;
    return pattern.test(toSource$2(value));
  }

  /**
   * Gets the value at `key` of `object`.
   *
   * @private
   * @param {Object} [object] The object to query.
   * @param {string} key The key of the property to get.
   * @returns {*} Returns the property value.
   */
  function getValue$2(object, key) {
    return object == null ? undefined : object[key];
  }

  /**
   * Gets the native function at `key` of `object`.
   *
   * @private
   * @param {Object} object The object to query.
   * @param {string} key The key of the method to get.
   * @returns {*} Returns the function if it's native, else `undefined`.
   */

  function getNative$2(object, key) {
    var value = getValue$2(object, key);
    return baseIsNative$2(value) ? value : undefined;
  }

  var defineProperty$2 = function () {
    try {
      var func = getNative$2(Object, 'defineProperty');
      func({}, '', {});
      return func;
    } catch (e) {}
  }();

  /**
   * The base implementation of `assignValue` and `assignMergeValue` without
   * value checks.
   *
   * @private
   * @param {Object} object The object to modify.
   * @param {string} key The key of the property to assign.
   * @param {*} value The value to assign.
   */

  function baseAssignValue$2(object, key, value) {
    if (key == '__proto__' && defineProperty$2) {
      defineProperty$2(object, key, {
        'configurable': true,
        'enumerable': true,
        'value': value,
        'writable': true
      });
    } else {
      object[key] = value;
    }
  }

  /**
   * Performs a
   * [`SameValueZero`](http://ecma-international.org/ecma-262/7.0/#sec-samevaluezero)
   * comparison between two values to determine if they are equivalent.
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to compare.
   * @param {*} other The other value to compare.
   * @returns {boolean} Returns `true` if the values are equivalent, else `false`.
   * @example
   *
   * var object = { 'a': 1 };
   * var other = { 'a': 1 };
   *
   * _.eq(object, object);
   * // => true
   *
   * _.eq(object, other);
   * // => false
   *
   * _.eq('a', 'a');
   * // => true
   *
   * _.eq('a', Object('a'));
   * // => false
   *
   * _.eq(NaN, NaN);
   * // => true
   */
  function eq$2(value, other) {
    return value === other || value !== value && other !== other;
  }

  /** Used for built-in method references. */

  var objectProto$q = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$k = objectProto$q.hasOwnProperty;
  /**
   * Assigns `value` to `key` of `object` if the existing value is not equivalent
   * using [`SameValueZero`](http://ecma-international.org/ecma-262/7.0/#sec-samevaluezero)
   * for equality comparisons.
   *
   * @private
   * @param {Object} object The object to modify.
   * @param {string} key The key of the property to assign.
   * @param {*} value The value to assign.
   */

  function assignValue$2(object, key, value) {
    var objValue = object[key];

    if (!(hasOwnProperty$k.call(object, key) && eq$2(objValue, value)) || value === undefined && !(key in object)) {
      baseAssignValue$2(object, key, value);
    }
  }

  /**
   * Copies properties of `source` to `object`.
   *
   * @private
   * @param {Object} source The object to copy properties from.
   * @param {Array} props The property identifiers to copy.
   * @param {Object} [object={}] The object to copy properties to.
   * @param {Function} [customizer] The function to customize copied values.
   * @returns {Object} Returns `object`.
   */

  function copyObject$2(source, props, object, customizer) {
    var isNew = !object;
    object || (object = {});
    var index = -1,
        length = props.length;

    while (++index < length) {
      var key = props[index];
      var newValue = customizer ? customizer(object[key], source[key], key, object, source) : undefined;

      if (newValue === undefined) {
        newValue = source[key];
      }

      if (isNew) {
        baseAssignValue$2(object, key, newValue);
      } else {
        assignValue$2(object, key, newValue);
      }
    }

    return object;
  }

  /**
   * This method returns the first argument it receives.
   *
   * @static
   * @since 0.1.0
   * @memberOf _
   * @category Util
   * @param {*} value Any value.
   * @returns {*} Returns `value`.
   * @example
   *
   * var object = { 'a': 1 };
   *
   * console.log(_.identity(object) === object);
   * // => true
   */
  function identity$2(value) {
    return value;
  }

  /**
   * A faster alternative to `Function#apply`, this function invokes `func`
   * with the `this` binding of `thisArg` and the arguments of `args`.
   *
   * @private
   * @param {Function} func The function to invoke.
   * @param {*} thisArg The `this` binding of `func`.
   * @param {Array} args The arguments to invoke `func` with.
   * @returns {*} Returns the result of `func`.
   */
  function apply$2(func, thisArg, args) {
    switch (args.length) {
      case 0:
        return func.call(thisArg);

      case 1:
        return func.call(thisArg, args[0]);

      case 2:
        return func.call(thisArg, args[0], args[1]);

      case 3:
        return func.call(thisArg, args[0], args[1], args[2]);
    }

    return func.apply(thisArg, args);
  }

  /* Built-in method references for those with the same name as other `lodash` methods. */

  var nativeMax$5 = Math.max;
  /**
   * A specialized version of `baseRest` which transforms the rest array.
   *
   * @private
   * @param {Function} func The function to apply a rest parameter to.
   * @param {number} [start=func.length-1] The start position of the rest parameter.
   * @param {Function} transform The rest array transform.
   * @returns {Function} Returns the new function.
   */

  function overRest$2(func, start, transform) {
    start = nativeMax$5(start === undefined ? func.length - 1 : start, 0);
    return function () {
      var args = arguments,
          index = -1,
          length = nativeMax$5(args.length - start, 0),
          array = Array(length);

      while (++index < length) {
        array[index] = args[start + index];
      }

      index = -1;
      var otherArgs = Array(start + 1);

      while (++index < start) {
        otherArgs[index] = args[index];
      }

      otherArgs[start] = transform(array);
      return apply$2(func, this, otherArgs);
    };
  }

  /**
   * Creates a function that returns `value`.
   *
   * @static
   * @memberOf _
   * @since 2.4.0
   * @category Util
   * @param {*} value The value to return from the new function.
   * @returns {Function} Returns the new constant function.
   * @example
   *
   * var objects = _.times(2, _.constant({ 'a': 1 }));
   *
   * console.log(objects);
   * // => [{ 'a': 1 }, { 'a': 1 }]
   *
   * console.log(objects[0] === objects[1]);
   * // => true
   */
  function constant$2(value) {
    return function () {
      return value;
    };
  }

  /**
   * The base implementation of `setToString` without support for hot loop shorting.
   *
   * @private
   * @param {Function} func The function to modify.
   * @param {Function} string The `toString` result.
   * @returns {Function} Returns `func`.
   */

  var baseSetToString$2 = !defineProperty$2 ? identity$2 : function (func, string) {
    return defineProperty$2(func, 'toString', {
      'configurable': true,
      'enumerable': false,
      'value': constant$2(string),
      'writable': true
    });
  };

  /** Used to detect hot functions by number of calls within a span of milliseconds. */
  var HOT_COUNT$2 = 800,
      HOT_SPAN$2 = 16;
  /* Built-in method references for those with the same name as other `lodash` methods. */

  var nativeNow$2 = Date.now;
  /**
   * Creates a function that'll short out and invoke `identity` instead
   * of `func` when it's called `HOT_COUNT` or more times in `HOT_SPAN`
   * milliseconds.
   *
   * @private
   * @param {Function} func The function to restrict.
   * @returns {Function} Returns the new shortable function.
   */

  function shortOut$2(func) {
    var count = 0,
        lastCalled = 0;
    return function () {
      var stamp = nativeNow$2(),
          remaining = HOT_SPAN$2 - (stamp - lastCalled);
      lastCalled = stamp;

      if (remaining > 0) {
        if (++count >= HOT_COUNT$2) {
          return arguments[0];
        }
      } else {
        count = 0;
      }

      return func.apply(undefined, arguments);
    };
  }

  /**
   * Sets the `toString` method of `func` to return `string`.
   *
   * @private
   * @param {Function} func The function to modify.
   * @param {Function} string The `toString` result.
   * @returns {Function} Returns `func`.
   */

  var setToString$2 = shortOut$2(baseSetToString$2);

  /**
   * The base implementation of `_.rest` which doesn't validate or coerce arguments.
   *
   * @private
   * @param {Function} func The function to apply a rest parameter to.
   * @param {number} [start=func.length-1] The start position of the rest parameter.
   * @returns {Function} Returns the new function.
   */

  function baseRest$2(func, start) {
    return setToString$2(overRest$2(func, start, identity$2), func + '');
  }

  /** Used as references for various `Number` constants. */
  var MAX_SAFE_INTEGER$4 = 9007199254740991;
  /**
   * Checks if `value` is a valid array-like length.
   *
   * **Note:** This method is loosely based on
   * [`ToLength`](http://ecma-international.org/ecma-262/7.0/#sec-tolength).
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a valid length, else `false`.
   * @example
   *
   * _.isLength(3);
   * // => true
   *
   * _.isLength(Number.MIN_VALUE);
   * // => false
   *
   * _.isLength(Infinity);
   * // => false
   *
   * _.isLength('3');
   * // => false
   */

  function isLength$2(value) {
    return typeof value == 'number' && value > -1 && value % 1 == 0 && value <= MAX_SAFE_INTEGER$4;
  }

  /**
   * Checks if `value` is array-like. A value is considered array-like if it's
   * not a function and has a `value.length` that's an integer greater than or
   * equal to `0` and less than or equal to `Number.MAX_SAFE_INTEGER`.
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is array-like, else `false`.
   * @example
   *
   * _.isArrayLike([1, 2, 3]);
   * // => true
   *
   * _.isArrayLike(document.body.children);
   * // => true
   *
   * _.isArrayLike('abc');
   * // => true
   *
   * _.isArrayLike(_.noop);
   * // => false
   */

  function isArrayLike$2(value) {
    return value != null && isLength$2(value.length) && !isFunction$2(value);
  }

  /** Used as references for various `Number` constants. */
  var MAX_SAFE_INTEGER$5 = 9007199254740991;
  /** Used to detect unsigned integer values. */

  var reIsUint$2 = /^(?:0|[1-9]\d*)$/;
  /**
   * Checks if `value` is a valid array-like index.
   *
   * @private
   * @param {*} value The value to check.
   * @param {number} [length=MAX_SAFE_INTEGER] The upper bounds of a valid index.
   * @returns {boolean} Returns `true` if `value` is a valid index, else `false`.
   */

  function isIndex$2(value, length) {
    var type = _typeof$1(value);

    length = length == null ? MAX_SAFE_INTEGER$5 : length;
    return !!length && (type == 'number' || type != 'symbol' && reIsUint$2.test(value)) && value > -1 && value % 1 == 0 && value < length;
  }

  /**
   * Checks if the given arguments are from an iteratee call.
   *
   * @private
   * @param {*} value The potential iteratee value argument.
   * @param {*} index The potential iteratee index or key argument.
   * @param {*} object The potential iteratee object argument.
   * @returns {boolean} Returns `true` if the arguments are from an iteratee call,
   *  else `false`.
   */

  function isIterateeCall$2(value, index, object) {
    if (!isObject$2(object)) {
      return false;
    }

    var type = _typeof$1(index);

    if (type == 'number' ? isArrayLike$2(object) && isIndex$2(index, object.length) : type == 'string' && index in object) {
      return eq$2(object[index], value);
    }

    return false;
  }

  /**
   * Creates a function like `_.assign`.
   *
   * @private
   * @param {Function} assigner The function to assign values.
   * @returns {Function} Returns the new assigner function.
   */

  function createAssigner$2(assigner) {
    return baseRest$2(function (object, sources) {
      var index = -1,
          length = sources.length,
          customizer = length > 1 ? sources[length - 1] : undefined,
          guard = length > 2 ? sources[2] : undefined;
      customizer = assigner.length > 3 && typeof customizer == 'function' ? (length--, customizer) : undefined;

      if (guard && isIterateeCall$2(sources[0], sources[1], guard)) {
        customizer = length < 3 ? undefined : customizer;
        length = 1;
      }

      object = Object(object);

      while (++index < length) {
        var source = sources[index];

        if (source) {
          assigner(object, source, index, customizer);
        }
      }

      return object;
    });
  }

  /** Used for built-in method references. */
  var objectProto$r = Object.prototype;
  /**
   * Checks if `value` is likely a prototype object.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a prototype, else `false`.
   */

  function isPrototype$2(value) {
    var Ctor = value && value.constructor,
        proto = typeof Ctor == 'function' && Ctor.prototype || objectProto$r;
    return value === proto;
  }

  /**
   * The base implementation of `_.times` without support for iteratee shorthands
   * or max array length checks.
   *
   * @private
   * @param {number} n The number of times to invoke `iteratee`.
   * @param {Function} iteratee The function invoked per iteration.
   * @returns {Array} Returns the array of results.
   */
  function baseTimes$2(n, iteratee) {
    var index = -1,
        result = Array(n);

    while (++index < n) {
      result[index] = iteratee(index);
    }

    return result;
  }

  /**
   * Checks if `value` is object-like. A value is object-like if it's not `null`
   * and has a `typeof` result of "object".
   *
   * @static
   * @memberOf _
   * @since 4.0.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is object-like, else `false`.
   * @example
   *
   * _.isObjectLike({});
   * // => true
   *
   * _.isObjectLike([1, 2, 3]);
   * // => true
   *
   * _.isObjectLike(_.noop);
   * // => false
   *
   * _.isObjectLike(null);
   * // => false
   */
  function isObjectLike$2(value) {
    return value != null && _typeof$1(value) == 'object';
  }

  /** `Object#toString` result references. */

  var argsTag$5 = '[object Arguments]';
  /**
   * The base implementation of `_.isArguments`.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is an `arguments` object,
   */

  function baseIsArguments$2(value) {
    return isObjectLike$2(value) && baseGetTag$2(value) == argsTag$5;
  }

  /** Used for built-in method references. */

  var objectProto$s = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$l = objectProto$s.hasOwnProperty;
  /** Built-in value references. */

  var propertyIsEnumerable$3 = objectProto$s.propertyIsEnumerable;
  /**
   * Checks if `value` is likely an `arguments` object.
   *
   * @static
   * @memberOf _
   * @since 0.1.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is an `arguments` object,
   *  else `false`.
   * @example
   *
   * _.isArguments(function() { return arguments; }());
   * // => true
   *
   * _.isArguments([1, 2, 3]);
   * // => false
   */

  var isArguments$2 = baseIsArguments$2(function () {
    return arguments;
  }()) ? baseIsArguments$2 : function (value) {
    return isObjectLike$2(value) && hasOwnProperty$l.call(value, 'callee') && !propertyIsEnumerable$3.call(value, 'callee');
  };

  /**
   * Checks if `value` is classified as an `Array` object.
   *
   * @static
   * @memberOf _
   * @since 0.1.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is an array, else `false`.
   * @example
   *
   * _.isArray([1, 2, 3]);
   * // => true
   *
   * _.isArray(document.body.children);
   * // => false
   *
   * _.isArray('abc');
   * // => false
   *
   * _.isArray(_.noop);
   * // => false
   */
  var isArray$2 = Array.isArray;

  /**
   * This method returns `false`.
   *
   * @static
   * @memberOf _
   * @since 4.13.0
   * @category Util
   * @returns {boolean} Returns `false`.
   * @example
   *
   * _.times(2, _.stubFalse);
   * // => [false, false]
   */
  function stubFalse$2() {
    return false;
  }

  /** Detect free variable `exports`. */

  var freeExports$4 = (typeof exports === "undefined" ? "undefined" : _typeof$1(exports)) == 'object' && exports && !exports.nodeType && exports;
  /** Detect free variable `module`. */

  var freeModule$4 = freeExports$4 && (typeof module === "undefined" ? "undefined" : _typeof$1(module)) == 'object' && module && !module.nodeType && module;
  /** Detect the popular CommonJS extension `module.exports`. */

  var moduleExports$4 = freeModule$4 && freeModule$4.exports === freeExports$4;
  /** Built-in value references. */

  var Buffer$2 = moduleExports$4 ? root$2.Buffer : undefined;
  /* Built-in method references for those with the same name as other `lodash` methods. */

  var nativeIsBuffer$2 = Buffer$2 ? Buffer$2.isBuffer : undefined;
  /**
   * Checks if `value` is a buffer.
   *
   * @static
   * @memberOf _
   * @since 4.3.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a buffer, else `false`.
   * @example
   *
   * _.isBuffer(new Buffer(2));
   * // => true
   *
   * _.isBuffer(new Uint8Array(2));
   * // => false
   */

  var isBuffer$2 = nativeIsBuffer$2 || stubFalse$2;

  /** `Object#toString` result references. */

  var argsTag$6 = '[object Arguments]',
      arrayTag$3 = '[object Array]',
      boolTag$3 = '[object Boolean]',
      dateTag$3 = '[object Date]',
      errorTag$3 = '[object Error]',
      funcTag$5 = '[object Function]',
      mapTag$4 = '[object Map]',
      numberTag$3 = '[object Number]',
      objectTag$4 = '[object Object]',
      regexpTag$3 = '[object RegExp]',
      setTag$4 = '[object Set]',
      stringTag$3 = '[object String]',
      weakMapTag$3 = '[object WeakMap]';
  var arrayBufferTag$3 = '[object ArrayBuffer]',
      dataViewTag$4 = '[object DataView]',
      float32Tag$2 = '[object Float32Array]',
      float64Tag$2 = '[object Float64Array]',
      int8Tag$2 = '[object Int8Array]',
      int16Tag$2 = '[object Int16Array]',
      int32Tag$2 = '[object Int32Array]',
      uint8Tag$2 = '[object Uint8Array]',
      uint8ClampedTag$2 = '[object Uint8ClampedArray]',
      uint16Tag$2 = '[object Uint16Array]',
      uint32Tag$2 = '[object Uint32Array]';
  /** Used to identify `toStringTag` values of typed arrays. */

  var typedArrayTags$2 = {};
  typedArrayTags$2[float32Tag$2] = typedArrayTags$2[float64Tag$2] = typedArrayTags$2[int8Tag$2] = typedArrayTags$2[int16Tag$2] = typedArrayTags$2[int32Tag$2] = typedArrayTags$2[uint8Tag$2] = typedArrayTags$2[uint8ClampedTag$2] = typedArrayTags$2[uint16Tag$2] = typedArrayTags$2[uint32Tag$2] = true;
  typedArrayTags$2[argsTag$6] = typedArrayTags$2[arrayTag$3] = typedArrayTags$2[arrayBufferTag$3] = typedArrayTags$2[boolTag$3] = typedArrayTags$2[dataViewTag$4] = typedArrayTags$2[dateTag$3] = typedArrayTags$2[errorTag$3] = typedArrayTags$2[funcTag$5] = typedArrayTags$2[mapTag$4] = typedArrayTags$2[numberTag$3] = typedArrayTags$2[objectTag$4] = typedArrayTags$2[regexpTag$3] = typedArrayTags$2[setTag$4] = typedArrayTags$2[stringTag$3] = typedArrayTags$2[weakMapTag$3] = false;
  /**
   * The base implementation of `_.isTypedArray` without Node.js optimizations.
   *
   * @private
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a typed array, else `false`.
   */

  function baseIsTypedArray$2(value) {
    return isObjectLike$2(value) && isLength$2(value.length) && !!typedArrayTags$2[baseGetTag$2(value)];
  }

  /**
   * The base implementation of `_.unary` without support for storing metadata.
   *
   * @private
   * @param {Function} func The function to cap arguments for.
   * @returns {Function} Returns the new capped function.
   */
  function baseUnary$2(func) {
    return function (value) {
      return func(value);
    };
  }

  /** Detect free variable `exports`. */

  var freeExports$5 = (typeof exports === "undefined" ? "undefined" : _typeof$1(exports)) == 'object' && exports && !exports.nodeType && exports;
  /** Detect free variable `module`. */

  var freeModule$5 = freeExports$5 && (typeof module === "undefined" ? "undefined" : _typeof$1(module)) == 'object' && module && !module.nodeType && module;
  /** Detect the popular CommonJS extension `module.exports`. */

  var moduleExports$5 = freeModule$5 && freeModule$5.exports === freeExports$5;
  /** Detect free variable `process` from Node.js. */

  var freeProcess$2 = moduleExports$5 && freeGlobal$2.process;
  /** Used to access faster Node.js helpers. */

  var nodeUtil$2 = function () {
    try {
      // Use `util.types` for Node.js 10+.
      var types = freeModule$5 && freeModule$5.require && freeModule$5.require('util').types;

      if (types) {
        return types;
      } // Legacy `process.binding('util')` for Node.js < 10.


      return freeProcess$2 && freeProcess$2.binding && freeProcess$2.binding('util');
    } catch (e) {}
  }();

  /* Node.js helper references. */

  var nodeIsTypedArray$2 = nodeUtil$2 && nodeUtil$2.isTypedArray;
  /**
   * Checks if `value` is classified as a typed array.
   *
   * @static
   * @memberOf _
   * @since 3.0.0
   * @category Lang
   * @param {*} value The value to check.
   * @returns {boolean} Returns `true` if `value` is a typed array, else `false`.
   * @example
   *
   * _.isTypedArray(new Uint8Array);
   * // => true
   *
   * _.isTypedArray([]);
   * // => false
   */

  var isTypedArray$2 = nodeIsTypedArray$2 ? baseUnary$2(nodeIsTypedArray$2) : baseIsTypedArray$2;

  /** Used for built-in method references. */

  var objectProto$t = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$m = objectProto$t.hasOwnProperty;
  /**
   * Creates an array of the enumerable property names of the array-like `value`.
   *
   * @private
   * @param {*} value The value to query.
   * @param {boolean} inherited Specify returning inherited property names.
   * @returns {Array} Returns the array of property names.
   */

  function arrayLikeKeys$2(value, inherited) {
    var isArr = isArray$2(value),
        isArg = !isArr && isArguments$2(value),
        isBuff = !isArr && !isArg && isBuffer$2(value),
        isType = !isArr && !isArg && !isBuff && isTypedArray$2(value),
        skipIndexes = isArr || isArg || isBuff || isType,
        result = skipIndexes ? baseTimes$2(value.length, String) : [],
        length = result.length;

    for (var key in value) {
      if ((inherited || hasOwnProperty$m.call(value, key)) && !(skipIndexes && ( // Safari 9 has enumerable `arguments.length` in strict mode.
      key == 'length' || // Node.js 0.10 has enumerable non-index properties on buffers.
      isBuff && (key == 'offset' || key == 'parent') || // PhantomJS 2 has enumerable non-index properties on typed arrays.
      isType && (key == 'buffer' || key == 'byteLength' || key == 'byteOffset') || // Skip index properties.
      isIndex$2(key, length)))) {
        result.push(key);
      }
    }

    return result;
  }

  /**
   * Creates a unary function that invokes `func` with its argument transformed.
   *
   * @private
   * @param {Function} func The function to wrap.
   * @param {Function} transform The argument transform.
   * @returns {Function} Returns the new function.
   */
  function overArg$2(func, transform) {
    return function (arg) {
      return func(transform(arg));
    };
  }

  /* Built-in method references for those with the same name as other `lodash` methods. */

  var nativeKeys$2 = overArg$2(Object.keys, Object);

  /** Used for built-in method references. */

  var objectProto$u = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$n = objectProto$u.hasOwnProperty;
  /**
   * The base implementation of `_.keys` which doesn't treat sparse arrays as dense.
   *
   * @private
   * @param {Object} object The object to query.
   * @returns {Array} Returns the array of property names.
   */

  function baseKeys$2(object) {
    if (!isPrototype$2(object)) {
      return nativeKeys$2(object);
    }

    var result = [];

    for (var key in Object(object)) {
      if (hasOwnProperty$n.call(object, key) && key != 'constructor') {
        result.push(key);
      }
    }

    return result;
  }

  /**
   * Creates an array of the own enumerable property names of `object`.
   *
   * **Note:** Non-object values are coerced to objects. See the
   * [ES spec](http://ecma-international.org/ecma-262/7.0/#sec-object.keys)
   * for more details.
   *
   * @static
   * @since 0.1.0
   * @memberOf _
   * @category Object
   * @param {Object} object The object to query.
   * @returns {Array} Returns the array of property names.
   * @example
   *
   * function Foo() {
   *   this.a = 1;
   *   this.b = 2;
   * }
   *
   * Foo.prototype.c = 3;
   *
   * _.keys(new Foo);
   * // => ['a', 'b'] (iteration order is not guaranteed)
   *
   * _.keys('hi');
   * // => ['0', '1']
   */

  function keys$2(object) {
    return isArrayLike$2(object) ? arrayLikeKeys$2(object) : baseKeys$2(object);
  }

  /** Used for built-in method references. */

  var objectProto$v = Object.prototype;
  /** Used to check objects for own properties. */

  var hasOwnProperty$o = objectProto$v.hasOwnProperty;
  /**
   * Assigns own enumerable string keyed properties of source objects to the
   * destination object. Source objects are applied from left to right.
   * Subsequent sources overwrite property assignments of previous sources.
   *
   * **Note:** This method mutates `object` and is loosely based on
   * [`Object.assign`](https://mdn.io/Object/assign).
   *
   * @static
   * @memberOf _
   * @since 0.10.0
   * @category Object
   * @param {Object} object The destination object.
   * @param {...Object} [sources] The source objects.
   * @returns {Object} Returns `object`.
   * @see _.assignIn
   * @example
   *
   * function Foo() {
   *   this.a = 1;
   * }
   *
   * function Bar() {
   *   this.c = 3;
   * }
   *
   * Foo.prototype.b = 2;
   * Bar.prototype.d = 4;
   *
   * _.assign({ 'a': 0 }, new Foo, new Bar);
   * // => { 'a': 1, 'c': 3 }
   */

  var assign$2 = createAssigner$2(function (object, source) {
    if (isPrototype$2(source) || isArrayLike$2(source)) {
      copyObject$2(source, keys$2(source), object);
      return;
    }

    for (var key in source) {
      if (hasOwnProperty$o.call(source, key)) {
        assignValue$2(object, key, source[key]);
      }
    }
  });

  var name = "@mixxx-launchpad/mk1";
  var version = "1.0.0";
  var main = "src/index.js";
  var module$1 = "src/index.js";
  var dependencies = {
  	"@babel/runtime": "^7.3.1",
  	"@mixxx-launchpad/app": "1.0.0",
  	"lodash-es": "~4.17.14"
  };
  var controller = {
  	device: "Launchpad",
  	manufacturer: "Novation",
  	global: "NovationLaunchpad",
  	path: "src"
  };
  var pkg = {
  	name: name,
  	"private": true,
  	version: version,
  	main: main,
  	module: module$1,
  	dependencies: dependencies,
  	controller: controller
  };

  var colors = {
    black: 4,
    lo_red: 1 + 4,
    hi_red: 3 + 4,
    lo_green: 16 + 4,
    hi_green: 48 + 4,
    lo_amber: 17 + 4,
    hi_amber: 51 + 4,
    hi_orange: 35 + 4,
    lo_orange: 18 + 4,
    hi_yellow: 50 + 4,
    lo_yellow: 33 + 4
  };

  var buttons = {
    /* eslint-disable key-spacing, no-multi-spaces */
    up: {
      status: 0xB0,
      midino: 0x68,
      name: 'up'
    },
    down: {
      status: 0xB0,
      midino: 0x69,
      name: 'down'
    },
    left: {
      status: 0xB0,
      midino: 0x6A,
      name: 'left'
    },
    right: {
      status: 0xB0,
      midino: 0x6B,
      name: 'right'
    },
    session: {
      status: 0xB0,
      midino: 0x6C,
      name: 'session'
    },
    user1: {
      status: 0xB0,
      midino: 0x6D,
      name: 'user1'
    },
    user2: {
      status: 0xB0,
      midino: 0x6E,
      name: 'user2'
    },
    mixer: {
      status: 0xB0,
      midino: 0x6F,
      name: 'mixer'
    },
    vol: {
      status: 0x90,
      midino: 0x08,
      name: 'vol'
    },
    pan: {
      status: 0x90,
      midino: 0x18,
      name: 'pan'
    },
    snda: {
      status: 0x90,
      midino: 0x28,
      name: 'snda'
    },
    sndb: {
      status: 0x90,
      midino: 0x38,
      name: 'sndb'
    },
    stop: {
      status: 0x90,
      midino: 0x48,
      name: 'stop'
    },
    trkon: {
      status: 0x90,
      midino: 0x58,
      name: 'trkon'
    },
    solo: {
      status: 0x90,
      midino: 0x68,
      name: 'solo'
    },
    arm: {
      status: 0x90,
      midino: 0x78,
      name: 'arm'
    },
    '0,0': {
      status: 0x90,
      midino: 0x00,
      name: '0,0'
    },
    '0,1': {
      status: 0x90,
      midino: 0x01,
      name: '0,1'
    },
    '0,2': {
      status: 0x90,
      midino: 0x02,
      name: '0,2'
    },
    '0,3': {
      status: 0x90,
      midino: 0x03,
      name: '0,3'
    },
    '0,4': {
      status: 0x90,
      midino: 0x04,
      name: '0,4'
    },
    '0,5': {
      status: 0x90,
      midino: 0x05,
      name: '0,5'
    },
    '0,6': {
      status: 0x90,
      midino: 0x06,
      name: '0,6'
    },
    '0,7': {
      status: 0x90,
      midino: 0x07,
      name: '0,7'
    },
    '1,0': {
      status: 0x90,
      midino: 0x10,
      name: '1,0'
    },
    '1,1': {
      status: 0x90,
      midino: 0x11,
      name: '1,1'
    },
    '1,2': {
      status: 0x90,
      midino: 0x12,
      name: '1,2'
    },
    '1,3': {
      status: 0x90,
      midino: 0x13,
      name: '1,3'
    },
    '1,4': {
      status: 0x90,
      midino: 0x14,
      name: '1,4'
    },
    '1,5': {
      status: 0x90,
      midino: 0x15,
      name: '1,5'
    },
    '1,6': {
      status: 0x90,
      midino: 0x16,
      name: '1,6'
    },
    '1,7': {
      status: 0x90,
      midino: 0x17,
      name: '1,7'
    },
    '2,0': {
      status: 0x90,
      midino: 0x20,
      name: '2,0'
    },
    '2,1': {
      status: 0x90,
      midino: 0x21,
      name: '2,1'
    },
    '2,2': {
      status: 0x90,
      midino: 0x22,
      name: '2,2'
    },
    '2,3': {
      status: 0x90,
      midino: 0x23,
      name: '2,3'
    },
    '2,4': {
      status: 0x90,
      midino: 0x24,
      name: '2,4'
    },
    '2,5': {
      status: 0x90,
      midino: 0x25,
      name: '2,5'
    },
    '2,6': {
      status: 0x90,
      midino: 0x26,
      name: '2,6'
    },
    '2,7': {
      status: 0x90,
      midino: 0x27,
      name: '2,7'
    },
    '3,0': {
      status: 0x90,
      midino: 0x30,
      name: '3,0'
    },
    '3,1': {
      status: 0x90,
      midino: 0x31,
      name: '3,1'
    },
    '3,2': {
      status: 0x90,
      midino: 0x32,
      name: '3,2'
    },
    '3,3': {
      status: 0x90,
      midino: 0x33,
      name: '3,3'
    },
    '3,4': {
      status: 0x90,
      midino: 0x34,
      name: '3,4'
    },
    '3,5': {
      status: 0x90,
      midino: 0x35,
      name: '3,5'
    },
    '3,6': {
      status: 0x90,
      midino: 0x36,
      name: '3,6'
    },
    '3,7': {
      status: 0x90,
      midino: 0x37,
      name: '3,7'
    },
    '4,0': {
      status: 0x90,
      midino: 0x40,
      name: '4,0'
    },
    '4,1': {
      status: 0x90,
      midino: 0x41,
      name: '4,1'
    },
    '4,2': {
      status: 0x90,
      midino: 0x42,
      name: '4,2'
    },
    '4,3': {
      status: 0x90,
      midino: 0x43,
      name: '4,3'
    },
    '4,4': {
      status: 0x90,
      midino: 0x44,
      name: '4,4'
    },
    '4,5': {
      status: 0x90,
      midino: 0x45,
      name: '4,5'
    },
    '4,6': {
      status: 0x90,
      midino: 0x46,
      name: '4,6'
    },
    '4,7': {
      status: 0x90,
      midino: 0x47,
      name: '4,7'
    },
    '5,0': {
      status: 0x90,
      midino: 0x50,
      name: '5,0'
    },
    '5,1': {
      status: 0x90,
      midino: 0x51,
      name: '5,1'
    },
    '5,2': {
      status: 0x90,
      midino: 0x52,
      name: '5,2'
    },
    '5,3': {
      status: 0x90,
      midino: 0x53,
      name: '5,3'
    },
    '5,4': {
      status: 0x90,
      midino: 0x54,
      name: '5,4'
    },
    '5,5': {
      status: 0x90,
      midino: 0x55,
      name: '5,5'
    },
    '5,6': {
      status: 0x90,
      midino: 0x56,
      name: '5,6'
    },
    '5,7': {
      status: 0x90,
      midino: 0x57,
      name: '5,7'
    },
    '6,0': {
      status: 0x90,
      midino: 0x60,
      name: '6,0'
    },
    '6,1': {
      status: 0x90,
      midino: 0x61,
      name: '6,1'
    },
    '6,2': {
      status: 0x90,
      midino: 0x62,
      name: '6,2'
    },
    '6,3': {
      status: 0x90,
      midino: 0x63,
      name: '6,3'
    },
    '6,4': {
      status: 0x90,
      midino: 0x64,
      name: '6,4'
    },
    '6,5': {
      status: 0x90,
      midino: 0x65,
      name: '6,5'
    },
    '6,6': {
      status: 0x90,
      midino: 0x66,
      name: '6,6'
    },
    '6,7': {
      status: 0x90,
      midino: 0x67,
      name: '6,7'
    },
    '7,0': {
      status: 0x90,
      midino: 0x70,
      name: '7,0'
    },
    '7,1': {
      status: 0x90,
      midino: 0x71,
      name: '7,1'
    },
    '7,2': {
      status: 0x90,
      midino: 0x72,
      name: '7,2'
    },
    '7,3': {
      status: 0x90,
      midino: 0x73,
      name: '7,3'
    },
    '7,4': {
      status: 0x90,
      midino: 0x74,
      name: '7,4'
    },
    '7,5': {
      status: 0x90,
      midino: 0x75,
      name: '7,5'
    },
    '7,6': {
      status: 0x90,
      midino: 0x76,
      name: '7,6'
    },
    '7,7': {
      status: 0x90,
      midino: 0x77,
      name: '7,7'
    }
    /* eslint-enable key-spacing, no-multi-spaces */

  };

  var LaunchpadMK1Device = /*#__PURE__*/function () {
    function LaunchpadMK1Device() {
      _classCallCheck(this, LaunchpadMK1Device);

      _defineProperty(this, "buttons", void 0);

      _defineProperty(this, "colors", void 0);

      this.buttons = Object.keys(buttons).reduce(function (obj, name) {
        return assign$2(obj, _defineProperty({}, name, new LaunchpadMidiButton(buttons[name])));
      }, {});
      this.colors = colors;
    }

    _createClass(LaunchpadMK1Device, [{
      key: "init",
      value: function init() {}
    }, {
      key: "shutdown",
      value: function shutdown() {}
    }]);

    return LaunchpadMK1Device;
  }();

  var index = create(pkg.controller.global, new LaunchpadMK1Device());

  return index;

}());
