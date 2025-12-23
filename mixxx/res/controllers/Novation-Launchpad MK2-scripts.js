/* eslint-disable */
/*
 * Novation-Launchpad MK2-scripts.js
 *
 * This file is generated. Do not edit directly.
 * Instead, edit the source file and regenerate it.
 * See https://github.com/dszakallas/mixxx-launchpad#building-from-source.
 *
 * Commit tag: v3.1.1-4-g4acd883-dirty
 * Commit hash: 4acd8832e0e93fef400055f7938c9f919036b7d6
 */
var NLMK2 = (function () {
	'use strict';

	var commonjsGlobal = typeof globalThis !== 'undefined' ? globalThis : typeof window !== 'undefined' ? window : typeof global !== 'undefined' ? global : typeof self !== 'undefined' ? self : {};

	function getDefaultExportFromCjs (x) {
		return x && x.__esModule && Object.prototype.hasOwnProperty.call(x, 'default') ? x['default'] : x;
	}

	var definePropertyExports$3 = {};
	var defineProperty$c = {
	  get exports(){ return definePropertyExports$3; },
	  set exports(v){ definePropertyExports$3 = v; },
	};

	var definePropertyExports$2 = {};
	var defineProperty$b = {
	  get exports(){ return definePropertyExports$2; },
	  set exports(v){ definePropertyExports$2 = v; },
	};

	var definePropertyExports$1 = {};
	var defineProperty$a = {
	  get exports(){ return definePropertyExports$1; },
	  set exports(v){ definePropertyExports$1 = v; },
	};

	var definePropertyExports = {};
	var defineProperty$9 = {
	  get exports(){ return definePropertyExports; },
	  set exports(v){ definePropertyExports = v; },
	};

	var check = function (it) {
	  return it && it.Math == Math && it;
	};

	// https://github.com/zloirock/core-js/issues/86#issuecomment-115759028
	var global$e =
	  // eslint-disable-next-line es/no-global-this -- safe
	  check(typeof globalThis == 'object' && globalThis) ||
	  check(typeof window == 'object' && window) ||
	  // eslint-disable-next-line no-restricted-globals -- safe
	  check(typeof self == 'object' && self) ||
	  check(typeof commonjsGlobal == 'object' && commonjsGlobal) ||
	  // eslint-disable-next-line no-new-func -- fallback
	  (function () { return this; })() || Function('return this')();

	var fails$f = function (exec) {
	  try {
	    return !!exec();
	  } catch (error) {
	    return true;
	  }
	};

	var fails$e = fails$f;

	var functionBindNative = !fails$e(function () {
	  // eslint-disable-next-line es/no-function-prototype-bind -- safe
	  var test = (function () { /* empty */ }).bind();
	  // eslint-disable-next-line no-prototype-builtins -- safe
	  return typeof test != 'function' || test.hasOwnProperty('prototype');
	});

	var NATIVE_BIND$3 = functionBindNative;

	var FunctionPrototype$2 = Function.prototype;
	var apply$2 = FunctionPrototype$2.apply;
	var call$c = FunctionPrototype$2.call;

	// eslint-disable-next-line es/no-reflect -- safe
	var functionApply = typeof Reflect == 'object' && Reflect.apply || (NATIVE_BIND$3 ? call$c.bind(apply$2) : function () {
	  return call$c.apply(apply$2, arguments);
	});

	var NATIVE_BIND$2 = functionBindNative;

	var FunctionPrototype$1 = Function.prototype;
	var call$b = FunctionPrototype$1.call;
	var uncurryThisWithBind = NATIVE_BIND$2 && FunctionPrototype$1.bind.bind(call$b, call$b);

	var functionUncurryThis = NATIVE_BIND$2 ? uncurryThisWithBind : function (fn) {
	  return function () {
	    return call$b.apply(fn, arguments);
	  };
	};

	var uncurryThis$i = functionUncurryThis;

	var toString$7 = uncurryThis$i({}.toString);
	var stringSlice$1 = uncurryThis$i(''.slice);

	var classofRaw$2 = function (it) {
	  return stringSlice$1(toString$7(it), 8, -1);
	};

	var classofRaw$1 = classofRaw$2;
	var uncurryThis$h = functionUncurryThis;

	var functionUncurryThisClause = function (fn) {
	  // Nashorn bug:
	  //   https://github.com/zloirock/core-js/issues/1128
	  //   https://github.com/zloirock/core-js/issues/1130
	  if (classofRaw$1(fn) === 'Function') return uncurryThis$h(fn);
	};

	var documentAll$2 = typeof document == 'object' && document.all;

	// https://tc39.es/ecma262/#sec-IsHTMLDDA-internal-slot
	// eslint-disable-next-line unicorn/no-typeof-undefined -- required for testing
	var IS_HTMLDDA = typeof documentAll$2 == 'undefined' && documentAll$2 !== undefined;

	var documentAll_1 = {
	  all: documentAll$2,
	  IS_HTMLDDA: IS_HTMLDDA
	};

	var $documentAll$1 = documentAll_1;

	var documentAll$1 = $documentAll$1.all;

	// `IsCallable` abstract operation
	// https://tc39.es/ecma262/#sec-iscallable
	var isCallable$f = $documentAll$1.IS_HTMLDDA ? function (argument) {
	  return typeof argument == 'function' || argument === documentAll$1;
	} : function (argument) {
	  return typeof argument == 'function';
	};

	var objectGetOwnPropertyDescriptor = {};

	var fails$d = fails$f;

	// Detect IE8's incomplete defineProperty implementation
	var descriptors = !fails$d(function () {
	  // eslint-disable-next-line es/no-object-defineproperty -- required for testing
	  return Object.defineProperty({}, 1, { get: function () { return 7; } })[1] != 7;
	});

	var NATIVE_BIND$1 = functionBindNative;

	var call$a = Function.prototype.call;

	var functionCall = NATIVE_BIND$1 ? call$a.bind(call$a) : function () {
	  return call$a.apply(call$a, arguments);
	};

	var objectPropertyIsEnumerable = {};

	var $propertyIsEnumerable$1 = {}.propertyIsEnumerable;
	// eslint-disable-next-line es/no-object-getownpropertydescriptor -- safe
	var getOwnPropertyDescriptor$1 = Object.getOwnPropertyDescriptor;

	// Nashorn ~ JDK8 bug
	var NASHORN_BUG = getOwnPropertyDescriptor$1 && !$propertyIsEnumerable$1.call({ 1: 2 }, 1);

	// `Object.prototype.propertyIsEnumerable` method implementation
	// https://tc39.es/ecma262/#sec-object.prototype.propertyisenumerable
	objectPropertyIsEnumerable.f = NASHORN_BUG ? function propertyIsEnumerable(V) {
	  var descriptor = getOwnPropertyDescriptor$1(this, V);
	  return !!descriptor && descriptor.enumerable;
	} : $propertyIsEnumerable$1;

	var createPropertyDescriptor$5 = function (bitmap, value) {
	  return {
	    enumerable: !(bitmap & 1),
	    configurable: !(bitmap & 2),
	    writable: !(bitmap & 4),
	    value: value
	  };
	};

	var uncurryThis$g = functionUncurryThis;
	var fails$c = fails$f;
	var classof$9 = classofRaw$2;

	var $Object$4 = Object;
	var split = uncurryThis$g(''.split);

	// fallback for non-array-like ES3 and non-enumerable old V8 strings
	var indexedObject = fails$c(function () {
	  // throws an error in rhino, see https://github.com/mozilla/rhino/issues/346
	  // eslint-disable-next-line no-prototype-builtins -- safe
	  return !$Object$4('z').propertyIsEnumerable(0);
	}) ? function (it) {
	  return classof$9(it) == 'String' ? split(it, '') : $Object$4(it);
	} : $Object$4;

	// we can't use just `it == null` since of `document.all` special case
	// https://tc39.es/ecma262/#sec-IsHTMLDDA-internal-slot-aec
	var isNullOrUndefined$3 = function (it) {
	  return it === null || it === undefined;
	};

	var isNullOrUndefined$2 = isNullOrUndefined$3;

	var $TypeError$8 = TypeError;

	// `RequireObjectCoercible` abstract operation
	// https://tc39.es/ecma262/#sec-requireobjectcoercible
	var requireObjectCoercible$3 = function (it) {
	  if (isNullOrUndefined$2(it)) throw $TypeError$8("Can't call method on " + it);
	  return it;
	};

	// toObject with fallback for non-array-like ES3 strings
	var IndexedObject$1 = indexedObject;
	var requireObjectCoercible$2 = requireObjectCoercible$3;

	var toIndexedObject$7 = function (it) {
	  return IndexedObject$1(requireObjectCoercible$2(it));
	};

	var isCallable$e = isCallable$f;
	var $documentAll = documentAll_1;

	var documentAll = $documentAll.all;

	var isObject$8 = $documentAll.IS_HTMLDDA ? function (it) {
	  return typeof it == 'object' ? it !== null : isCallable$e(it) || it === documentAll;
	} : function (it) {
	  return typeof it == 'object' ? it !== null : isCallable$e(it);
	};

	var path$7 = {};

	var path$6 = path$7;
	var global$d = global$e;
	var isCallable$d = isCallable$f;

	var aFunction = function (variable) {
	  return isCallable$d(variable) ? variable : undefined;
	};

	var getBuiltIn$9 = function (namespace, method) {
	  return arguments.length < 2 ? aFunction(path$6[namespace]) || aFunction(global$d[namespace])
	    : path$6[namespace] && path$6[namespace][method] || global$d[namespace] && global$d[namespace][method];
	};

	var uncurryThis$f = functionUncurryThis;

	var objectIsPrototypeOf = uncurryThis$f({}.isPrototypeOf);

	var engineUserAgent = typeof navigator != 'undefined' && String(navigator.userAgent) || '';

	var global$c = global$e;
	var userAgent = engineUserAgent;

	var process = global$c.process;
	var Deno = global$c.Deno;
	var versions = process && process.versions || Deno && Deno.version;
	var v8 = versions && versions.v8;
	var match, version;

	if (v8) {
	  match = v8.split('.');
	  // in old Chrome, versions of V8 isn't V8 = Chrome / 10
	  // but their correct versions are not interesting for us
	  version = match[0] > 0 && match[0] < 4 ? 1 : +(match[0] + match[1]);
	}

	// BrowserFS NodeJS `process` polyfill incorrectly set `.v8` to `0.0`
	// so check `userAgent` even if `.v8` exists, but 0
	if (!version && userAgent) {
	  match = userAgent.match(/Edge\/(\d+)/);
	  if (!match || match[1] >= 74) {
	    match = userAgent.match(/Chrome\/(\d+)/);
	    if (match) version = +match[1];
	  }
	}

	var engineV8Version = version;

	/* eslint-disable es/no-symbol -- required for testing */

	var V8_VERSION$2 = engineV8Version;
	var fails$b = fails$f;

	// eslint-disable-next-line es/no-object-getownpropertysymbols -- required for testing
	var symbolConstructorDetection = !!Object.getOwnPropertySymbols && !fails$b(function () {
	  var symbol = Symbol();
	  // Chrome 38 Symbol has incorrect toString conversion
	  // `get-own-property-symbols` polyfill symbols converted to object are not Symbol instances
	  return !String(symbol) || !(Object(symbol) instanceof Symbol) ||
	    // Chrome 38-40 symbols are not inherited from DOM collections prototypes to instances
	    !Symbol.sham && V8_VERSION$2 && V8_VERSION$2 < 41;
	});

	/* eslint-disable es/no-symbol -- required for testing */

	var NATIVE_SYMBOL$5 = symbolConstructorDetection;

	var useSymbolAsUid = NATIVE_SYMBOL$5
	  && !Symbol.sham
	  && typeof Symbol.iterator == 'symbol';

	var getBuiltIn$8 = getBuiltIn$9;
	var isCallable$c = isCallable$f;
	var isPrototypeOf$3 = objectIsPrototypeOf;
	var USE_SYMBOL_AS_UID$1 = useSymbolAsUid;

	var $Object$3 = Object;

	var isSymbol$5 = USE_SYMBOL_AS_UID$1 ? function (it) {
	  return typeof it == 'symbol';
	} : function (it) {
	  var $Symbol = getBuiltIn$8('Symbol');
	  return isCallable$c($Symbol) && isPrototypeOf$3($Symbol.prototype, $Object$3(it));
	};

	var $String$3 = String;

	var tryToString$4 = function (argument) {
	  try {
	    return $String$3(argument);
	  } catch (error) {
	    return 'Object';
	  }
	};

	var isCallable$b = isCallable$f;
	var tryToString$3 = tryToString$4;

	var $TypeError$7 = TypeError;

	// `Assert: IsCallable(argument) is true`
	var aCallable$4 = function (argument) {
	  if (isCallable$b(argument)) return argument;
	  throw $TypeError$7(tryToString$3(argument) + ' is not a function');
	};

	var aCallable$3 = aCallable$4;
	var isNullOrUndefined$1 = isNullOrUndefined$3;

	// `GetMethod` abstract operation
	// https://tc39.es/ecma262/#sec-getmethod
	var getMethod$3 = function (V, P) {
	  var func = V[P];
	  return isNullOrUndefined$1(func) ? undefined : aCallable$3(func);
	};

	var call$9 = functionCall;
	var isCallable$a = isCallable$f;
	var isObject$7 = isObject$8;

	var $TypeError$6 = TypeError;

	// `OrdinaryToPrimitive` abstract operation
	// https://tc39.es/ecma262/#sec-ordinarytoprimitive
	var ordinaryToPrimitive$1 = function (input, pref) {
	  var fn, val;
	  if (pref === 'string' && isCallable$a(fn = input.toString) && !isObject$7(val = call$9(fn, input))) return val;
	  if (isCallable$a(fn = input.valueOf) && !isObject$7(val = call$9(fn, input))) return val;
	  if (pref !== 'string' && isCallable$a(fn = input.toString) && !isObject$7(val = call$9(fn, input))) return val;
	  throw $TypeError$6("Can't convert object to primitive value");
	};

	var sharedExports = {};
	var shared$7 = {
	  get exports(){ return sharedExports; },
	  set exports(v){ sharedExports = v; },
	};

	var global$b = global$e;

	// eslint-disable-next-line es/no-object-defineproperty -- safe
	var defineProperty$8 = Object.defineProperty;

	var defineGlobalProperty$1 = function (key, value) {
	  try {
	    defineProperty$8(global$b, key, { value: value, configurable: true, writable: true });
	  } catch (error) {
	    global$b[key] = value;
	  } return value;
	};

	var global$a = global$e;
	var defineGlobalProperty = defineGlobalProperty$1;

	var SHARED = '__core-js_shared__';
	var store$3 = global$a[SHARED] || defineGlobalProperty(SHARED, {});

	var sharedStore = store$3;

	var store$2 = sharedStore;

	(shared$7.exports = function (key, value) {
	  return store$2[key] || (store$2[key] = value !== undefined ? value : {});
	})('versions', []).push({
	  version: '3.29.1',
	  mode: 'pure' ,
	  copyright: '© 2014-2023 Denis Pushkarev (zloirock.ru)',
	  license: 'https://github.com/zloirock/core-js/blob/v3.29.1/LICENSE',
	  source: 'https://github.com/zloirock/core-js'
	});

	var requireObjectCoercible$1 = requireObjectCoercible$3;

	var $Object$2 = Object;

	// `ToObject` abstract operation
	// https://tc39.es/ecma262/#sec-toobject
	var toObject$6 = function (argument) {
	  return $Object$2(requireObjectCoercible$1(argument));
	};

	var uncurryThis$e = functionUncurryThis;
	var toObject$5 = toObject$6;

	var hasOwnProperty = uncurryThis$e({}.hasOwnProperty);

	// `HasOwnProperty` abstract operation
	// https://tc39.es/ecma262/#sec-hasownproperty
	// eslint-disable-next-line es/no-object-hasown -- safe
	var hasOwnProperty_1 = Object.hasOwn || function hasOwn(it, key) {
	  return hasOwnProperty(toObject$5(it), key);
	};

	var uncurryThis$d = functionUncurryThis;

	var id = 0;
	var postfix = Math.random();
	var toString$6 = uncurryThis$d(1.0.toString);

	var uid$3 = function (key) {
	  return 'Symbol(' + (key === undefined ? '' : key) + ')_' + toString$6(++id + postfix, 36);
	};

	var global$9 = global$e;
	var shared$6 = sharedExports;
	var hasOwn$b = hasOwnProperty_1;
	var uid$2 = uid$3;
	var NATIVE_SYMBOL$4 = symbolConstructorDetection;
	var USE_SYMBOL_AS_UID = useSymbolAsUid;

	var Symbol$3 = global$9.Symbol;
	var WellKnownSymbolsStore$2 = shared$6('wks');
	var createWellKnownSymbol = USE_SYMBOL_AS_UID ? Symbol$3['for'] || Symbol$3 : Symbol$3 && Symbol$3.withoutSetter || uid$2;

	var wellKnownSymbol$g = function (name) {
	  if (!hasOwn$b(WellKnownSymbolsStore$2, name)) {
	    WellKnownSymbolsStore$2[name] = NATIVE_SYMBOL$4 && hasOwn$b(Symbol$3, name)
	      ? Symbol$3[name]
	      : createWellKnownSymbol('Symbol.' + name);
	  } return WellKnownSymbolsStore$2[name];
	};

	var call$8 = functionCall;
	var isObject$6 = isObject$8;
	var isSymbol$4 = isSymbol$5;
	var getMethod$2 = getMethod$3;
	var ordinaryToPrimitive = ordinaryToPrimitive$1;
	var wellKnownSymbol$f = wellKnownSymbol$g;

	var $TypeError$5 = TypeError;
	var TO_PRIMITIVE = wellKnownSymbol$f('toPrimitive');

	// `ToPrimitive` abstract operation
	// https://tc39.es/ecma262/#sec-toprimitive
	var toPrimitive$8 = function (input, pref) {
	  if (!isObject$6(input) || isSymbol$4(input)) return input;
	  var exoticToPrim = getMethod$2(input, TO_PRIMITIVE);
	  var result;
	  if (exoticToPrim) {
	    if (pref === undefined) pref = 'default';
	    result = call$8(exoticToPrim, input, pref);
	    if (!isObject$6(result) || isSymbol$4(result)) return result;
	    throw $TypeError$5("Can't convert object to primitive value");
	  }
	  if (pref === undefined) pref = 'number';
	  return ordinaryToPrimitive(input, pref);
	};

	var toPrimitive$7 = toPrimitive$8;
	var isSymbol$3 = isSymbol$5;

	// `ToPropertyKey` abstract operation
	// https://tc39.es/ecma262/#sec-topropertykey
	var toPropertyKey$5 = function (argument) {
	  var key = toPrimitive$7(argument, 'string');
	  return isSymbol$3(key) ? key : key + '';
	};

	var global$8 = global$e;
	var isObject$5 = isObject$8;

	var document$1 = global$8.document;
	// typeof document.createElement is 'object' in old IE
	var EXISTS$1 = isObject$5(document$1) && isObject$5(document$1.createElement);

	var documentCreateElement$1 = function (it) {
	  return EXISTS$1 ? document$1.createElement(it) : {};
	};

	var DESCRIPTORS$8 = descriptors;
	var fails$a = fails$f;
	var createElement = documentCreateElement$1;

	// Thanks to IE8 for its funny defineProperty
	var ie8DomDefine = !DESCRIPTORS$8 && !fails$a(function () {
	  // eslint-disable-next-line es/no-object-defineproperty -- required for testing
	  return Object.defineProperty(createElement('div'), 'a', {
	    get: function () { return 7; }
	  }).a != 7;
	});

	var DESCRIPTORS$7 = descriptors;
	var call$7 = functionCall;
	var propertyIsEnumerableModule$1 = objectPropertyIsEnumerable;
	var createPropertyDescriptor$4 = createPropertyDescriptor$5;
	var toIndexedObject$6 = toIndexedObject$7;
	var toPropertyKey$4 = toPropertyKey$5;
	var hasOwn$a = hasOwnProperty_1;
	var IE8_DOM_DEFINE$1 = ie8DomDefine;

	// eslint-disable-next-line es/no-object-getownpropertydescriptor -- safe
	var $getOwnPropertyDescriptor$2 = Object.getOwnPropertyDescriptor;

	// `Object.getOwnPropertyDescriptor` method
	// https://tc39.es/ecma262/#sec-object.getownpropertydescriptor
	objectGetOwnPropertyDescriptor.f = DESCRIPTORS$7 ? $getOwnPropertyDescriptor$2 : function getOwnPropertyDescriptor(O, P) {
	  O = toIndexedObject$6(O);
	  P = toPropertyKey$4(P);
	  if (IE8_DOM_DEFINE$1) try {
	    return $getOwnPropertyDescriptor$2(O, P);
	  } catch (error) { /* empty */ }
	  if (hasOwn$a(O, P)) return createPropertyDescriptor$4(!call$7(propertyIsEnumerableModule$1.f, O, P), O[P]);
	};

	var fails$9 = fails$f;
	var isCallable$9 = isCallable$f;

	var replacement = /#|\.prototype\./;

	var isForced$1 = function (feature, detection) {
	  var value = data[normalize(feature)];
	  return value == POLYFILL ? true
	    : value == NATIVE ? false
	    : isCallable$9(detection) ? fails$9(detection)
	    : !!detection;
	};

	var normalize = isForced$1.normalize = function (string) {
	  return String(string).replace(replacement, '.').toLowerCase();
	};

	var data = isForced$1.data = {};
	var NATIVE = isForced$1.NATIVE = 'N';
	var POLYFILL = isForced$1.POLYFILL = 'P';

	var isForced_1 = isForced$1;

	var uncurryThis$c = functionUncurryThisClause;
	var aCallable$2 = aCallable$4;
	var NATIVE_BIND = functionBindNative;

	var bind$4 = uncurryThis$c(uncurryThis$c.bind);

	// optional / simple context binding
	var functionBindContext = function (fn, that) {
	  aCallable$2(fn);
	  return that === undefined ? fn : NATIVE_BIND ? bind$4(fn, that) : function (/* ...args */) {
	    return fn.apply(that, arguments);
	  };
	};

	var objectDefineProperty = {};

	var DESCRIPTORS$6 = descriptors;
	var fails$8 = fails$f;

	// V8 ~ Chrome 36-
	// https://bugs.chromium.org/p/v8/issues/detail?id=3334
	var v8PrototypeDefineBug = DESCRIPTORS$6 && fails$8(function () {
	  // eslint-disable-next-line es/no-object-defineproperty -- required for testing
	  return Object.defineProperty(function () { /* empty */ }, 'prototype', {
	    value: 42,
	    writable: false
	  }).prototype != 42;
	});

	var isObject$4 = isObject$8;

	var $String$2 = String;
	var $TypeError$4 = TypeError;

	// `Assert: Type(argument) is Object`
	var anObject$7 = function (argument) {
	  if (isObject$4(argument)) return argument;
	  throw $TypeError$4($String$2(argument) + ' is not an object');
	};

	var DESCRIPTORS$5 = descriptors;
	var IE8_DOM_DEFINE = ie8DomDefine;
	var V8_PROTOTYPE_DEFINE_BUG$1 = v8PrototypeDefineBug;
	var anObject$6 = anObject$7;
	var toPropertyKey$3 = toPropertyKey$5;

	var $TypeError$3 = TypeError;
	// eslint-disable-next-line es/no-object-defineproperty -- safe
	var $defineProperty$1 = Object.defineProperty;
	// eslint-disable-next-line es/no-object-getownpropertydescriptor -- safe
	var $getOwnPropertyDescriptor$1 = Object.getOwnPropertyDescriptor;
	var ENUMERABLE = 'enumerable';
	var CONFIGURABLE$1 = 'configurable';
	var WRITABLE = 'writable';

	// `Object.defineProperty` method
	// https://tc39.es/ecma262/#sec-object.defineproperty
	objectDefineProperty.f = DESCRIPTORS$5 ? V8_PROTOTYPE_DEFINE_BUG$1 ? function defineProperty(O, P, Attributes) {
	  anObject$6(O);
	  P = toPropertyKey$3(P);
	  anObject$6(Attributes);
	  if (typeof O === 'function' && P === 'prototype' && 'value' in Attributes && WRITABLE in Attributes && !Attributes[WRITABLE]) {
	    var current = $getOwnPropertyDescriptor$1(O, P);
	    if (current && current[WRITABLE]) {
	      O[P] = Attributes.value;
	      Attributes = {
	        configurable: CONFIGURABLE$1 in Attributes ? Attributes[CONFIGURABLE$1] : current[CONFIGURABLE$1],
	        enumerable: ENUMERABLE in Attributes ? Attributes[ENUMERABLE] : current[ENUMERABLE],
	        writable: false
	      };
	    }
	  } return $defineProperty$1(O, P, Attributes);
	} : $defineProperty$1 : function defineProperty(O, P, Attributes) {
	  anObject$6(O);
	  P = toPropertyKey$3(P);
	  anObject$6(Attributes);
	  if (IE8_DOM_DEFINE) try {
	    return $defineProperty$1(O, P, Attributes);
	  } catch (error) { /* empty */ }
	  if ('get' in Attributes || 'set' in Attributes) throw $TypeError$3('Accessors not supported');
	  if ('value' in Attributes) O[P] = Attributes.value;
	  return O;
	};

	var DESCRIPTORS$4 = descriptors;
	var definePropertyModule$3 = objectDefineProperty;
	var createPropertyDescriptor$3 = createPropertyDescriptor$5;

	var createNonEnumerableProperty$5 = DESCRIPTORS$4 ? function (object, key, value) {
	  return definePropertyModule$3.f(object, key, createPropertyDescriptor$3(1, value));
	} : function (object, key, value) {
	  object[key] = value;
	  return object;
	};

	var global$7 = global$e;
	var apply$1 = functionApply;
	var uncurryThis$b = functionUncurryThisClause;
	var isCallable$8 = isCallable$f;
	var getOwnPropertyDescriptor = objectGetOwnPropertyDescriptor.f;
	var isForced = isForced_1;
	var path$5 = path$7;
	var bind$3 = functionBindContext;
	var createNonEnumerableProperty$4 = createNonEnumerableProperty$5;
	var hasOwn$9 = hasOwnProperty_1;

	var wrapConstructor = function (NativeConstructor) {
	  var Wrapper = function (a, b, c) {
	    if (this instanceof Wrapper) {
	      switch (arguments.length) {
	        case 0: return new NativeConstructor();
	        case 1: return new NativeConstructor(a);
	        case 2: return new NativeConstructor(a, b);
	      } return new NativeConstructor(a, b, c);
	    } return apply$1(NativeConstructor, this, arguments);
	  };
	  Wrapper.prototype = NativeConstructor.prototype;
	  return Wrapper;
	};

	/*
	  options.target         - name of the target object
	  options.global         - target is the global object
	  options.stat           - export as static methods of target
	  options.proto          - export as prototype methods of target
	  options.real           - real prototype method for the `pure` version
	  options.forced         - export even if the native feature is available
	  options.bind           - bind methods to the target, required for the `pure` version
	  options.wrap           - wrap constructors to preventing global pollution, required for the `pure` version
	  options.unsafe         - use the simple assignment of property instead of delete + defineProperty
	  options.sham           - add a flag to not completely full polyfills
	  options.enumerable     - export as enumerable property
	  options.dontCallGetSet - prevent calling a getter on target
	  options.name           - the .name of the function if it does not match the key
	*/
	var _export = function (options, source) {
	  var TARGET = options.target;
	  var GLOBAL = options.global;
	  var STATIC = options.stat;
	  var PROTO = options.proto;

	  var nativeSource = GLOBAL ? global$7 : STATIC ? global$7[TARGET] : (global$7[TARGET] || {}).prototype;

	  var target = GLOBAL ? path$5 : path$5[TARGET] || createNonEnumerableProperty$4(path$5, TARGET, {})[TARGET];
	  var targetPrototype = target.prototype;

	  var FORCED, USE_NATIVE, VIRTUAL_PROTOTYPE;
	  var key, sourceProperty, targetProperty, nativeProperty, resultProperty, descriptor;

	  for (key in source) {
	    FORCED = isForced(GLOBAL ? key : TARGET + (STATIC ? '.' : '#') + key, options.forced);
	    // contains in native
	    USE_NATIVE = !FORCED && nativeSource && hasOwn$9(nativeSource, key);

	    targetProperty = target[key];

	    if (USE_NATIVE) if (options.dontCallGetSet) {
	      descriptor = getOwnPropertyDescriptor(nativeSource, key);
	      nativeProperty = descriptor && descriptor.value;
	    } else nativeProperty = nativeSource[key];

	    // export native or implementation
	    sourceProperty = (USE_NATIVE && nativeProperty) ? nativeProperty : source[key];

	    if (USE_NATIVE && typeof targetProperty == typeof sourceProperty) continue;

	    // bind methods to global for calling from export context
	    if (options.bind && USE_NATIVE) resultProperty = bind$3(sourceProperty, global$7);
	    // wrap global constructors for prevent changes in this version
	    else if (options.wrap && USE_NATIVE) resultProperty = wrapConstructor(sourceProperty);
	    // make static versions for prototype methods
	    else if (PROTO && isCallable$8(sourceProperty)) resultProperty = uncurryThis$b(sourceProperty);
	    // default case
	    else resultProperty = sourceProperty;

	    // add a flag to not completely full polyfills
	    if (options.sham || (sourceProperty && sourceProperty.sham) || (targetProperty && targetProperty.sham)) {
	      createNonEnumerableProperty$4(resultProperty, 'sham', true);
	    }

	    createNonEnumerableProperty$4(target, key, resultProperty);

	    if (PROTO) {
	      VIRTUAL_PROTOTYPE = TARGET + 'Prototype';
	      if (!hasOwn$9(path$5, VIRTUAL_PROTOTYPE)) {
	        createNonEnumerableProperty$4(path$5, VIRTUAL_PROTOTYPE, {});
	      }
	      // export virtual prototype methods
	      createNonEnumerableProperty$4(path$5[VIRTUAL_PROTOTYPE], key, sourceProperty);
	      // export real prototype methods
	      if (options.real && targetPrototype && (FORCED || !targetPrototype[key])) {
	        createNonEnumerableProperty$4(targetPrototype, key, sourceProperty);
	      }
	    }
	  }
	};

	var $$b = _export;
	var DESCRIPTORS$3 = descriptors;
	var defineProperty$7 = objectDefineProperty.f;

	// `Object.defineProperty` method
	// https://tc39.es/ecma262/#sec-object.defineproperty
	// eslint-disable-next-line es/no-object-defineproperty -- safe
	$$b({ target: 'Object', stat: true, forced: Object.defineProperty !== defineProperty$7, sham: !DESCRIPTORS$3 }, {
	  defineProperty: defineProperty$7
	});

	var path$4 = path$7;

	var Object$1 = path$4.Object;

	var defineProperty$6 = defineProperty$9.exports = function defineProperty(it, key, desc) {
	  return Object$1.defineProperty(it, key, desc);
	};

	if (Object$1.defineProperty.sham) defineProperty$6.sham = true;

	var parent$d = definePropertyExports;

	var defineProperty$5 = parent$d;

	var parent$c = defineProperty$5;

	var defineProperty$4 = parent$c;

	var parent$b = defineProperty$4;

	var defineProperty$3 = parent$b;

	(function (module) {
		module.exports = defineProperty$3;
	} (defineProperty$a));

	(function (module) {
		module.exports = definePropertyExports$1;
	} (defineProperty$b));

	var toPropertyKeyExports = {};
	var toPropertyKey$2 = {
	  get exports(){ return toPropertyKeyExports; },
	  set exports(v){ toPropertyKeyExports = v; },
	};

	var _typeofExports = {};
	var _typeof = {
	  get exports(){ return _typeofExports; },
	  set exports(v){ _typeofExports = v; },
	};

	var symbolExports$1 = {};
	var symbol$5 = {
	  get exports(){ return symbolExports$1; },
	  set exports(v){ symbolExports$1 = v; },
	};

	var symbolExports = {};
	var symbol$4 = {
	  get exports(){ return symbolExports; },
	  set exports(v){ symbolExports = v; },
	};

	var classof$8 = classofRaw$2;

	// `IsArray` abstract operation
	// https://tc39.es/ecma262/#sec-isarray
	// eslint-disable-next-line es/no-array-isarray -- safe
	var isArray$4 = Array.isArray || function isArray(argument) {
	  return classof$8(argument) == 'Array';
	};

	var ceil = Math.ceil;
	var floor = Math.floor;

	// `Math.trunc` method
	// https://tc39.es/ecma262/#sec-math.trunc
	// eslint-disable-next-line es/no-math-trunc -- safe
	var mathTrunc = Math.trunc || function trunc(x) {
	  var n = +x;
	  return (n > 0 ? floor : ceil)(n);
	};

	var trunc = mathTrunc;

	// `ToIntegerOrInfinity` abstract operation
	// https://tc39.es/ecma262/#sec-tointegerorinfinity
	var toIntegerOrInfinity$3 = function (argument) {
	  var number = +argument;
	  // eslint-disable-next-line no-self-compare -- NaN check
	  return number !== number || number === 0 ? 0 : trunc(number);
	};

	var toIntegerOrInfinity$2 = toIntegerOrInfinity$3;

	var min$1 = Math.min;

	// `ToLength` abstract operation
	// https://tc39.es/ecma262/#sec-tolength
	var toLength$1 = function (argument) {
	  return argument > 0 ? min$1(toIntegerOrInfinity$2(argument), 0x1FFFFFFFFFFFFF) : 0; // 2 ** 53 - 1 == 9007199254740991
	};

	var toLength = toLength$1;

	// `LengthOfArrayLike` abstract operation
	// https://tc39.es/ecma262/#sec-lengthofarraylike
	var lengthOfArrayLike$7 = function (obj) {
	  return toLength(obj.length);
	};

	var $TypeError$2 = TypeError;
	var MAX_SAFE_INTEGER = 0x1FFFFFFFFFFFFF; // 2 ** 53 - 1 == 9007199254740991

	var doesNotExceedSafeInteger$2 = function (it) {
	  if (it > MAX_SAFE_INTEGER) throw $TypeError$2('Maximum allowed index exceeded');
	  return it;
	};

	var toPropertyKey$1 = toPropertyKey$5;
	var definePropertyModule$2 = objectDefineProperty;
	var createPropertyDescriptor$2 = createPropertyDescriptor$5;

	var createProperty$3 = function (object, key, value) {
	  var propertyKey = toPropertyKey$1(key);
	  if (propertyKey in object) definePropertyModule$2.f(object, propertyKey, createPropertyDescriptor$2(0, value));
	  else object[propertyKey] = value;
	};

	var wellKnownSymbol$e = wellKnownSymbol$g;

	var TO_STRING_TAG$3 = wellKnownSymbol$e('toStringTag');
	var test = {};

	test[TO_STRING_TAG$3] = 'z';

	var toStringTagSupport = String(test) === '[object z]';

	var TO_STRING_TAG_SUPPORT$2 = toStringTagSupport;
	var isCallable$7 = isCallable$f;
	var classofRaw = classofRaw$2;
	var wellKnownSymbol$d = wellKnownSymbol$g;

	var TO_STRING_TAG$2 = wellKnownSymbol$d('toStringTag');
	var $Object$1 = Object;

	// ES3 wrong here
	var CORRECT_ARGUMENTS = classofRaw(function () { return arguments; }()) == 'Arguments';

	// fallback for IE11 Script Access Denied error
	var tryGet = function (it, key) {
	  try {
	    return it[key];
	  } catch (error) { /* empty */ }
	};

	// getting tag from ES6+ `Object.prototype.toString`
	var classof$7 = TO_STRING_TAG_SUPPORT$2 ? classofRaw : function (it) {
	  var O, tag, result;
	  return it === undefined ? 'Undefined' : it === null ? 'Null'
	    // @@toStringTag case
	    : typeof (tag = tryGet(O = $Object$1(it), TO_STRING_TAG$2)) == 'string' ? tag
	    // builtinTag case
	    : CORRECT_ARGUMENTS ? classofRaw(O)
	    // ES3 arguments fallback
	    : (result = classofRaw(O)) == 'Object' && isCallable$7(O.callee) ? 'Arguments' : result;
	};

	var uncurryThis$a = functionUncurryThis;
	var isCallable$6 = isCallable$f;
	var store$1 = sharedStore;

	var functionToString = uncurryThis$a(Function.toString);

	// this helper broken in `core-js@3.4.1-3.4.4`, so we can't use `shared` helper
	if (!isCallable$6(store$1.inspectSource)) {
	  store$1.inspectSource = function (it) {
	    return functionToString(it);
	  };
	}

	var inspectSource$1 = store$1.inspectSource;

	var uncurryThis$9 = functionUncurryThis;
	var fails$7 = fails$f;
	var isCallable$5 = isCallable$f;
	var classof$6 = classof$7;
	var getBuiltIn$7 = getBuiltIn$9;
	var inspectSource = inspectSource$1;

	var noop = function () { /* empty */ };
	var empty = [];
	var construct = getBuiltIn$7('Reflect', 'construct');
	var constructorRegExp = /^\s*(?:class|function)\b/;
	var exec$1 = uncurryThis$9(constructorRegExp.exec);
	var INCORRECT_TO_STRING = !constructorRegExp.exec(noop);

	var isConstructorModern = function isConstructor(argument) {
	  if (!isCallable$5(argument)) return false;
	  try {
	    construct(noop, empty, argument);
	    return true;
	  } catch (error) {
	    return false;
	  }
	};

	var isConstructorLegacy = function isConstructor(argument) {
	  if (!isCallable$5(argument)) return false;
	  switch (classof$6(argument)) {
	    case 'AsyncFunction':
	    case 'GeneratorFunction':
	    case 'AsyncGeneratorFunction': return false;
	  }
	  try {
	    // we can't check .prototype since constructors produced by .bind haven't it
	    // `Function#toString` throws on some built-it function in some legacy engines
	    // (for example, `DOMQuad` and similar in FF41-)
	    return INCORRECT_TO_STRING || !!exec$1(constructorRegExp, inspectSource(argument));
	  } catch (error) {
	    return true;
	  }
	};

	isConstructorLegacy.sham = true;

	// `IsConstructor` abstract operation
	// https://tc39.es/ecma262/#sec-isconstructor
	var isConstructor$1 = !construct || fails$7(function () {
	  var called;
	  return isConstructorModern(isConstructorModern.call)
	    || !isConstructorModern(Object)
	    || !isConstructorModern(function () { called = true; })
	    || called;
	}) ? isConstructorLegacy : isConstructorModern;

	var isArray$3 = isArray$4;
	var isConstructor = isConstructor$1;
	var isObject$3 = isObject$8;
	var wellKnownSymbol$c = wellKnownSymbol$g;

	var SPECIES$1 = wellKnownSymbol$c('species');
	var $Array$1 = Array;

	// a part of `ArraySpeciesCreate` abstract operation
	// https://tc39.es/ecma262/#sec-arrayspeciescreate
	var arraySpeciesConstructor$1 = function (originalArray) {
	  var C;
	  if (isArray$3(originalArray)) {
	    C = originalArray.constructor;
	    // cross-realm fallback
	    if (isConstructor(C) && (C === $Array$1 || isArray$3(C.prototype))) C = undefined;
	    else if (isObject$3(C)) {
	      C = C[SPECIES$1];
	      if (C === null) C = undefined;
	    }
	  } return C === undefined ? $Array$1 : C;
	};

	var arraySpeciesConstructor = arraySpeciesConstructor$1;

	// `ArraySpeciesCreate` abstract operation
	// https://tc39.es/ecma262/#sec-arrayspeciescreate
	var arraySpeciesCreate$3 = function (originalArray, length) {
	  return new (arraySpeciesConstructor(originalArray))(length === 0 ? 0 : length);
	};

	var fails$6 = fails$f;
	var wellKnownSymbol$b = wellKnownSymbol$g;
	var V8_VERSION$1 = engineV8Version;

	var SPECIES = wellKnownSymbol$b('species');

	var arrayMethodHasSpeciesSupport$1 = function (METHOD_NAME) {
	  // We can't use this feature detection in V8 since it causes
	  // deoptimization and serious performance degradation
	  // https://github.com/zloirock/core-js/issues/677
	  return V8_VERSION$1 >= 51 || !fails$6(function () {
	    var array = [];
	    var constructor = array.constructor = {};
	    constructor[SPECIES] = function () {
	      return { foo: 1 };
	    };
	    return array[METHOD_NAME](Boolean).foo !== 1;
	  });
	};

	var $$a = _export;
	var fails$5 = fails$f;
	var isArray$2 = isArray$4;
	var isObject$2 = isObject$8;
	var toObject$4 = toObject$6;
	var lengthOfArrayLike$6 = lengthOfArrayLike$7;
	var doesNotExceedSafeInteger$1 = doesNotExceedSafeInteger$2;
	var createProperty$2 = createProperty$3;
	var arraySpeciesCreate$2 = arraySpeciesCreate$3;
	var arrayMethodHasSpeciesSupport = arrayMethodHasSpeciesSupport$1;
	var wellKnownSymbol$a = wellKnownSymbol$g;
	var V8_VERSION = engineV8Version;

	var IS_CONCAT_SPREADABLE = wellKnownSymbol$a('isConcatSpreadable');

	// We can't use this feature detection in V8 since it causes
	// deoptimization and serious performance degradation
	// https://github.com/zloirock/core-js/issues/679
	var IS_CONCAT_SPREADABLE_SUPPORT = V8_VERSION >= 51 || !fails$5(function () {
	  var array = [];
	  array[IS_CONCAT_SPREADABLE] = false;
	  return array.concat()[0] !== array;
	});

	var isConcatSpreadable = function (O) {
	  if (!isObject$2(O)) return false;
	  var spreadable = O[IS_CONCAT_SPREADABLE];
	  return spreadable !== undefined ? !!spreadable : isArray$2(O);
	};

	var FORCED$1 = !IS_CONCAT_SPREADABLE_SUPPORT || !arrayMethodHasSpeciesSupport('concat');

	// `Array.prototype.concat` method
	// https://tc39.es/ecma262/#sec-array.prototype.concat
	// with adding support of @@isConcatSpreadable and @@species
	$$a({ target: 'Array', proto: true, arity: 1, forced: FORCED$1 }, {
	  // eslint-disable-next-line no-unused-vars -- required for `.length`
	  concat: function concat(arg) {
	    var O = toObject$4(this);
	    var A = arraySpeciesCreate$2(O, 0);
	    var n = 0;
	    var i, k, length, len, E;
	    for (i = -1, length = arguments.length; i < length; i++) {
	      E = i === -1 ? O : arguments[i];
	      if (isConcatSpreadable(E)) {
	        len = lengthOfArrayLike$6(E);
	        doesNotExceedSafeInteger$1(n + len);
	        for (k = 0; k < len; k++, n++) if (k in E) createProperty$2(A, n, E[k]);
	      } else {
	        doesNotExceedSafeInteger$1(n + 1);
	        createProperty$2(A, n++, E);
	      }
	    }
	    A.length = n;
	    return A;
	  }
	});

	var classof$5 = classof$7;

	var $String$1 = String;

	var toString$5 = function (argument) {
	  if (classof$5(argument) === 'Symbol') throw TypeError('Cannot convert a Symbol value to a string');
	  return $String$1(argument);
	};

	var objectDefineProperties = {};

	var toIntegerOrInfinity$1 = toIntegerOrInfinity$3;

	var max$1 = Math.max;
	var min = Math.min;

	// Helper for a popular repeating case of the spec:
	// Let integer be ? ToInteger(index).
	// If integer < 0, let result be max((length + integer), 0); else let result be min(integer, length).
	var toAbsoluteIndex$2 = function (index, length) {
	  var integer = toIntegerOrInfinity$1(index);
	  return integer < 0 ? max$1(integer + length, 0) : min(integer, length);
	};

	var toIndexedObject$5 = toIndexedObject$7;
	var toAbsoluteIndex$1 = toAbsoluteIndex$2;
	var lengthOfArrayLike$5 = lengthOfArrayLike$7;

	// `Array.prototype.{ indexOf, includes }` methods implementation
	var createMethod$2 = function (IS_INCLUDES) {
	  return function ($this, el, fromIndex) {
	    var O = toIndexedObject$5($this);
	    var length = lengthOfArrayLike$5(O);
	    var index = toAbsoluteIndex$1(fromIndex, length);
	    var value;
	    // Array#includes uses SameValueZero equality algorithm
	    // eslint-disable-next-line no-self-compare -- NaN check
	    if (IS_INCLUDES && el != el) while (length > index) {
	      value = O[index++];
	      // eslint-disable-next-line no-self-compare -- NaN check
	      if (value != value) return true;
	    // Array#indexOf ignores holes, Array#includes - not
	    } else for (;length > index; index++) {
	      if ((IS_INCLUDES || index in O) && O[index] === el) return IS_INCLUDES || index || 0;
	    } return !IS_INCLUDES && -1;
	  };
	};

	var arrayIncludes = {
	  // `Array.prototype.includes` method
	  // https://tc39.es/ecma262/#sec-array.prototype.includes
	  includes: createMethod$2(true),
	  // `Array.prototype.indexOf` method
	  // https://tc39.es/ecma262/#sec-array.prototype.indexof
	  indexOf: createMethod$2(false)
	};

	var hiddenKeys$5 = {};

	var uncurryThis$8 = functionUncurryThis;
	var hasOwn$8 = hasOwnProperty_1;
	var toIndexedObject$4 = toIndexedObject$7;
	var indexOf = arrayIncludes.indexOf;
	var hiddenKeys$4 = hiddenKeys$5;

	var push$3 = uncurryThis$8([].push);

	var objectKeysInternal = function (object, names) {
	  var O = toIndexedObject$4(object);
	  var i = 0;
	  var result = [];
	  var key;
	  for (key in O) !hasOwn$8(hiddenKeys$4, key) && hasOwn$8(O, key) && push$3(result, key);
	  // Don't enum bug & hidden keys
	  while (names.length > i) if (hasOwn$8(O, key = names[i++])) {
	    ~indexOf(result, key) || push$3(result, key);
	  }
	  return result;
	};

	// IE8- don't enum bug keys
	var enumBugKeys$3 = [
	  'constructor',
	  'hasOwnProperty',
	  'isPrototypeOf',
	  'propertyIsEnumerable',
	  'toLocaleString',
	  'toString',
	  'valueOf'
	];

	var internalObjectKeys$1 = objectKeysInternal;
	var enumBugKeys$2 = enumBugKeys$3;

	// `Object.keys` method
	// https://tc39.es/ecma262/#sec-object.keys
	// eslint-disable-next-line es/no-object-keys -- safe
	var objectKeys$2 = Object.keys || function keys(O) {
	  return internalObjectKeys$1(O, enumBugKeys$2);
	};

	var DESCRIPTORS$2 = descriptors;
	var V8_PROTOTYPE_DEFINE_BUG = v8PrototypeDefineBug;
	var definePropertyModule$1 = objectDefineProperty;
	var anObject$5 = anObject$7;
	var toIndexedObject$3 = toIndexedObject$7;
	var objectKeys$1 = objectKeys$2;

	// `Object.defineProperties` method
	// https://tc39.es/ecma262/#sec-object.defineproperties
	// eslint-disable-next-line es/no-object-defineproperties -- safe
	objectDefineProperties.f = DESCRIPTORS$2 && !V8_PROTOTYPE_DEFINE_BUG ? Object.defineProperties : function defineProperties(O, Properties) {
	  anObject$5(O);
	  var props = toIndexedObject$3(Properties);
	  var keys = objectKeys$1(Properties);
	  var length = keys.length;
	  var index = 0;
	  var key;
	  while (length > index) definePropertyModule$1.f(O, key = keys[index++], props[key]);
	  return O;
	};

	var getBuiltIn$6 = getBuiltIn$9;

	var html$1 = getBuiltIn$6('document', 'documentElement');

	var shared$5 = sharedExports;
	var uid$1 = uid$3;

	var keys = shared$5('keys');

	var sharedKey$4 = function (key) {
	  return keys[key] || (keys[key] = uid$1(key));
	};

	/* global ActiveXObject -- old IE, WSH */

	var anObject$4 = anObject$7;
	var definePropertiesModule$1 = objectDefineProperties;
	var enumBugKeys$1 = enumBugKeys$3;
	var hiddenKeys$3 = hiddenKeys$5;
	var html = html$1;
	var documentCreateElement = documentCreateElement$1;
	var sharedKey$3 = sharedKey$4;

	var GT = '>';
	var LT = '<';
	var PROTOTYPE$1 = 'prototype';
	var SCRIPT = 'script';
	var IE_PROTO$1 = sharedKey$3('IE_PROTO');

	var EmptyConstructor = function () { /* empty */ };

	var scriptTag = function (content) {
	  return LT + SCRIPT + GT + content + LT + '/' + SCRIPT + GT;
	};

	// Create object with fake `null` prototype: use ActiveX Object with cleared prototype
	var NullProtoObjectViaActiveX = function (activeXDocument) {
	  activeXDocument.write(scriptTag(''));
	  activeXDocument.close();
	  var temp = activeXDocument.parentWindow.Object;
	  activeXDocument = null; // avoid memory leak
	  return temp;
	};

	// Create object with fake `null` prototype: use iframe Object with cleared prototype
	var NullProtoObjectViaIFrame = function () {
	  // Thrash, waste and sodomy: IE GC bug
	  var iframe = documentCreateElement('iframe');
	  var JS = 'java' + SCRIPT + ':';
	  var iframeDocument;
	  iframe.style.display = 'none';
	  html.appendChild(iframe);
	  // https://github.com/zloirock/core-js/issues/475
	  iframe.src = String(JS);
	  iframeDocument = iframe.contentWindow.document;
	  iframeDocument.open();
	  iframeDocument.write(scriptTag('document.F=Object'));
	  iframeDocument.close();
	  return iframeDocument.F;
	};

	// Check for document.domain and active x support
	// No need to use active x approach when document.domain is not set
	// see https://github.com/es-shims/es5-shim/issues/150
	// variation of https://github.com/kitcambridge/es5-shim/commit/4f738ac066346
	// avoid IE GC bug
	var activeXDocument;
	var NullProtoObject = function () {
	  try {
	    activeXDocument = new ActiveXObject('htmlfile');
	  } catch (error) { /* ignore */ }
	  NullProtoObject = typeof document != 'undefined'
	    ? document.domain && activeXDocument
	      ? NullProtoObjectViaActiveX(activeXDocument) // old IE
	      : NullProtoObjectViaIFrame()
	    : NullProtoObjectViaActiveX(activeXDocument); // WSH
	  var length = enumBugKeys$1.length;
	  while (length--) delete NullProtoObject[PROTOTYPE$1][enumBugKeys$1[length]];
	  return NullProtoObject();
	};

	hiddenKeys$3[IE_PROTO$1] = true;

	// `Object.create` method
	// https://tc39.es/ecma262/#sec-object.create
	// eslint-disable-next-line es/no-object-create -- safe
	var objectCreate = Object.create || function create(O, Properties) {
	  var result;
	  if (O !== null) {
	    EmptyConstructor[PROTOTYPE$1] = anObject$4(O);
	    result = new EmptyConstructor();
	    EmptyConstructor[PROTOTYPE$1] = null;
	    // add "__proto__" for Object.getPrototypeOf polyfill
	    result[IE_PROTO$1] = O;
	  } else result = NullProtoObject();
	  return Properties === undefined ? result : definePropertiesModule$1.f(result, Properties);
	};

	var objectGetOwnPropertyNames = {};

	var internalObjectKeys = objectKeysInternal;
	var enumBugKeys = enumBugKeys$3;

	var hiddenKeys$2 = enumBugKeys.concat('length', 'prototype');

	// `Object.getOwnPropertyNames` method
	// https://tc39.es/ecma262/#sec-object.getownpropertynames
	// eslint-disable-next-line es/no-object-getownpropertynames -- safe
	objectGetOwnPropertyNames.f = Object.getOwnPropertyNames || function getOwnPropertyNames(O) {
	  return internalObjectKeys(O, hiddenKeys$2);
	};

	var objectGetOwnPropertyNamesExternal = {};

	var toAbsoluteIndex = toAbsoluteIndex$2;
	var lengthOfArrayLike$4 = lengthOfArrayLike$7;
	var createProperty$1 = createProperty$3;

	var $Array = Array;
	var max = Math.max;

	var arraySliceSimple = function (O, start, end) {
	  var length = lengthOfArrayLike$4(O);
	  var k = toAbsoluteIndex(start, length);
	  var fin = toAbsoluteIndex(end === undefined ? length : end, length);
	  var result = $Array(max(fin - k, 0));
	  for (var n = 0; k < fin; k++, n++) createProperty$1(result, n, O[k]);
	  result.length = n;
	  return result;
	};

	/* eslint-disable es/no-object-getownpropertynames -- safe */

	var classof$4 = classofRaw$2;
	var toIndexedObject$2 = toIndexedObject$7;
	var $getOwnPropertyNames$1 = objectGetOwnPropertyNames.f;
	var arraySlice$2 = arraySliceSimple;

	var windowNames = typeof window == 'object' && window && Object.getOwnPropertyNames
	  ? Object.getOwnPropertyNames(window) : [];

	var getWindowNames = function (it) {
	  try {
	    return $getOwnPropertyNames$1(it);
	  } catch (error) {
	    return arraySlice$2(windowNames);
	  }
	};

	// fallback for IE11 buggy Object.getOwnPropertyNames with iframe and window
	objectGetOwnPropertyNamesExternal.f = function getOwnPropertyNames(it) {
	  return windowNames && classof$4(it) == 'Window'
	    ? getWindowNames(it)
	    : $getOwnPropertyNames$1(toIndexedObject$2(it));
	};

	var objectGetOwnPropertySymbols = {};

	// eslint-disable-next-line es/no-object-getownpropertysymbols -- safe
	objectGetOwnPropertySymbols.f = Object.getOwnPropertySymbols;

	var createNonEnumerableProperty$3 = createNonEnumerableProperty$5;

	var defineBuiltIn$4 = function (target, key, value, options) {
	  if (options && options.enumerable) target[key] = value;
	  else createNonEnumerableProperty$3(target, key, value);
	  return target;
	};

	var defineProperty$2 = objectDefineProperty;

	var defineBuiltInAccessor$1 = function (target, name, descriptor) {
	  return defineProperty$2.f(target, name, descriptor);
	};

	var wellKnownSymbolWrapped = {};

	var wellKnownSymbol$9 = wellKnownSymbol$g;

	wellKnownSymbolWrapped.f = wellKnownSymbol$9;

	var path$3 = path$7;
	var hasOwn$7 = hasOwnProperty_1;
	var wrappedWellKnownSymbolModule$1 = wellKnownSymbolWrapped;
	var defineProperty$1 = objectDefineProperty.f;

	var wellKnownSymbolDefine = function (NAME) {
	  var Symbol = path$3.Symbol || (path$3.Symbol = {});
	  if (!hasOwn$7(Symbol, NAME)) defineProperty$1(Symbol, NAME, {
	    value: wrappedWellKnownSymbolModule$1.f(NAME)
	  });
	};

	var call$6 = functionCall;
	var getBuiltIn$5 = getBuiltIn$9;
	var wellKnownSymbol$8 = wellKnownSymbol$g;
	var defineBuiltIn$3 = defineBuiltIn$4;

	var symbolDefineToPrimitive = function () {
	  var Symbol = getBuiltIn$5('Symbol');
	  var SymbolPrototype = Symbol && Symbol.prototype;
	  var valueOf = SymbolPrototype && SymbolPrototype.valueOf;
	  var TO_PRIMITIVE = wellKnownSymbol$8('toPrimitive');

	  if (SymbolPrototype && !SymbolPrototype[TO_PRIMITIVE]) {
	    // `Symbol.prototype[@@toPrimitive]` method
	    // https://tc39.es/ecma262/#sec-symbol.prototype-@@toprimitive
	    // eslint-disable-next-line no-unused-vars -- required for .length
	    defineBuiltIn$3(SymbolPrototype, TO_PRIMITIVE, function (hint) {
	      return call$6(valueOf, this);
	    }, { arity: 1 });
	  }
	};

	var TO_STRING_TAG_SUPPORT$1 = toStringTagSupport;
	var classof$3 = classof$7;

	// `Object.prototype.toString` method implementation
	// https://tc39.es/ecma262/#sec-object.prototype.tostring
	var objectToString = TO_STRING_TAG_SUPPORT$1 ? {}.toString : function toString() {
	  return '[object ' + classof$3(this) + ']';
	};

	var TO_STRING_TAG_SUPPORT = toStringTagSupport;
	var defineProperty = objectDefineProperty.f;
	var createNonEnumerableProperty$2 = createNonEnumerableProperty$5;
	var hasOwn$6 = hasOwnProperty_1;
	var toString$4 = objectToString;
	var wellKnownSymbol$7 = wellKnownSymbol$g;

	var TO_STRING_TAG$1 = wellKnownSymbol$7('toStringTag');

	var setToStringTag$5 = function (it, TAG, STATIC, SET_METHOD) {
	  if (it) {
	    var target = STATIC ? it : it.prototype;
	    if (!hasOwn$6(target, TO_STRING_TAG$1)) {
	      defineProperty(target, TO_STRING_TAG$1, { configurable: true, value: TAG });
	    }
	    if (SET_METHOD && !TO_STRING_TAG_SUPPORT) {
	      createNonEnumerableProperty$2(target, 'toString', toString$4);
	    }
	  }
	};

	var global$6 = global$e;
	var isCallable$4 = isCallable$f;

	var WeakMap$1 = global$6.WeakMap;

	var weakMapBasicDetection = isCallable$4(WeakMap$1) && /native code/.test(String(WeakMap$1));

	var NATIVE_WEAK_MAP = weakMapBasicDetection;
	var global$5 = global$e;
	var isObject$1 = isObject$8;
	var createNonEnumerableProperty$1 = createNonEnumerableProperty$5;
	var hasOwn$5 = hasOwnProperty_1;
	var shared$4 = sharedStore;
	var sharedKey$2 = sharedKey$4;
	var hiddenKeys$1 = hiddenKeys$5;

	var OBJECT_ALREADY_INITIALIZED = 'Object already initialized';
	var TypeError$2 = global$5.TypeError;
	var WeakMap = global$5.WeakMap;
	var set, get, has;

	var enforce = function (it) {
	  return has(it) ? get(it) : set(it, {});
	};

	var getterFor = function (TYPE) {
	  return function (it) {
	    var state;
	    if (!isObject$1(it) || (state = get(it)).type !== TYPE) {
	      throw TypeError$2('Incompatible receiver, ' + TYPE + ' required');
	    } return state;
	  };
	};

	if (NATIVE_WEAK_MAP || shared$4.state) {
	  var store = shared$4.state || (shared$4.state = new WeakMap());
	  /* eslint-disable no-self-assign -- prototype methods protection */
	  store.get = store.get;
	  store.has = store.has;
	  store.set = store.set;
	  /* eslint-enable no-self-assign -- prototype methods protection */
	  set = function (it, metadata) {
	    if (store.has(it)) throw TypeError$2(OBJECT_ALREADY_INITIALIZED);
	    metadata.facade = it;
	    store.set(it, metadata);
	    return metadata;
	  };
	  get = function (it) {
	    return store.get(it) || {};
	  };
	  has = function (it) {
	    return store.has(it);
	  };
	} else {
	  var STATE = sharedKey$2('state');
	  hiddenKeys$1[STATE] = true;
	  set = function (it, metadata) {
	    if (hasOwn$5(it, STATE)) throw TypeError$2(OBJECT_ALREADY_INITIALIZED);
	    metadata.facade = it;
	    createNonEnumerableProperty$1(it, STATE, metadata);
	    return metadata;
	  };
	  get = function (it) {
	    return hasOwn$5(it, STATE) ? it[STATE] : {};
	  };
	  has = function (it) {
	    return hasOwn$5(it, STATE);
	  };
	}

	var internalState = {
	  set: set,
	  get: get,
	  has: has,
	  enforce: enforce,
	  getterFor: getterFor
	};

	var bind$2 = functionBindContext;
	var uncurryThis$7 = functionUncurryThis;
	var IndexedObject = indexedObject;
	var toObject$3 = toObject$6;
	var lengthOfArrayLike$3 = lengthOfArrayLike$7;
	var arraySpeciesCreate$1 = arraySpeciesCreate$3;

	var push$2 = uncurryThis$7([].push);

	// `Array.prototype.{ forEach, map, filter, some, every, find, findIndex, filterReject }` methods implementation
	var createMethod$1 = function (TYPE) {
	  var IS_MAP = TYPE == 1;
	  var IS_FILTER = TYPE == 2;
	  var IS_SOME = TYPE == 3;
	  var IS_EVERY = TYPE == 4;
	  var IS_FIND_INDEX = TYPE == 6;
	  var IS_FILTER_REJECT = TYPE == 7;
	  var NO_HOLES = TYPE == 5 || IS_FIND_INDEX;
	  return function ($this, callbackfn, that, specificCreate) {
	    var O = toObject$3($this);
	    var self = IndexedObject(O);
	    var boundFunction = bind$2(callbackfn, that);
	    var length = lengthOfArrayLike$3(self);
	    var index = 0;
	    var create = specificCreate || arraySpeciesCreate$1;
	    var target = IS_MAP ? create($this, length) : IS_FILTER || IS_FILTER_REJECT ? create($this, 0) : undefined;
	    var value, result;
	    for (;length > index; index++) if (NO_HOLES || index in self) {
	      value = self[index];
	      result = boundFunction(value, index, O);
	      if (TYPE) {
	        if (IS_MAP) target[index] = result; // map
	        else if (result) switch (TYPE) {
	          case 3: return true;              // some
	          case 5: return value;             // find
	          case 6: return index;             // findIndex
	          case 2: push$2(target, value);      // filter
	        } else switch (TYPE) {
	          case 4: return false;             // every
	          case 7: push$2(target, value);      // filterReject
	        }
	      }
	    }
	    return IS_FIND_INDEX ? -1 : IS_SOME || IS_EVERY ? IS_EVERY : target;
	  };
	};

	var arrayIteration = {
	  // `Array.prototype.forEach` method
	  // https://tc39.es/ecma262/#sec-array.prototype.foreach
	  forEach: createMethod$1(0),
	  // `Array.prototype.map` method
	  // https://tc39.es/ecma262/#sec-array.prototype.map
	  map: createMethod$1(1),
	  // `Array.prototype.filter` method
	  // https://tc39.es/ecma262/#sec-array.prototype.filter
	  filter: createMethod$1(2),
	  // `Array.prototype.some` method
	  // https://tc39.es/ecma262/#sec-array.prototype.some
	  some: createMethod$1(3),
	  // `Array.prototype.every` method
	  // https://tc39.es/ecma262/#sec-array.prototype.every
	  every: createMethod$1(4),
	  // `Array.prototype.find` method
	  // https://tc39.es/ecma262/#sec-array.prototype.find
	  find: createMethod$1(5),
	  // `Array.prototype.findIndex` method
	  // https://tc39.es/ecma262/#sec-array.prototype.findIndex
	  findIndex: createMethod$1(6),
	  // `Array.prototype.filterReject` method
	  // https://github.com/tc39/proposal-array-filtering
	  filterReject: createMethod$1(7)
	};

	var $$9 = _export;
	var global$4 = global$e;
	var call$5 = functionCall;
	var uncurryThis$6 = functionUncurryThis;
	var DESCRIPTORS$1 = descriptors;
	var NATIVE_SYMBOL$3 = symbolConstructorDetection;
	var fails$4 = fails$f;
	var hasOwn$4 = hasOwnProperty_1;
	var isPrototypeOf$2 = objectIsPrototypeOf;
	var anObject$3 = anObject$7;
	var toIndexedObject$1 = toIndexedObject$7;
	var toPropertyKey = toPropertyKey$5;
	var $toString = toString$5;
	var createPropertyDescriptor$1 = createPropertyDescriptor$5;
	var nativeObjectCreate = objectCreate;
	var objectKeys = objectKeys$2;
	var getOwnPropertyNamesModule = objectGetOwnPropertyNames;
	var getOwnPropertyNamesExternal = objectGetOwnPropertyNamesExternal;
	var getOwnPropertySymbolsModule$1 = objectGetOwnPropertySymbols;
	var getOwnPropertyDescriptorModule = objectGetOwnPropertyDescriptor;
	var definePropertyModule = objectDefineProperty;
	var definePropertiesModule = objectDefineProperties;
	var propertyIsEnumerableModule = objectPropertyIsEnumerable;
	var defineBuiltIn$2 = defineBuiltIn$4;
	var defineBuiltInAccessor = defineBuiltInAccessor$1;
	var shared$3 = sharedExports;
	var sharedKey$1 = sharedKey$4;
	var hiddenKeys = hiddenKeys$5;
	var uid = uid$3;
	var wellKnownSymbol$6 = wellKnownSymbol$g;
	var wrappedWellKnownSymbolModule = wellKnownSymbolWrapped;
	var defineWellKnownSymbol$l = wellKnownSymbolDefine;
	var defineSymbolToPrimitive$1 = symbolDefineToPrimitive;
	var setToStringTag$4 = setToStringTag$5;
	var InternalStateModule$2 = internalState;
	var $forEach = arrayIteration.forEach;

	var HIDDEN = sharedKey$1('hidden');
	var SYMBOL = 'Symbol';
	var PROTOTYPE = 'prototype';

	var setInternalState$2 = InternalStateModule$2.set;
	var getInternalState$2 = InternalStateModule$2.getterFor(SYMBOL);

	var ObjectPrototype$1 = Object[PROTOTYPE];
	var $Symbol = global$4.Symbol;
	var SymbolPrototype = $Symbol && $Symbol[PROTOTYPE];
	var TypeError$1 = global$4.TypeError;
	var QObject = global$4.QObject;
	var nativeGetOwnPropertyDescriptor = getOwnPropertyDescriptorModule.f;
	var nativeDefineProperty = definePropertyModule.f;
	var nativeGetOwnPropertyNames = getOwnPropertyNamesExternal.f;
	var nativePropertyIsEnumerable = propertyIsEnumerableModule.f;
	var push$1 = uncurryThis$6([].push);

	var AllSymbols = shared$3('symbols');
	var ObjectPrototypeSymbols = shared$3('op-symbols');
	var WellKnownSymbolsStore$1 = shared$3('wks');

	// Don't use setters in Qt Script, https://github.com/zloirock/core-js/issues/173
	var USE_SETTER = !QObject || !QObject[PROTOTYPE] || !QObject[PROTOTYPE].findChild;

	// fallback for old Android, https://code.google.com/p/v8/issues/detail?id=687
	var setSymbolDescriptor = DESCRIPTORS$1 && fails$4(function () {
	  return nativeObjectCreate(nativeDefineProperty({}, 'a', {
	    get: function () { return nativeDefineProperty(this, 'a', { value: 7 }).a; }
	  })).a != 7;
	}) ? function (O, P, Attributes) {
	  var ObjectPrototypeDescriptor = nativeGetOwnPropertyDescriptor(ObjectPrototype$1, P);
	  if (ObjectPrototypeDescriptor) delete ObjectPrototype$1[P];
	  nativeDefineProperty(O, P, Attributes);
	  if (ObjectPrototypeDescriptor && O !== ObjectPrototype$1) {
	    nativeDefineProperty(ObjectPrototype$1, P, ObjectPrototypeDescriptor);
	  }
	} : nativeDefineProperty;

	var wrap = function (tag, description) {
	  var symbol = AllSymbols[tag] = nativeObjectCreate(SymbolPrototype);
	  setInternalState$2(symbol, {
	    type: SYMBOL,
	    tag: tag,
	    description: description
	  });
	  if (!DESCRIPTORS$1) symbol.description = description;
	  return symbol;
	};

	var $defineProperty = function defineProperty(O, P, Attributes) {
	  if (O === ObjectPrototype$1) $defineProperty(ObjectPrototypeSymbols, P, Attributes);
	  anObject$3(O);
	  var key = toPropertyKey(P);
	  anObject$3(Attributes);
	  if (hasOwn$4(AllSymbols, key)) {
	    if (!Attributes.enumerable) {
	      if (!hasOwn$4(O, HIDDEN)) nativeDefineProperty(O, HIDDEN, createPropertyDescriptor$1(1, {}));
	      O[HIDDEN][key] = true;
	    } else {
	      if (hasOwn$4(O, HIDDEN) && O[HIDDEN][key]) O[HIDDEN][key] = false;
	      Attributes = nativeObjectCreate(Attributes, { enumerable: createPropertyDescriptor$1(0, false) });
	    } return setSymbolDescriptor(O, key, Attributes);
	  } return nativeDefineProperty(O, key, Attributes);
	};

	var $defineProperties = function defineProperties(O, Properties) {
	  anObject$3(O);
	  var properties = toIndexedObject$1(Properties);
	  var keys = objectKeys(properties).concat($getOwnPropertySymbols(properties));
	  $forEach(keys, function (key) {
	    if (!DESCRIPTORS$1 || call$5($propertyIsEnumerable, properties, key)) $defineProperty(O, key, properties[key]);
	  });
	  return O;
	};

	var $create = function create(O, Properties) {
	  return Properties === undefined ? nativeObjectCreate(O) : $defineProperties(nativeObjectCreate(O), Properties);
	};

	var $propertyIsEnumerable = function propertyIsEnumerable(V) {
	  var P = toPropertyKey(V);
	  var enumerable = call$5(nativePropertyIsEnumerable, this, P);
	  if (this === ObjectPrototype$1 && hasOwn$4(AllSymbols, P) && !hasOwn$4(ObjectPrototypeSymbols, P)) return false;
	  return enumerable || !hasOwn$4(this, P) || !hasOwn$4(AllSymbols, P) || hasOwn$4(this, HIDDEN) && this[HIDDEN][P]
	    ? enumerable : true;
	};

	var $getOwnPropertyDescriptor = function getOwnPropertyDescriptor(O, P) {
	  var it = toIndexedObject$1(O);
	  var key = toPropertyKey(P);
	  if (it === ObjectPrototype$1 && hasOwn$4(AllSymbols, key) && !hasOwn$4(ObjectPrototypeSymbols, key)) return;
	  var descriptor = nativeGetOwnPropertyDescriptor(it, key);
	  if (descriptor && hasOwn$4(AllSymbols, key) && !(hasOwn$4(it, HIDDEN) && it[HIDDEN][key])) {
	    descriptor.enumerable = true;
	  }
	  return descriptor;
	};

	var $getOwnPropertyNames = function getOwnPropertyNames(O) {
	  var names = nativeGetOwnPropertyNames(toIndexedObject$1(O));
	  var result = [];
	  $forEach(names, function (key) {
	    if (!hasOwn$4(AllSymbols, key) && !hasOwn$4(hiddenKeys, key)) push$1(result, key);
	  });
	  return result;
	};

	var $getOwnPropertySymbols = function (O) {
	  var IS_OBJECT_PROTOTYPE = O === ObjectPrototype$1;
	  var names = nativeGetOwnPropertyNames(IS_OBJECT_PROTOTYPE ? ObjectPrototypeSymbols : toIndexedObject$1(O));
	  var result = [];
	  $forEach(names, function (key) {
	    if (hasOwn$4(AllSymbols, key) && (!IS_OBJECT_PROTOTYPE || hasOwn$4(ObjectPrototype$1, key))) {
	      push$1(result, AllSymbols[key]);
	    }
	  });
	  return result;
	};

	// `Symbol` constructor
	// https://tc39.es/ecma262/#sec-symbol-constructor
	if (!NATIVE_SYMBOL$3) {
	  $Symbol = function Symbol() {
	    if (isPrototypeOf$2(SymbolPrototype, this)) throw TypeError$1('Symbol is not a constructor');
	    var description = !arguments.length || arguments[0] === undefined ? undefined : $toString(arguments[0]);
	    var tag = uid(description);
	    var setter = function (value) {
	      if (this === ObjectPrototype$1) call$5(setter, ObjectPrototypeSymbols, value);
	      if (hasOwn$4(this, HIDDEN) && hasOwn$4(this[HIDDEN], tag)) this[HIDDEN][tag] = false;
	      setSymbolDescriptor(this, tag, createPropertyDescriptor$1(1, value));
	    };
	    if (DESCRIPTORS$1 && USE_SETTER) setSymbolDescriptor(ObjectPrototype$1, tag, { configurable: true, set: setter });
	    return wrap(tag, description);
	  };

	  SymbolPrototype = $Symbol[PROTOTYPE];

	  defineBuiltIn$2(SymbolPrototype, 'toString', function toString() {
	    return getInternalState$2(this).tag;
	  });

	  defineBuiltIn$2($Symbol, 'withoutSetter', function (description) {
	    return wrap(uid(description), description);
	  });

	  propertyIsEnumerableModule.f = $propertyIsEnumerable;
	  definePropertyModule.f = $defineProperty;
	  definePropertiesModule.f = $defineProperties;
	  getOwnPropertyDescriptorModule.f = $getOwnPropertyDescriptor;
	  getOwnPropertyNamesModule.f = getOwnPropertyNamesExternal.f = $getOwnPropertyNames;
	  getOwnPropertySymbolsModule$1.f = $getOwnPropertySymbols;

	  wrappedWellKnownSymbolModule.f = function (name) {
	    return wrap(wellKnownSymbol$6(name), name);
	  };

	  if (DESCRIPTORS$1) {
	    // https://github.com/tc39/proposal-Symbol-description
	    defineBuiltInAccessor(SymbolPrototype, 'description', {
	      configurable: true,
	      get: function description() {
	        return getInternalState$2(this).description;
	      }
	    });
	  }
	}

	$$9({ global: true, constructor: true, wrap: true, forced: !NATIVE_SYMBOL$3, sham: !NATIVE_SYMBOL$3 }, {
	  Symbol: $Symbol
	});

	$forEach(objectKeys(WellKnownSymbolsStore$1), function (name) {
	  defineWellKnownSymbol$l(name);
	});

	$$9({ target: SYMBOL, stat: true, forced: !NATIVE_SYMBOL$3 }, {
	  useSetter: function () { USE_SETTER = true; },
	  useSimple: function () { USE_SETTER = false; }
	});

	$$9({ target: 'Object', stat: true, forced: !NATIVE_SYMBOL$3, sham: !DESCRIPTORS$1 }, {
	  // `Object.create` method
	  // https://tc39.es/ecma262/#sec-object.create
	  create: $create,
	  // `Object.defineProperty` method
	  // https://tc39.es/ecma262/#sec-object.defineproperty
	  defineProperty: $defineProperty,
	  // `Object.defineProperties` method
	  // https://tc39.es/ecma262/#sec-object.defineproperties
	  defineProperties: $defineProperties,
	  // `Object.getOwnPropertyDescriptor` method
	  // https://tc39.es/ecma262/#sec-object.getownpropertydescriptors
	  getOwnPropertyDescriptor: $getOwnPropertyDescriptor
	});

	$$9({ target: 'Object', stat: true, forced: !NATIVE_SYMBOL$3 }, {
	  // `Object.getOwnPropertyNames` method
	  // https://tc39.es/ecma262/#sec-object.getownpropertynames
	  getOwnPropertyNames: $getOwnPropertyNames
	});

	// `Symbol.prototype[@@toPrimitive]` method
	// https://tc39.es/ecma262/#sec-symbol.prototype-@@toprimitive
	defineSymbolToPrimitive$1();

	// `Symbol.prototype[@@toStringTag]` property
	// https://tc39.es/ecma262/#sec-symbol.prototype-@@tostringtag
	setToStringTag$4($Symbol, SYMBOL);

	hiddenKeys[HIDDEN] = true;

	var NATIVE_SYMBOL$2 = symbolConstructorDetection;

	/* eslint-disable es/no-symbol -- safe */
	var symbolRegistryDetection = NATIVE_SYMBOL$2 && !!Symbol['for'] && !!Symbol.keyFor;

	var $$8 = _export;
	var getBuiltIn$4 = getBuiltIn$9;
	var hasOwn$3 = hasOwnProperty_1;
	var toString$3 = toString$5;
	var shared$2 = sharedExports;
	var NATIVE_SYMBOL_REGISTRY$1 = symbolRegistryDetection;

	var StringToSymbolRegistry = shared$2('string-to-symbol-registry');
	var SymbolToStringRegistry$1 = shared$2('symbol-to-string-registry');

	// `Symbol.for` method
	// https://tc39.es/ecma262/#sec-symbol.for
	$$8({ target: 'Symbol', stat: true, forced: !NATIVE_SYMBOL_REGISTRY$1 }, {
	  'for': function (key) {
	    var string = toString$3(key);
	    if (hasOwn$3(StringToSymbolRegistry, string)) return StringToSymbolRegistry[string];
	    var symbol = getBuiltIn$4('Symbol')(string);
	    StringToSymbolRegistry[string] = symbol;
	    SymbolToStringRegistry$1[symbol] = string;
	    return symbol;
	  }
	});

	var $$7 = _export;
	var hasOwn$2 = hasOwnProperty_1;
	var isSymbol$2 = isSymbol$5;
	var tryToString$2 = tryToString$4;
	var shared$1 = sharedExports;
	var NATIVE_SYMBOL_REGISTRY = symbolRegistryDetection;

	var SymbolToStringRegistry = shared$1('symbol-to-string-registry');

	// `Symbol.keyFor` method
	// https://tc39.es/ecma262/#sec-symbol.keyfor
	$$7({ target: 'Symbol', stat: true, forced: !NATIVE_SYMBOL_REGISTRY }, {
	  keyFor: function keyFor(sym) {
	    if (!isSymbol$2(sym)) throw TypeError(tryToString$2(sym) + ' is not a symbol');
	    if (hasOwn$2(SymbolToStringRegistry, sym)) return SymbolToStringRegistry[sym];
	  }
	});

	var uncurryThis$5 = functionUncurryThis;

	var arraySlice$1 = uncurryThis$5([].slice);

	var uncurryThis$4 = functionUncurryThis;
	var isArray$1 = isArray$4;
	var isCallable$3 = isCallable$f;
	var classof$2 = classofRaw$2;
	var toString$2 = toString$5;

	var push = uncurryThis$4([].push);

	var getJsonReplacerFunction = function (replacer) {
	  if (isCallable$3(replacer)) return replacer;
	  if (!isArray$1(replacer)) return;
	  var rawLength = replacer.length;
	  var keys = [];
	  for (var i = 0; i < rawLength; i++) {
	    var element = replacer[i];
	    if (typeof element == 'string') push(keys, element);
	    else if (typeof element == 'number' || classof$2(element) == 'Number' || classof$2(element) == 'String') push(keys, toString$2(element));
	  }
	  var keysLength = keys.length;
	  var root = true;
	  return function (key, value) {
	    if (root) {
	      root = false;
	      return value;
	    }
	    if (isArray$1(this)) return value;
	    for (var j = 0; j < keysLength; j++) if (keys[j] === key) return value;
	  };
	};

	var $$6 = _export;
	var getBuiltIn$3 = getBuiltIn$9;
	var apply = functionApply;
	var call$4 = functionCall;
	var uncurryThis$3 = functionUncurryThis;
	var fails$3 = fails$f;
	var isCallable$2 = isCallable$f;
	var isSymbol$1 = isSymbol$5;
	var arraySlice = arraySlice$1;
	var getReplacerFunction = getJsonReplacerFunction;
	var NATIVE_SYMBOL$1 = symbolConstructorDetection;

	var $String = String;
	var $stringify = getBuiltIn$3('JSON', 'stringify');
	var exec = uncurryThis$3(/./.exec);
	var charAt$2 = uncurryThis$3(''.charAt);
	var charCodeAt$1 = uncurryThis$3(''.charCodeAt);
	var replace = uncurryThis$3(''.replace);
	var numberToString = uncurryThis$3(1.0.toString);

	var tester = /[\uD800-\uDFFF]/g;
	var low = /^[\uD800-\uDBFF]$/;
	var hi = /^[\uDC00-\uDFFF]$/;

	var WRONG_SYMBOLS_CONVERSION = !NATIVE_SYMBOL$1 || fails$3(function () {
	  var symbol = getBuiltIn$3('Symbol')();
	  // MS Edge converts symbol values to JSON as {}
	  return $stringify([symbol]) != '[null]'
	    // WebKit converts symbol values to JSON as null
	    || $stringify({ a: symbol }) != '{}'
	    // V8 throws on boxed symbols
	    || $stringify(Object(symbol)) != '{}';
	});

	// https://github.com/tc39/proposal-well-formed-stringify
	var ILL_FORMED_UNICODE = fails$3(function () {
	  return $stringify('\uDF06\uD834') !== '"\\udf06\\ud834"'
	    || $stringify('\uDEAD') !== '"\\udead"';
	});

	var stringifyWithSymbolsFix = function (it, replacer) {
	  var args = arraySlice(arguments);
	  var $replacer = getReplacerFunction(replacer);
	  if (!isCallable$2($replacer) && (it === undefined || isSymbol$1(it))) return; // IE8 returns string on undefined
	  args[1] = function (key, value) {
	    // some old implementations (like WebKit) could pass numbers as keys
	    if (isCallable$2($replacer)) value = call$4($replacer, this, $String(key), value);
	    if (!isSymbol$1(value)) return value;
	  };
	  return apply($stringify, null, args);
	};

	var fixIllFormed = function (match, offset, string) {
	  var prev = charAt$2(string, offset - 1);
	  var next = charAt$2(string, offset + 1);
	  if ((exec(low, match) && !exec(hi, next)) || (exec(hi, match) && !exec(low, prev))) {
	    return '\\u' + numberToString(charCodeAt$1(match, 0), 16);
	  } return match;
	};

	if ($stringify) {
	  // `JSON.stringify` method
	  // https://tc39.es/ecma262/#sec-json.stringify
	  $$6({ target: 'JSON', stat: true, arity: 3, forced: WRONG_SYMBOLS_CONVERSION || ILL_FORMED_UNICODE }, {
	    // eslint-disable-next-line no-unused-vars -- required for `.length`
	    stringify: function stringify(it, replacer, space) {
	      var args = arraySlice(arguments);
	      var result = apply(WRONG_SYMBOLS_CONVERSION ? stringifyWithSymbolsFix : $stringify, null, args);
	      return ILL_FORMED_UNICODE && typeof result == 'string' ? replace(result, tester, fixIllFormed) : result;
	    }
	  });
	}

	var $$5 = _export;
	var NATIVE_SYMBOL = symbolConstructorDetection;
	var fails$2 = fails$f;
	var getOwnPropertySymbolsModule = objectGetOwnPropertySymbols;
	var toObject$2 = toObject$6;

	// V8 ~ Chrome 38 and 39 `Object.getOwnPropertySymbols` fails on primitives
	// https://bugs.chromium.org/p/v8/issues/detail?id=3443
	var FORCED = !NATIVE_SYMBOL || fails$2(function () { getOwnPropertySymbolsModule.f(1); });

	// `Object.getOwnPropertySymbols` method
	// https://tc39.es/ecma262/#sec-object.getownpropertysymbols
	$$5({ target: 'Object', stat: true, forced: FORCED }, {
	  getOwnPropertySymbols: function getOwnPropertySymbols(it) {
	    var $getOwnPropertySymbols = getOwnPropertySymbolsModule.f;
	    return $getOwnPropertySymbols ? $getOwnPropertySymbols(toObject$2(it)) : [];
	  }
	});

	var defineWellKnownSymbol$k = wellKnownSymbolDefine;

	// `Symbol.asyncIterator` well-known symbol
	// https://tc39.es/ecma262/#sec-symbol.asynciterator
	defineWellKnownSymbol$k('asyncIterator');

	var defineWellKnownSymbol$j = wellKnownSymbolDefine;

	// `Symbol.hasInstance` well-known symbol
	// https://tc39.es/ecma262/#sec-symbol.hasinstance
	defineWellKnownSymbol$j('hasInstance');

	var defineWellKnownSymbol$i = wellKnownSymbolDefine;

	// `Symbol.isConcatSpreadable` well-known symbol
	// https://tc39.es/ecma262/#sec-symbol.isconcatspreadable
	defineWellKnownSymbol$i('isConcatSpreadable');

	var defineWellKnownSymbol$h = wellKnownSymbolDefine;

	// `Symbol.iterator` well-known symbol
	// https://tc39.es/ecma262/#sec-symbol.iterator
	defineWellKnownSymbol$h('iterator');

	var defineWellKnownSymbol$g = wellKnownSymbolDefine;

	// `Symbol.match` well-known symbol
	// https://tc39.es/ecma262/#sec-symbol.match
	defineWellKnownSymbol$g('match');

	var defineWellKnownSymbol$f = wellKnownSymbolDefine;

	// `Symbol.matchAll` well-known symbol
	// https://tc39.es/ecma262/#sec-symbol.matchall
	defineWellKnownSymbol$f('matchAll');

	var defineWellKnownSymbol$e = wellKnownSymbolDefine;

	// `Symbol.replace` well-known symbol
	// https://tc39.es/ecma262/#sec-symbol.replace
	defineWellKnownSymbol$e('replace');

	var defineWellKnownSymbol$d = wellKnownSymbolDefine;

	// `Symbol.search` well-known symbol
	// https://tc39.es/ecma262/#sec-symbol.search
	defineWellKnownSymbol$d('search');

	var defineWellKnownSymbol$c = wellKnownSymbolDefine;

	// `Symbol.species` well-known symbol
	// https://tc39.es/ecma262/#sec-symbol.species
	defineWellKnownSymbol$c('species');

	var defineWellKnownSymbol$b = wellKnownSymbolDefine;

	// `Symbol.split` well-known symbol
	// https://tc39.es/ecma262/#sec-symbol.split
	defineWellKnownSymbol$b('split');

	var defineWellKnownSymbol$a = wellKnownSymbolDefine;
	var defineSymbolToPrimitive = symbolDefineToPrimitive;

	// `Symbol.toPrimitive` well-known symbol
	// https://tc39.es/ecma262/#sec-symbol.toprimitive
	defineWellKnownSymbol$a('toPrimitive');

	// `Symbol.prototype[@@toPrimitive]` method
	// https://tc39.es/ecma262/#sec-symbol.prototype-@@toprimitive
	defineSymbolToPrimitive();

	var getBuiltIn$2 = getBuiltIn$9;
	var defineWellKnownSymbol$9 = wellKnownSymbolDefine;
	var setToStringTag$3 = setToStringTag$5;

	// `Symbol.toStringTag` well-known symbol
	// https://tc39.es/ecma262/#sec-symbol.tostringtag
	defineWellKnownSymbol$9('toStringTag');

	// `Symbol.prototype[@@toStringTag]` property
	// https://tc39.es/ecma262/#sec-symbol.prototype-@@tostringtag
	setToStringTag$3(getBuiltIn$2('Symbol'), 'Symbol');

	var defineWellKnownSymbol$8 = wellKnownSymbolDefine;

	// `Symbol.unscopables` well-known symbol
	// https://tc39.es/ecma262/#sec-symbol.unscopables
	defineWellKnownSymbol$8('unscopables');

	var global$3 = global$e;
	var setToStringTag$2 = setToStringTag$5;

	// JSON[@@toStringTag] property
	// https://tc39.es/ecma262/#sec-json-@@tostringtag
	setToStringTag$2(global$3.JSON, 'JSON', true);

	var path$2 = path$7;

	var symbol$3 = path$2.Symbol;

	var iterators = {};

	var DESCRIPTORS = descriptors;
	var hasOwn$1 = hasOwnProperty_1;

	var FunctionPrototype = Function.prototype;
	// eslint-disable-next-line es/no-object-getownpropertydescriptor -- safe
	var getDescriptor = DESCRIPTORS && Object.getOwnPropertyDescriptor;

	var EXISTS = hasOwn$1(FunctionPrototype, 'name');
	// additional protection from minified / mangled / dropped function names
	var PROPER = EXISTS && (function something() { /* empty */ }).name === 'something';
	var CONFIGURABLE = EXISTS && (!DESCRIPTORS || (DESCRIPTORS && getDescriptor(FunctionPrototype, 'name').configurable));

	var functionName = {
	  EXISTS: EXISTS,
	  PROPER: PROPER,
	  CONFIGURABLE: CONFIGURABLE
	};

	var fails$1 = fails$f;

	var correctPrototypeGetter = !fails$1(function () {
	  function F() { /* empty */ }
	  F.prototype.constructor = null;
	  // eslint-disable-next-line es/no-object-getprototypeof -- required for testing
	  return Object.getPrototypeOf(new F()) !== F.prototype;
	});

	var hasOwn = hasOwnProperty_1;
	var isCallable$1 = isCallable$f;
	var toObject$1 = toObject$6;
	var sharedKey = sharedKey$4;
	var CORRECT_PROTOTYPE_GETTER = correctPrototypeGetter;

	var IE_PROTO = sharedKey('IE_PROTO');
	var $Object = Object;
	var ObjectPrototype = $Object.prototype;

	// `Object.getPrototypeOf` method
	// https://tc39.es/ecma262/#sec-object.getprototypeof
	// eslint-disable-next-line es/no-object-getprototypeof -- safe
	var objectGetPrototypeOf = CORRECT_PROTOTYPE_GETTER ? $Object.getPrototypeOf : function (O) {
	  var object = toObject$1(O);
	  if (hasOwn(object, IE_PROTO)) return object[IE_PROTO];
	  var constructor = object.constructor;
	  if (isCallable$1(constructor) && object instanceof constructor) {
	    return constructor.prototype;
	  } return object instanceof $Object ? ObjectPrototype : null;
	};

	var fails = fails$f;
	var isCallable = isCallable$f;
	var isObject = isObject$8;
	var create$1 = objectCreate;
	var getPrototypeOf$1 = objectGetPrototypeOf;
	var defineBuiltIn$1 = defineBuiltIn$4;
	var wellKnownSymbol$5 = wellKnownSymbol$g;

	var ITERATOR$3 = wellKnownSymbol$5('iterator');
	var BUGGY_SAFARI_ITERATORS$1 = false;

	// `%IteratorPrototype%` object
	// https://tc39.es/ecma262/#sec-%iteratorprototype%-object
	var IteratorPrototype$1, PrototypeOfArrayIteratorPrototype, arrayIterator;

	/* eslint-disable es/no-array-prototype-keys -- safe */
	if ([].keys) {
	  arrayIterator = [].keys();
	  // Safari 8 has buggy iterators w/o `next`
	  if (!('next' in arrayIterator)) BUGGY_SAFARI_ITERATORS$1 = true;
	  else {
	    PrototypeOfArrayIteratorPrototype = getPrototypeOf$1(getPrototypeOf$1(arrayIterator));
	    if (PrototypeOfArrayIteratorPrototype !== Object.prototype) IteratorPrototype$1 = PrototypeOfArrayIteratorPrototype;
	  }
	}

	var NEW_ITERATOR_PROTOTYPE = !isObject(IteratorPrototype$1) || fails(function () {
	  var test = {};
	  // FF44- legacy iterators case
	  return IteratorPrototype$1[ITERATOR$3].call(test) !== test;
	});

	if (NEW_ITERATOR_PROTOTYPE) IteratorPrototype$1 = {};
	else IteratorPrototype$1 = create$1(IteratorPrototype$1);

	// `%IteratorPrototype%[@@iterator]()` method
	// https://tc39.es/ecma262/#sec-%iteratorprototype%-@@iterator
	if (!isCallable(IteratorPrototype$1[ITERATOR$3])) {
	  defineBuiltIn$1(IteratorPrototype$1, ITERATOR$3, function () {
	    return this;
	  });
	}

	var iteratorsCore = {
	  IteratorPrototype: IteratorPrototype$1,
	  BUGGY_SAFARI_ITERATORS: BUGGY_SAFARI_ITERATORS$1
	};

	var IteratorPrototype = iteratorsCore.IteratorPrototype;
	var create = objectCreate;
	var createPropertyDescriptor = createPropertyDescriptor$5;
	var setToStringTag$1 = setToStringTag$5;
	var Iterators$5 = iterators;

	var returnThis$1 = function () { return this; };

	var iteratorCreateConstructor = function (IteratorConstructor, NAME, next, ENUMERABLE_NEXT) {
	  var TO_STRING_TAG = NAME + ' Iterator';
	  IteratorConstructor.prototype = create(IteratorPrototype, { next: createPropertyDescriptor(+!ENUMERABLE_NEXT, next) });
	  setToStringTag$1(IteratorConstructor, TO_STRING_TAG, false, true);
	  Iterators$5[TO_STRING_TAG] = returnThis$1;
	  return IteratorConstructor;
	};

	var $$4 = _export;
	var call$3 = functionCall;
	var FunctionName = functionName;
	var createIteratorConstructor = iteratorCreateConstructor;
	var getPrototypeOf = objectGetPrototypeOf;
	var setToStringTag = setToStringTag$5;
	var defineBuiltIn = defineBuiltIn$4;
	var wellKnownSymbol$4 = wellKnownSymbol$g;
	var Iterators$4 = iterators;
	var IteratorsCore = iteratorsCore;

	var PROPER_FUNCTION_NAME = FunctionName.PROPER;
	var BUGGY_SAFARI_ITERATORS = IteratorsCore.BUGGY_SAFARI_ITERATORS;
	var ITERATOR$2 = wellKnownSymbol$4('iterator');
	var KEYS = 'keys';
	var VALUES = 'values';
	var ENTRIES = 'entries';

	var returnThis = function () { return this; };

	var iteratorDefine = function (Iterable, NAME, IteratorConstructor, next, DEFAULT, IS_SET, FORCED) {
	  createIteratorConstructor(IteratorConstructor, NAME, next);

	  var getIterationMethod = function (KIND) {
	    if (KIND === DEFAULT && defaultIterator) return defaultIterator;
	    if (!BUGGY_SAFARI_ITERATORS && KIND in IterablePrototype) return IterablePrototype[KIND];
	    switch (KIND) {
	      case KEYS: return function keys() { return new IteratorConstructor(this, KIND); };
	      case VALUES: return function values() { return new IteratorConstructor(this, KIND); };
	      case ENTRIES: return function entries() { return new IteratorConstructor(this, KIND); };
	    } return function () { return new IteratorConstructor(this); };
	  };

	  var TO_STRING_TAG = NAME + ' Iterator';
	  var INCORRECT_VALUES_NAME = false;
	  var IterablePrototype = Iterable.prototype;
	  var nativeIterator = IterablePrototype[ITERATOR$2]
	    || IterablePrototype['@@iterator']
	    || DEFAULT && IterablePrototype[DEFAULT];
	  var defaultIterator = !BUGGY_SAFARI_ITERATORS && nativeIterator || getIterationMethod(DEFAULT);
	  var anyNativeIterator = NAME == 'Array' ? IterablePrototype.entries || nativeIterator : nativeIterator;
	  var CurrentIteratorPrototype, methods, KEY;

	  // fix native
	  if (anyNativeIterator) {
	    CurrentIteratorPrototype = getPrototypeOf(anyNativeIterator.call(new Iterable()));
	    if (CurrentIteratorPrototype !== Object.prototype && CurrentIteratorPrototype.next) {
	      // Set @@toStringTag to native iterators
	      setToStringTag(CurrentIteratorPrototype, TO_STRING_TAG, true, true);
	      Iterators$4[TO_STRING_TAG] = returnThis;
	    }
	  }

	  // fix Array.prototype.{ values, @@iterator }.name in V8 / FF
	  if (PROPER_FUNCTION_NAME && DEFAULT == VALUES && nativeIterator && nativeIterator.name !== VALUES) {
	    {
	      INCORRECT_VALUES_NAME = true;
	      defaultIterator = function values() { return call$3(nativeIterator, this); };
	    }
	  }

	  // export additional methods
	  if (DEFAULT) {
	    methods = {
	      values: getIterationMethod(VALUES),
	      keys: IS_SET ? defaultIterator : getIterationMethod(KEYS),
	      entries: getIterationMethod(ENTRIES)
	    };
	    if (FORCED) for (KEY in methods) {
	      if (BUGGY_SAFARI_ITERATORS || INCORRECT_VALUES_NAME || !(KEY in IterablePrototype)) {
	        defineBuiltIn(IterablePrototype, KEY, methods[KEY]);
	      }
	    } else $$4({ target: NAME, proto: true, forced: BUGGY_SAFARI_ITERATORS || INCORRECT_VALUES_NAME }, methods);
	  }

	  // define iterator
	  if ((FORCED) && IterablePrototype[ITERATOR$2] !== defaultIterator) {
	    defineBuiltIn(IterablePrototype, ITERATOR$2, defaultIterator, { name: DEFAULT });
	  }
	  Iterators$4[NAME] = defaultIterator;

	  return methods;
	};

	// `CreateIterResultObject` abstract operation
	// https://tc39.es/ecma262/#sec-createiterresultobject
	var createIterResultObject$2 = function (value, done) {
	  return { value: value, done: done };
	};

	var toIndexedObject = toIndexedObject$7;
	var Iterators$3 = iterators;
	var InternalStateModule$1 = internalState;
	var defineIterator$1 = iteratorDefine;
	var createIterResultObject$1 = createIterResultObject$2;

	var ARRAY_ITERATOR = 'Array Iterator';
	var setInternalState$1 = InternalStateModule$1.set;
	var getInternalState$1 = InternalStateModule$1.getterFor(ARRAY_ITERATOR);

	// `Array.prototype.entries` method
	// https://tc39.es/ecma262/#sec-array.prototype.entries
	// `Array.prototype.keys` method
	// https://tc39.es/ecma262/#sec-array.prototype.keys
	// `Array.prototype.values` method
	// https://tc39.es/ecma262/#sec-array.prototype.values
	// `Array.prototype[@@iterator]` method
	// https://tc39.es/ecma262/#sec-array.prototype-@@iterator
	// `CreateArrayIterator` internal method
	// https://tc39.es/ecma262/#sec-createarrayiterator
	defineIterator$1(Array, 'Array', function (iterated, kind) {
	  setInternalState$1(this, {
	    type: ARRAY_ITERATOR,
	    target: toIndexedObject(iterated), // target
	    index: 0,                          // next index
	    kind: kind                         // kind
	  });
	// `%ArrayIteratorPrototype%.next` method
	// https://tc39.es/ecma262/#sec-%arrayiteratorprototype%.next
	}, function () {
	  var state = getInternalState$1(this);
	  var target = state.target;
	  var kind = state.kind;
	  var index = state.index++;
	  if (!target || index >= target.length) {
	    state.target = undefined;
	    return createIterResultObject$1(undefined, true);
	  }
	  if (kind == 'keys') return createIterResultObject$1(index, false);
	  if (kind == 'values') return createIterResultObject$1(target[index], false);
	  return createIterResultObject$1([index, target[index]], false);
	}, 'values');

	// argumentsList[@@iterator] is %ArrayProto_values%
	// https://tc39.es/ecma262/#sec-createunmappedargumentsobject
	// https://tc39.es/ecma262/#sec-createmappedargumentsobject
	Iterators$3.Arguments = Iterators$3.Array;

	// iterable DOM collections
	// flag - `iterable` interface - 'entries', 'keys', 'values', 'forEach' methods
	var domIterables = {
	  CSSRuleList: 0,
	  CSSStyleDeclaration: 0,
	  CSSValueList: 0,
	  ClientRectList: 0,
	  DOMRectList: 0,
	  DOMStringList: 0,
	  DOMTokenList: 1,
	  DataTransferItemList: 0,
	  FileList: 0,
	  HTMLAllCollection: 0,
	  HTMLCollection: 0,
	  HTMLFormElement: 0,
	  HTMLSelectElement: 0,
	  MediaList: 0,
	  MimeTypeArray: 0,
	  NamedNodeMap: 0,
	  NodeList: 1,
	  PaintRequestList: 0,
	  Plugin: 0,
	  PluginArray: 0,
	  SVGLengthList: 0,
	  SVGNumberList: 0,
	  SVGPathSegList: 0,
	  SVGPointList: 0,
	  SVGStringList: 0,
	  SVGTransformList: 0,
	  SourceBufferList: 0,
	  StyleSheetList: 0,
	  TextTrackCueList: 0,
	  TextTrackList: 0,
	  TouchList: 0
	};

	var DOMIterables = domIterables;
	var global$2 = global$e;
	var classof$1 = classof$7;
	var createNonEnumerableProperty = createNonEnumerableProperty$5;
	var Iterators$2 = iterators;
	var wellKnownSymbol$3 = wellKnownSymbol$g;

	var TO_STRING_TAG = wellKnownSymbol$3('toStringTag');

	for (var COLLECTION_NAME in DOMIterables) {
	  var Collection = global$2[COLLECTION_NAME];
	  var CollectionPrototype = Collection && Collection.prototype;
	  if (CollectionPrototype && classof$1(CollectionPrototype) !== TO_STRING_TAG) {
	    createNonEnumerableProperty(CollectionPrototype, TO_STRING_TAG, COLLECTION_NAME);
	  }
	  Iterators$2[COLLECTION_NAME] = Iterators$2.Array;
	}

	var parent$a = symbol$3;


	var symbol$2 = parent$a;

	var defineWellKnownSymbol$7 = wellKnownSymbolDefine;

	// `Symbol.dispose` well-known symbol
	// https://github.com/tc39/proposal-explicit-resource-management
	defineWellKnownSymbol$7('dispose');

	var parent$9 = symbol$2;



	var symbol$1 = parent$9;

	var defineWellKnownSymbol$6 = wellKnownSymbolDefine;

	// `Symbol.asyncDispose` well-known symbol
	// https://github.com/tc39/proposal-async-explicit-resource-management
	defineWellKnownSymbol$6('asyncDispose');

	var $$3 = _export;
	var getBuiltIn$1 = getBuiltIn$9;
	var uncurryThis$2 = functionUncurryThis;

	var Symbol$2 = getBuiltIn$1('Symbol');
	var keyFor = Symbol$2.keyFor;
	var thisSymbolValue$1 = uncurryThis$2(Symbol$2.prototype.valueOf);

	// `Symbol.isRegistered` method
	// https://tc39.es/proposal-symbol-predicates/#sec-symbol-isregistered
	$$3({ target: 'Symbol', stat: true }, {
	  isRegistered: function isRegistered(value) {
	    try {
	      return keyFor(thisSymbolValue$1(value)) !== undefined;
	    } catch (error) {
	      return false;
	    }
	  }
	});

	var $$2 = _export;
	var shared = sharedExports;
	var getBuiltIn = getBuiltIn$9;
	var uncurryThis$1 = functionUncurryThis;
	var isSymbol = isSymbol$5;
	var wellKnownSymbol$2 = wellKnownSymbol$g;

	var Symbol$1 = getBuiltIn('Symbol');
	var $isWellKnown = Symbol$1.isWellKnown;
	var getOwnPropertyNames = getBuiltIn('Object', 'getOwnPropertyNames');
	var thisSymbolValue = uncurryThis$1(Symbol$1.prototype.valueOf);
	var WellKnownSymbolsStore = shared('wks');

	for (var i = 0, symbolKeys = getOwnPropertyNames(Symbol$1), symbolKeysLength = symbolKeys.length; i < symbolKeysLength; i++) {
	  // some old engines throws on access to some keys like `arguments` or `caller`
	  try {
	    var symbolKey = symbolKeys[i];
	    if (isSymbol(Symbol$1[symbolKey])) wellKnownSymbol$2(symbolKey);
	  } catch (error) { /* empty */ }
	}

	// `Symbol.isWellKnown` method
	// https://tc39.es/proposal-symbol-predicates/#sec-symbol-iswellknown
	// We should patch it for newly added well-known symbols. If it's not required, this module just will not be injected
	$$2({ target: 'Symbol', stat: true, forced: true }, {
	  isWellKnown: function isWellKnown(value) {
	    if ($isWellKnown && $isWellKnown(value)) return true;
	    try {
	      var symbol = thisSymbolValue(value);
	      for (var j = 0, keys = getOwnPropertyNames(WellKnownSymbolsStore), keysLength = keys.length; j < keysLength; j++) {
	        if (WellKnownSymbolsStore[keys[j]] == symbol) return true;
	      }
	    } catch (error) { /* empty */ }
	    return false;
	  }
	});

	var defineWellKnownSymbol$5 = wellKnownSymbolDefine;

	// `Symbol.matcher` well-known symbol
	// https://github.com/tc39/proposal-pattern-matching
	defineWellKnownSymbol$5('matcher');

	var defineWellKnownSymbol$4 = wellKnownSymbolDefine;

	// `Symbol.metadataKey` well-known symbol
	// https://github.com/tc39/proposal-decorator-metadata
	defineWellKnownSymbol$4('metadataKey');

	var defineWellKnownSymbol$3 = wellKnownSymbolDefine;

	// `Symbol.observable` well-known symbol
	// https://github.com/tc39/proposal-observable
	defineWellKnownSymbol$3('observable');

	// TODO: Remove from `core-js@4`
	var defineWellKnownSymbol$2 = wellKnownSymbolDefine;

	// `Symbol.metadata` well-known symbol
	// https://github.com/tc39/proposal-decorators
	defineWellKnownSymbol$2('metadata');

	// TODO: remove from `core-js@4`
	var defineWellKnownSymbol$1 = wellKnownSymbolDefine;

	// `Symbol.patternMatch` well-known symbol
	// https://github.com/tc39/proposal-pattern-matching
	defineWellKnownSymbol$1('patternMatch');

	// TODO: remove from `core-js@4`
	var defineWellKnownSymbol = wellKnownSymbolDefine;

	defineWellKnownSymbol('replaceAll');

	var parent$8 = symbol$1;






	// TODO: Remove from `core-js@4`




	var symbol = parent$8;

	(function (module) {
		module.exports = symbol;
	} (symbol$4));

	(function (module) {
		module.exports = symbolExports;
	} (symbol$5));

	var iteratorExports$1 = {};
	var iterator$5 = {
	  get exports(){ return iteratorExports$1; },
	  set exports(v){ iteratorExports$1 = v; },
	};

	var iteratorExports = {};
	var iterator$4 = {
	  get exports(){ return iteratorExports; },
	  set exports(v){ iteratorExports = v; },
	};

	var uncurryThis = functionUncurryThis;
	var toIntegerOrInfinity = toIntegerOrInfinity$3;
	var toString$1 = toString$5;
	var requireObjectCoercible = requireObjectCoercible$3;

	var charAt$1 = uncurryThis(''.charAt);
	var charCodeAt = uncurryThis(''.charCodeAt);
	var stringSlice = uncurryThis(''.slice);

	var createMethod = function (CONVERT_TO_STRING) {
	  return function ($this, pos) {
	    var S = toString$1(requireObjectCoercible($this));
	    var position = toIntegerOrInfinity(pos);
	    var size = S.length;
	    var first, second;
	    if (position < 0 || position >= size) return CONVERT_TO_STRING ? '' : undefined;
	    first = charCodeAt(S, position);
	    return first < 0xD800 || first > 0xDBFF || position + 1 === size
	      || (second = charCodeAt(S, position + 1)) < 0xDC00 || second > 0xDFFF
	        ? CONVERT_TO_STRING
	          ? charAt$1(S, position)
	          : first
	        : CONVERT_TO_STRING
	          ? stringSlice(S, position, position + 2)
	          : (first - 0xD800 << 10) + (second - 0xDC00) + 0x10000;
	  };
	};

	var stringMultibyte = {
	  // `String.prototype.codePointAt` method
	  // https://tc39.es/ecma262/#sec-string.prototype.codepointat
	  codeAt: createMethod(false),
	  // `String.prototype.at` method
	  // https://github.com/mathiasbynens/String.prototype.at
	  charAt: createMethod(true)
	};

	var charAt = stringMultibyte.charAt;
	var toString = toString$5;
	var InternalStateModule = internalState;
	var defineIterator = iteratorDefine;
	var createIterResultObject = createIterResultObject$2;

	var STRING_ITERATOR = 'String Iterator';
	var setInternalState = InternalStateModule.set;
	var getInternalState = InternalStateModule.getterFor(STRING_ITERATOR);

	// `String.prototype[@@iterator]` method
	// https://tc39.es/ecma262/#sec-string.prototype-@@iterator
	defineIterator(String, 'String', function (iterated) {
	  setInternalState(this, {
	    type: STRING_ITERATOR,
	    string: toString(iterated),
	    index: 0
	  });
	// `%StringIteratorPrototype%.next` method
	// https://tc39.es/ecma262/#sec-%stringiteratorprototype%.next
	}, function next() {
	  var state = getInternalState(this);
	  var string = state.string;
	  var index = state.index;
	  var point;
	  if (index >= string.length) return createIterResultObject(undefined, true);
	  point = charAt(string, index);
	  state.index += point.length;
	  return createIterResultObject(point, false);
	});

	var WrappedWellKnownSymbolModule$1 = wellKnownSymbolWrapped;

	var iterator$3 = WrappedWellKnownSymbolModule$1.f('iterator');

	var parent$7 = iterator$3;


	var iterator$2 = parent$7;

	var parent$6 = iterator$2;

	var iterator$1 = parent$6;

	var parent$5 = iterator$1;

	var iterator = parent$5;

	(function (module) {
		module.exports = iterator;
	} (iterator$4));

	(function (module) {
		module.exports = iteratorExports;
	} (iterator$5));

	(function (module) {
		var _Symbol = symbolExports$1;
		var _Symbol$iterator = iteratorExports$1;
		function _typeof(obj) {
		  "@babel/helpers - typeof";

		  return (module.exports = _typeof = "function" == typeof _Symbol && "symbol" == typeof _Symbol$iterator ? function (obj) {
		    return typeof obj;
		  } : function (obj) {
		    return obj && "function" == typeof _Symbol && obj.constructor === _Symbol && obj !== _Symbol.prototype ? "symbol" : typeof obj;
		  }, module.exports.__esModule = true, module.exports["default"] = module.exports), _typeof(obj);
		}
		module.exports = _typeof, module.exports.__esModule = true, module.exports["default"] = module.exports;
	} (_typeof));

	var toPrimitiveExports$2 = {};
	var toPrimitive$6 = {
	  get exports(){ return toPrimitiveExports$2; },
	  set exports(v){ toPrimitiveExports$2 = v; },
	};

	var toPrimitiveExports$1 = {};
	var toPrimitive$5 = {
	  get exports(){ return toPrimitiveExports$1; },
	  set exports(v){ toPrimitiveExports$1 = v; },
	};

	var toPrimitiveExports = {};
	var toPrimitive$4 = {
	  get exports(){ return toPrimitiveExports; },
	  set exports(v){ toPrimitiveExports = v; },
	};

	var WrappedWellKnownSymbolModule = wellKnownSymbolWrapped;

	var toPrimitive$3 = WrappedWellKnownSymbolModule.f('toPrimitive');

	var parent$4 = toPrimitive$3;

	var toPrimitive$2 = parent$4;

	var parent$3 = toPrimitive$2;

	var toPrimitive$1 = parent$3;

	var parent$2 = toPrimitive$1;

	var toPrimitive = parent$2;

	(function (module) {
		module.exports = toPrimitive;
	} (toPrimitive$4));

	(function (module) {
		module.exports = toPrimitiveExports;
	} (toPrimitive$5));

	(function (module) {
		var _Symbol$toPrimitive = toPrimitiveExports$1;
		var _typeof = _typeofExports["default"];
		function _toPrimitive(input, hint) {
		  if (_typeof(input) !== "object" || input === null) return input;
		  var prim = input[_Symbol$toPrimitive];
		  if (prim !== undefined) {
		    var res = prim.call(input, hint || "default");
		    if (_typeof(res) !== "object") return res;
		    throw new TypeError("@@toPrimitive must return a primitive value.");
		  }
		  return (hint === "string" ? String : Number)(input);
		}
		module.exports = _toPrimitive, module.exports.__esModule = true, module.exports["default"] = module.exports;
	} (toPrimitive$6));

	(function (module) {
		var _typeof = _typeofExports["default"];
		var toPrimitive = toPrimitiveExports$2;
		function _toPropertyKey(arg) {
		  var key = toPrimitive(arg, "string");
		  return _typeof(key) === "symbol" ? key : String(key);
		}
		module.exports = _toPropertyKey, module.exports.__esModule = true, module.exports["default"] = module.exports;
	} (toPropertyKey$2));

	(function (module) {
		var _Object$defineProperty = definePropertyExports$2;
		var toPropertyKey = toPropertyKeyExports;
		function _defineProperty(obj, key, value) {
		  key = toPropertyKey(key);
		  if (key in obj) {
		    _Object$defineProperty(obj, key, {
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
		module.exports = _defineProperty, module.exports.__esModule = true, module.exports["default"] = module.exports;
	} (defineProperty$c));

	var _defineProperty = /*@__PURE__*/getDefaultExportFromCjs(definePropertyExports$3);

	var fromEntriesExports = {};
	var fromEntries$2 = {
	  get exports(){ return fromEntriesExports; },
	  set exports(v){ fromEntriesExports = v; },
	};

	var wellKnownSymbol$1 = wellKnownSymbol$g;
	var Iterators$1 = iterators;

	var ITERATOR$1 = wellKnownSymbol$1('iterator');
	var ArrayPrototype$1 = Array.prototype;

	// check on default Array iterator
	var isArrayIteratorMethod$1 = function (it) {
	  return it !== undefined && (Iterators$1.Array === it || ArrayPrototype$1[ITERATOR$1] === it);
	};

	var classof = classof$7;
	var getMethod$1 = getMethod$3;
	var isNullOrUndefined = isNullOrUndefined$3;
	var Iterators = iterators;
	var wellKnownSymbol = wellKnownSymbol$g;

	var ITERATOR = wellKnownSymbol('iterator');

	var getIteratorMethod$2 = function (it) {
	  if (!isNullOrUndefined(it)) return getMethod$1(it, ITERATOR)
	    || getMethod$1(it, '@@iterator')
	    || Iterators[classof(it)];
	};

	var call$2 = functionCall;
	var aCallable$1 = aCallable$4;
	var anObject$2 = anObject$7;
	var tryToString$1 = tryToString$4;
	var getIteratorMethod$1 = getIteratorMethod$2;

	var $TypeError$1 = TypeError;

	var getIterator$1 = function (argument, usingIterator) {
	  var iteratorMethod = arguments.length < 2 ? getIteratorMethod$1(argument) : usingIterator;
	  if (aCallable$1(iteratorMethod)) return anObject$2(call$2(iteratorMethod, argument));
	  throw $TypeError$1(tryToString$1(argument) + ' is not iterable');
	};

	var call$1 = functionCall;
	var anObject$1 = anObject$7;
	var getMethod = getMethod$3;

	var iteratorClose$1 = function (iterator, kind, value) {
	  var innerResult, innerError;
	  anObject$1(iterator);
	  try {
	    innerResult = getMethod(iterator, 'return');
	    if (!innerResult) {
	      if (kind === 'throw') throw value;
	      return value;
	    }
	    innerResult = call$1(innerResult, iterator);
	  } catch (error) {
	    innerError = true;
	    innerResult = error;
	  }
	  if (kind === 'throw') throw value;
	  if (innerError) throw innerResult;
	  anObject$1(innerResult);
	  return value;
	};

	var bind$1 = functionBindContext;
	var call = functionCall;
	var anObject = anObject$7;
	var tryToString = tryToString$4;
	var isArrayIteratorMethod = isArrayIteratorMethod$1;
	var lengthOfArrayLike$2 = lengthOfArrayLike$7;
	var isPrototypeOf$1 = objectIsPrototypeOf;
	var getIterator = getIterator$1;
	var getIteratorMethod = getIteratorMethod$2;
	var iteratorClose = iteratorClose$1;

	var $TypeError = TypeError;

	var Result = function (stopped, result) {
	  this.stopped = stopped;
	  this.result = result;
	};

	var ResultPrototype = Result.prototype;

	var iterate$1 = function (iterable, unboundFunction, options) {
	  var that = options && options.that;
	  var AS_ENTRIES = !!(options && options.AS_ENTRIES);
	  var IS_RECORD = !!(options && options.IS_RECORD);
	  var IS_ITERATOR = !!(options && options.IS_ITERATOR);
	  var INTERRUPTED = !!(options && options.INTERRUPTED);
	  var fn = bind$1(unboundFunction, that);
	  var iterator, iterFn, index, length, result, next, step;

	  var stop = function (condition) {
	    if (iterator) iteratorClose(iterator, 'normal', condition);
	    return new Result(true, condition);
	  };

	  var callFn = function (value) {
	    if (AS_ENTRIES) {
	      anObject(value);
	      return INTERRUPTED ? fn(value[0], value[1], stop) : fn(value[0], value[1]);
	    } return INTERRUPTED ? fn(value, stop) : fn(value);
	  };

	  if (IS_RECORD) {
	    iterator = iterable.iterator;
	  } else if (IS_ITERATOR) {
	    iterator = iterable;
	  } else {
	    iterFn = getIteratorMethod(iterable);
	    if (!iterFn) throw $TypeError(tryToString(iterable) + ' is not iterable');
	    // optimisation for array iterators
	    if (isArrayIteratorMethod(iterFn)) {
	      for (index = 0, length = lengthOfArrayLike$2(iterable); length > index; index++) {
	        result = callFn(iterable[index]);
	        if (result && isPrototypeOf$1(ResultPrototype, result)) return result;
	      } return new Result(false);
	    }
	    iterator = getIterator(iterable, iterFn);
	  }

	  next = IS_RECORD ? iterable.next : iterator.next;
	  while (!(step = call(next, iterator)).done) {
	    try {
	      result = callFn(step.value);
	    } catch (error) {
	      iteratorClose(iterator, 'throw', error);
	    }
	    if (typeof result == 'object' && result && isPrototypeOf$1(ResultPrototype, result)) return result;
	  } return new Result(false);
	};

	var $$1 = _export;
	var iterate = iterate$1;
	var createProperty = createProperty$3;

	// `Object.fromEntries` method
	// https://github.com/tc39/proposal-object-from-entries
	$$1({ target: 'Object', stat: true }, {
	  fromEntries: function fromEntries(iterable) {
	    var obj = {};
	    iterate(iterable, function (k, v) {
	      createProperty(obj, k, v);
	    }, { AS_ENTRIES: true });
	    return obj;
	  }
	});

	var path$1 = path$7;

	var fromEntries$1 = path$1.Object.fromEntries;

	var parent$1 = fromEntries$1;


	var fromEntries = parent$1;

	(function (module) {
		module.exports = fromEntries;
	} (fromEntries$2));

	var _Object$fromEntries = /*@__PURE__*/getDefaultExportFromCjs(fromEntriesExports);

	const conf = {
	  theme: {
	    // color to use when mixxx doesn't report a color for the control
	    // only used for RGB LEDs
	    fallbackHotcueColor: [127, 127, 127],
	    fallbackTrackColor: [127, 127, 127]
	  },
	  // what channels should be initially selected
	  initialSelection: [0, 1],
	  // mapping of sizes to presets
	  // list of presets are cycled through in the specified order,
	  // first element in the list serves as the default
	  // possible sizes are: grande (8x8), tall (4x8) and short (4x4)
	  // grid is indexed horizontally left-to-right then vertically bottom-to-top
	  // check for available controls and their parameters in the controls directory
	  presets: {
	    grande: [{
	      // 'Grande'
	      deck: [{
	        pos: [0, 0],
	        control: {
	          type: 'play'
	        }
	      }, {
	        pos: [1, 0],
	        control: {
	          type: 'sync'
	        }
	      }, {
	        pos: [2, 0],
	        control: {
	          type: 'nudge'
	        }
	      }, {
	        pos: [0, 1],
	        control: {
	          type: 'cue'
	        }
	      }, {
	        pos: [1, 1],
	        control: {
	          type: 'tap'
	        }
	      }, {
	        pos: [2, 1],
	        control: {
	          type: 'grid'
	        }
	      }, {
	        pos: [0, 2],
	        control: {
	          type: 'pfl'
	        }
	      }, {
	        pos: [1, 2],
	        control: {
	          type: 'quantize'
	        }
	      }, {
	        pos: [2, 2],
	        control: {
	          type: 'keyshift',
	          params: {
	            shifts: [[1, 1], [2, 2], [3, 3], [5, 4], [7, 5], [8, 6], [10, 7], [12, 8]],
	            rows: 2
	          }
	        }
	      }, {
	        pos: [0, 3],
	        control: {
	          type: 'load'
	        }
	      }, {
	        pos: [1, 3],
	        control: {
	          type: 'key'
	        }
	      }, {
	        pos: [0, 4],
	        control: {
	          type: 'hotcue',
	          params: {
	            cues: 8,
	            rows: 2
	          }
	        }
	      }, {
	        pos: [2, 6],
	        control: {
	          type: 'beatjump',
	          params: {
	            jumps: [[0.25, 1], [0.33, 2], [0.5, 4], [0.75, 8], [1, 16], [2, 32]]
	          }
	        }
	      }, {
	        pos: [4, 2],
	        control: {
	          type: 'beatloop',
	          params: {
	            loops: [0.5, 1, 2, 4, 8, 16, 32, 64],
	            rows: 2
	          }
	        }
	      }, {
	        pos: [6, 2],
	        control: {
	          type: 'loopjump',
	          params: {
	            jumps: [[0.5, 8], [1, 16], [2, 32], [4, 64]],
	            vertical: true
	          }
	        }
	      }, {
	        pos: [6, 1],
	        control: {
	          type: 'loopjumpSmall',
	          params: {
	            amount: 0.03125
	          }
	        }
	      }, {
	        pos: [4, 1],
	        control: {
	          type: 'loopMultiply'
	        }
	      }, {
	        pos: [4, 0],
	        control: {
	          type: 'reloop'
	        }
	      }, {
	        pos: [5, 0],
	        control: {
	          type: 'loopIo'
	        }
	      }, {
	        pos: [7, 0],
	        control: {
	          type: 'slip'
	        }
	      }]
	    }, {
	      samplerPalette: {
	        n: 64,
	        offset: 4,
	        rows: 8
	      }
	    }],
	    tall: [{
	      // 'Tall'
	      deck: [{
	        pos: [0, 0],
	        control: {
	          type: 'play'
	        }
	      }, {
	        pos: [1, 0],
	        control: {
	          type: 'sync'
	        }
	      }, {
	        pos: [2, 0],
	        control: {
	          type: 'nudge'
	        }
	      }, {
	        pos: [0, 1],
	        control: {
	          type: 'cue'
	        }
	      }, {
	        pos: [1, 1],
	        control: {
	          type: 'tap'
	        }
	      }, {
	        pos: [2, 1],
	        control: {
	          type: 'grid'
	        }
	      }, {
	        pos: [0, 2],
	        control: {
	          type: 'pfl'
	        }
	      }, {
	        pos: [1, 2],
	        control: {
	          type: 'quantize'
	        }
	      }, {
	        pos: [2, 2],
	        control: {
	          type: 'loopIo'
	        }
	      }, {
	        pos: [0, 3],
	        control: {
	          type: 'load'
	        }
	      }, {
	        pos: [1, 3],
	        control: {
	          type: 'key'
	        }
	      }, {
	        pos: [2, 3],
	        control: {
	          type: 'reloop'
	        }
	      }, {
	        pos: [3, 3],
	        control: {
	          type: 'slip'
	        }
	      }, {
	        pos: [0, 4],
	        control: {
	          type: 'hotcue',
	          params: {
	            cues: 4,
	            rows: 2
	          }
	        }
	      }, {
	        pos: [2, 4],
	        control: {
	          type: 'loopMultiply'
	        }
	      }, {
	        pos: [2, 5],
	        control: {
	          type: 'beatloop',
	          params: {
	            loops: [0.5, 1, 2, 4, 8, 16],
	            rows: 2
	          }
	        }
	      }, {
	        pos: [0, 6],
	        control: {
	          type: 'beatjump',
	          params: {
	            jumps: [[1, 16], [2, 32]]
	          }
	        }
	      }]
	    }, {
	      // 'Juggler'
	      deck: [{
	        pos: [0, 0],
	        control: {
	          type: 'play'
	        }
	      }, {
	        pos: [1, 0],
	        control: {
	          type: 'load'
	        }
	      }, {
	        pos: [2, 0],
	        control: {
	          type: 'beatjump',
	          params: {
	            jumps: [[0.5, 4], [1, 16], [2, 32], [4, 64]],
	            vertical: true
	          }
	        }
	      }, {
	        pos: [0, 1],
	        control: {
	          type: 'loopjump',
	          params: {
	            jumps: [[1, 16], [4, 64]]
	          }
	        }
	      }, {
	        pos: [0, 3],
	        control: {
	          type: 'reloop'
	        }
	      }, {
	        pos: [0, 4],
	        control: {
	          type: 'loopMultiply'
	        }
	      }, {
	        pos: [2, 4],
	        control: {
	          type: 'hotcue',
	          params: {
	            cues: 8,
	            rows: 2
	          }
	        }
	      }, {
	        pos: [0, 5],
	        control: {
	          type: 'beatloop',
	          params: {
	            loops: [0.5, 1, 2, 4, 8, 16],
	            rows: 2
	          }
	        }
	      }]
	    }],
	    short: [{
	      // 'Short'
	      deck: [{
	        pos: [0, 0],
	        control: {
	          type: 'play'
	        }
	      }, {
	        pos: [1, 0],
	        control: {
	          type: 'sync'
	        }
	      }, {
	        pos: [2, 0],
	        control: {
	          type: 'nudge'
	        }
	      }, {
	        pos: [0, 1],
	        control: {
	          type: 'cue'
	        }
	      }, {
	        pos: [1, 1],
	        control: {
	          type: 'tap'
	        }
	      }, {
	        pos: [2, 1],
	        control: {
	          type: 'grid'
	        }
	      }, {
	        pos: [0, 2],
	        control: {
	          type: 'pfl'
	        }
	      }, {
	        pos: [1, 2],
	        control: {
	          type: 'quantize'
	        }
	      }, {
	        pos: [2, 2],
	        control: {
	          type: 'loopIo'
	        }
	      }, {
	        pos: [0, 3],
	        control: {
	          type: 'load'
	        }
	      }, {
	        pos: [1, 3],
	        control: {
	          type: 'key'
	        }
	      }, {
	        pos: [2, 3],
	        control: {
	          type: 'reloop'
	        }
	      }, {
	        pos: [3, 3],
	        control: {
	          type: 'slip'
	        }
	      }]
	    }, {
	      // 'Sampler'
	      deck: [{
	        pos: [0, 0],
	        control: {
	          type: 'hotcue',
	          params: {
	            cues: 16,
	            rows: 4
	          }
	        }
	      }]
	    }]
	  }
	};
	var config = conf;

	var eventemitter3Exports = {};
	var eventemitter3 = {
	  get exports(){ return eventemitter3Exports; },
	  set exports(v){ eventemitter3Exports = v; },
	};

	(function (module) {

		var has = Object.prototype.hasOwnProperty
		  , prefix = '~';

		/**
		 * Constructor to create a storage for our `EE` objects.
		 * An `Events` instance is a plain object whose properties are event names.
		 *
		 * @constructor
		 * @private
		 */
		function Events() {}

		//
		// We try to not inherit from `Object.prototype`. In some engines creating an
		// instance in this way is faster than calling `Object.create(null)` directly.
		// If `Object.create(null)` is not supported we prefix the event names with a
		// character to make sure that the built-in object properties are not
		// overridden or used as an attack vector.
		//
		if (Object.create) {
		  Events.prototype = Object.create(null);

		  //
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

		  var listener = new EE(fn, context || emitter, once)
		    , evt = prefix ? prefix + event : event;

		  if (!emitter._events[evt]) emitter._events[evt] = listener, emitter._eventsCount++;
		  else if (!emitter._events[evt].fn) emitter._events[evt].push(listener);
		  else emitter._events[evt] = [emitter._events[evt], listener];

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
		  if (--emitter._eventsCount === 0) emitter._events = new Events();
		  else delete emitter._events[evt];
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
		  var names = []
		    , events
		    , name;

		  if (this._eventsCount === 0) return names;

		  for (name in (events = this._events)) {
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
		  var evt = prefix ? prefix + event : event
		    , handlers = this._events[evt];

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
		  var evt = prefix ? prefix + event : event
		    , listeners = this._events[evt];

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

		  var listeners = this._events[evt]
		    , len = arguments.length
		    , args
		    , i;

		  if (listeners.fn) {
		    if (listeners.once) this.removeListener(event, listeners.fn, undefined, true);

		    switch (len) {
		      case 1: return listeners.fn.call(listeners.context), true;
		      case 2: return listeners.fn.call(listeners.context, a1), true;
		      case 3: return listeners.fn.call(listeners.context, a1, a2), true;
		      case 4: return listeners.fn.call(listeners.context, a1, a2, a3), true;
		      case 5: return listeners.fn.call(listeners.context, a1, a2, a3, a4), true;
		      case 6: return listeners.fn.call(listeners.context, a1, a2, a3, a4, a5), true;
		    }

		    for (i = 1, args = new Array(len -1); i < len; i++) {
		      args[i - 1] = arguments[i];
		    }

		    listeners.fn.apply(listeners.context, args);
		  } else {
		    var length = listeners.length
		      , j;

		    for (i = 0; i < length; i++) {
		      if (listeners[i].once) this.removeListener(event, listeners[i].fn, undefined, true);

		      switch (len) {
		        case 1: listeners[i].fn.call(listeners[i].context); break;
		        case 2: listeners[i].fn.call(listeners[i].context, a1); break;
		        case 3: listeners[i].fn.call(listeners[i].context, a1, a2); break;
		        case 4: listeners[i].fn.call(listeners[i].context, a1, a2, a3); break;
		        default:
		          if (!args) for (j = 1, args = new Array(len -1); j < len; j++) {
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
		    if (
		      listeners.fn === fn &&
		      (!once || listeners.once) &&
		      (!context || listeners.context === context)
		    ) {
		      clearEvent(this, evt);
		    }
		  } else {
		    for (var i = 0, events = [], length = listeners.length; i < length; i++) {
		      if (
		        listeners[i].fn !== fn ||
		        (once && !listeners[i].once) ||
		        (context && listeners[i].context !== context)
		      ) {
		        events.push(listeners[i]);
		      }
		    }

		    //
		    // Reset the array, or remove it completely if we have no more listeners.
		    //
		    if (events.length) this._events[evt] = events.length === 1 ? events[0] : events;
		    else clearEvent(this, evt);
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
		};

		//
		// Alias methods names because people roll like that.
		//
		EventEmitter.prototype.off = EventEmitter.prototype.removeListener;
		EventEmitter.prototype.addListener = EventEmitter.prototype.on;

		//
		// Expose the prefix.
		//
		EventEmitter.prefixed = prefix;

		//
		// Allow `EventEmitter` to be imported as module namespace.
		//
		EventEmitter.EventEmitter = EventEmitter;

		//
		// Expose the module.
		//
		{
		  module.exports = EventEmitter;
		}
	} (eventemitter3));

	var EventEmitter = eventemitter3Exports;

	class Component extends eventemitter3Exports {
	  constructor() {
	    super();
	    _defineProperty(this, "mounted", false);
	  }
	  mount() {
	    this.onMount();
	    this.emit('mount', this);
	    this.mounted = true;
	  }
	  unmount() {
	    this.onUnmount();
	    this.emit('unmount', this);
	    this.mounted = false;
	  }

	  // eslint-disable-next-line @typescript-eslint/no-empty-function
	  onMount() {}

	  // eslint-disable-next-line @typescript-eslint/no-empty-function
	  onUnmount() {}
	}

	const posMod = function (x, n) {
	  return (x % n + n) % n;
	};
	const hexFormat = function (n, d) {
	  return '0x' + n.toString(16).toUpperCase().padStart(d, '0');
	};
	const range = function* (n) {
	  for (let i = 0; i < n; i++) {
	    yield i;
	  }
	};
	const array = function (n) {
	  return [...n];
	};
	const map = function* (f, n) {
	  for (const x of n) {
	    yield f(x);
	  }
	};
	class Lazy {
	  constructor(fn) {
	    _defineProperty(this, "_fn", void 0);
	    _defineProperty(this, "_cached", void 0);
	    _defineProperty(this, "_value", void 0);
	    this._fn = fn;
	    this._cached = false;
	    this._value = undefined;
	  }
	  get value() {
	    if (!this._cached) {
	      this._value = this._fn();
	      this._cached = true;
	    }
	    return this._value;
	  }
	}
	const lazy = function (fn) {
	  return new Lazy(fn);
	};

	// eslint-disable-next-line @typescript-eslint/no-explicit-any
	const isLazy = function (x) {
	  return x instanceof Lazy;
	};

	// eslint-disable-next-line @typescript-eslint/no-explicit-any

	const lazyArray = function (lazies) {
	  return new Proxy(lazies, {
	    get: function (target, prop) {
	      if (typeof prop === 'string' && Number.isInteger(Number(prop)) &&
	      // eslint-disable-next-line @typescript-eslint/ban-ts-comment
	      // @ts-ignore
	      isLazy(target[prop])) {
	        // key is an index
	        // eslint-disable-next-line @typescript-eslint/ban-ts-comment
	        // @ts-ignore
	        return target[prop].value;
	      } else {
	        // eslint-disable-next-line @typescript-eslint/ban-ts-comment
	        // @ts-ignore
	        return target[prop];
	      }
	    }
	  });
	};

	const playListControlDef = {
	  LoadSelectedIntoFirstStopped: {
	    group: '[Playlist]',
	    name: 'LoadSelectedIntoFirstStopped',
	    type: 'binary'
	  },
	  SelectNextPlaylist: {
	    group: '[Playlist]',
	    name: 'SelectNextPlaylist',
	    type: 'binary'
	  },
	  SelectPrevPlaylist: {
	    group: '[Playlist]',
	    name: 'SelectPrevPlaylist',
	    type: 'binary'
	  },
	  ToggleSelectedSidebarItem: {
	    group: '[Playlist]',
	    name: 'ToggleSelectedSidebarItem',
	    type: 'binary'
	  },
	  SelectNextTrack: {
	    group: '[Playlist]',
	    name: 'SelectNextTrack',
	    type: 'binary'
	  },
	  SelectPrevTrack: {
	    group: '[Playlist]',
	    name: 'SelectPrevTrack',
	    type: 'binary'
	  },
	  SelectTrackKnob: {
	    group: '[Playlist]',
	    name: 'SelectTrackKnob',
	    type: 'relative value'
	  },
	  AutoDjAddBottom: {
	    group: '[Playlist]',
	    name: 'AutoDjAddBottom',
	    type: 'binary'
	  },
	  AutoDjAddTop: {
	    group: '[Playlist]',
	    name: 'AutoDjAddTop',
	    type: 'binary'
	  }
	};
	const masterControlDef = {
	  maximize_library: {
	    group: '[Skin]',
	    name: 'show_maximized_library',
	    type: 'binary'
	  },
	  num_samplers: {
	    group: '[App]',
	    name: 'num_samplers',
	    type: 'number'
	  }
	};
	const createSamplerControlDef = function (i) {
	  const [type, number] = getChannelNameForOrdinal(i);
	  return {
	    LoadSelectedTrack: {
	      group: `[${type}${number}]`,
	      name: 'LoadSelectedTrack',
	      type: 'binary'
	    },
	    cue_gotoandplay: {
	      group: `[${type}${number}]`,
	      name: 'cue_gotoandplay',
	      type: 'binary'
	    },
	    eject: {
	      group: `[${type}${number}]`,
	      name: 'eject',
	      type: 'binary'
	    },
	    play: {
	      group: `[${type}${number}]`,
	      name: 'play',
	      type: 'binary'
	    },
	    play_latched: {
	      group: `[${type}${number}]`,
	      name: 'play_latched',
	      type: 'binary'
	    },
	    stop: {
	      group: `[${type}${number}]`,
	      name: 'stop',
	      type: 'binary'
	    },
	    track_color: {
	      group: `[${type}${number}]`,
	      name: 'track_color',
	      type: 'number'
	    },
	    track_loaded: {
	      group: `[${type}${number}]`,
	      name: 'track_loaded',
	      type: 'binary'
	    }
	  };
	};
	const numDecks = 4;
	const numSamplers = 64;
  // use 'root.master.num_samplers'?
  if (engine.getValue("[App]", "num_samplers") < numSamplers) {
    engine.setValue("[App]", "num_samplers", numSamplers);
  }
	const getChannelNameForOrdinal = function (i) {
	  return i < numDecks ? ['Channel', i + 1] : ['Sampler', i - 4 + 1];
	};
	array(map(createSamplerControlDef, range(numDecks + numSamplers)));

	// the full control palette for decks, minus repeated controls (e.g hotcues)

	const createSimpleChannelControlDef = function (type, i) {
	  return {
	    back: {
	      group: `[${type}${i}]`,
	      name: 'back',
	      type: 'binary'
	    },
	    beat_active: {
	      group: `[${type}${i}]`,
	      name: 'beat_active',
	      type: 'binary'
	    },
	    beatjump: {
	      group: `[${type}${i}]`,
	      name: 'beatjump',
	      type: 'real number'
	    },
	    beatloop: {
	      group: `[${type}${i}]`,
	      name: 'beatloop',
	      type: 'positive real number'
	    },
	    beats_adjust_faster: {
	      group: `[${type}${i}]`,
	      name: 'beats_adjust_faster',
	      type: 'binary'
	    },
	    beats_adjust_slower: {
	      group: `[${type}${i}]`,
	      name: 'beats_adjust_slower',
	      type: 'binary'
	    },
	    beats_translate_curpos: {
	      group: `[${type}${i}]`,
	      name: 'beats_translate_curpos',
	      type: 'binary'
	    },
	    beats_translate_match_alignment: {
	      group: `[${type}${i}]`,
	      name: 'beats_translate_match_alignment',
	      type: 'binary'
	    },
	    beats_translate_earlier: {
	      group: `[${type}${i}]`,
	      name: 'beats_translate_earlier',
	      type: 'binary'
	    },
	    beats_translate_later: {
	      group: `[${type}${i}]`,
	      name: 'beats_translate_later',
	      type: 'binary'
	    },
	    beatsync: {
	      group: `[${type}${i}]`,
	      name: 'beatsync',
	      type: 'binary'
	    },
	    beatsync_phase: {
	      group: `[${type}${i}]`,
	      name: 'beatsync_phase',
	      type: 'binary'
	    },
	    beatsync_tempo: {
	      group: `[${type}${i}]`,
	      name: 'beatsync_tempo',
	      type: 'binary'
	    },
	    bpm: {
	      group: `[${type}${i}]`,
	      name: 'bpm',
	      type: 'real-valued'
	    },
	    bpm_tap: {
	      group: `[${type}${i}]`,
	      name: 'bpm_tap',
	      type: 'binary'
	    },
	    cue_default: {
	      group: `[${type}${i}]`,
	      name: 'cue_default',
	      type: 'binary'
	    },
	    cue_gotoandplay: {
	      group: `[${type}${i}]`,
	      name: 'cue_gotoandplay',
	      type: 'binary'
	    },
	    cue_gotoandstop: {
	      group: `[${type}${i}]`,
	      name: 'cue_gotoandstop',
	      type: 'binary'
	    },
	    cue_indicator: {
	      group: `[${type}${i}]`,
	      name: 'cue_indicator',
	      type: 'binary'
	    },
	    cue_cdj: {
	      group: `[${type}${i}]`,
	      name: 'cue_cdj',
	      type: 'binary'
	    },
	    cue_play: {
	      group: `[${type}${i}]`,
	      name: 'cue_play',
	      type: 'binary'
	    },
	    cue_point: {
	      group: `[${type}${i}]`,
	      name: 'cue_point',
	      type: 'absolute value'
	    },
	    cue_preview: {
	      group: `[${type}${i}]`,
	      name: 'cue_preview',
	      type: 'binary'
	    },
	    cue_set: {
	      group: `[${type}${i}]`,
	      name: 'cue_set',
	      type: 'binary'
	    },
	    cue_simple: {
	      group: `[${type}${i}]`,
	      name: 'cue_simple',
	      type: 'binary'
	    },
	    duration: {
	      group: `[${type}${i}]`,
	      name: 'duration',
	      type: 'absolute value'
	    },
	    eject: {
	      group: `[${type}${i}]`,
	      name: 'eject',
	      type: 'binary'
	    },
	    end: {
	      group: `[${type}${i}]`,
	      name: 'end',
	      type: 'binary'
	    },
	    file_bpm: {
	      group: `[${type}${i}]`,
	      name: 'file_bpm',
	      type: 'positive value'
	    },
	    file_key: {
	      group: `[${type}${i}]`,
	      name: 'file_key',
	      type: '?'
	    },
	    fwd: {
	      group: `[${type}${i}]`,
	      name: 'fwd',
	      type: 'binary'
	    },
	    key: {
	      group: `[${type}${i}]`,
	      name: 'key',
	      type: 'real-valued'
	    },
	    keylock: {
	      group: `[${type}${i}]`,
	      name: 'keylock',
	      type: 'binary'
	    },
	    LoadSelectedTrack: {
	      group: `[${type}${i}]`,
	      name: 'LoadSelectedTrack',
	      type: 'binary'
	    },
	    LoadSelectedTrackAndPlay: {
	      group: `[${type}${i}]`,
	      name: 'LoadSelectedTrackAndPlay',
	      type: 'binary'
	    },
	    loop_double: {
	      group: `[${type}${i}]`,
	      name: 'loop_double',
	      type: 'binary'
	    },
	    loop_enabled: {
	      group: `[${type}${i}]`,
	      name: 'loop_enabled',
	      type: 'read-only, binary'
	    },
	    loop_end_position: {
	      group: `[${type}${i}]`,
	      name: 'loop_end_position',
	      type: 'positive integer'
	    },
	    loop_halve: {
	      group: `[${type}${i}]`,
	      name: 'loop_halve',
	      type: 'binary'
	    },
	    loop_in: {
	      group: `[${type}${i}]`,
	      name: 'loop_in',
	      type: 'binary'
	    },
	    loop_out: {
	      group: `[${type}${i}]`,
	      name: 'loop_out',
	      type: 'binary'
	    },
	    loop_move: {
	      group: `[${type}${i}]`,
	      name: 'loop_move',
	      type: 'real number'
	    },
	    loop_scale: {
	      group: `[${type}${i}]`,
	      name: 'loop_scale',
	      type: '0.0 - infinity'
	    },
	    loop_start_position: {
	      group: `[${type}${i}]`,
	      name: 'loop_start_position',
	      type: 'positive integer'
	    },
	    orientation: {
	      group: `[${type}${i}]`,
	      name: 'orientation',
	      type: '0-2'
	    },
	    passthrough: {
	      group: `[${type}${i}]`,
	      name: 'passthrough',
	      type: 'binary'
	    },
	    PeakIndicator: {
	      group: `[${type}${i}]`,
	      name: 'peak_indicator',
	      type: 'binary'
	    },
	    pfl: {
	      group: `[${type}${i}]`,
	      name: 'pfl',
	      type: 'binary'
	    },
	    pitch: {
	      group: `[${type}${i}]`,
	      name: 'pitch',
	      type: '-6.0..6.0'
	    },
	    pitch_adjust: {
	      group: `[${type}${i}]`,
	      name: 'pitch_adjust',
	      type: '-3.0..3.0'
	    },
	    play: {
	      group: `[${type}${i}]`,
	      name: 'play',
	      type: 'binary'
	    },
	    play_latched: {
	      group: `[${type}${i}]`,
	      name: 'play_latched',
	      type: 'binary'
	    },
	    play_indicator: {
	      group: `[${type}${i}]`,
	      name: 'play_indicator',
	      type: 'binary'
	    },
	    play_stutter: {
	      group: `[${type}${i}]`,
	      name: 'play_stutter',
	      type: 'binary'
	    },
	    playposition: {
	      group: `[${type}${i}]`,
	      name: 'playposition',
	      type: 'default'
	    },
	    pregain: {
	      group: `[${type}${i}]`,
	      name: 'pregain',
	      type: '0.0..1.0..4.0'
	    },
	    quantize: {
	      group: `[${type}${i}]`,
	      name: 'quantize',
	      type: 'binary'
	    },
	    rate: {
	      group: `[${type}${i}]`,
	      name: 'rate',
	      type: '-1.0..1.0'
	    },
	    rate_dir: {
	      group: `[${type}${i}]`,
	      name: 'rate_dir',
	      type: '-1 or 1'
	    },
	    rate_perm_down: {
	      group: `[${type}${i}]`,
	      name: 'rate_perm_down',
	      type: 'binary'
	    },
	    rate_perm_down_small: {
	      group: `[${type}${i}]`,
	      name: 'rate_perm_down_small',
	      type: 'binary'
	    },
	    rate_perm_up: {
	      group: `[${type}${i}]`,
	      name: 'rate_perm_up',
	      type: 'binary'
	    },
	    rate_perm_up_small: {
	      group: `[${type}${i}]`,
	      name: 'rate_perm_up_small',
	      type: 'binary'
	    },
	    rate_temp_down: {
	      group: `[${type}${i}]`,
	      name: 'rate_temp_down',
	      type: 'binary'
	    },
	    rate_temp_down_small: {
	      group: `[${type}${i}]`,
	      name: 'rate_temp_down_small',
	      type: 'binary'
	    },
	    rate_temp_up: {
	      group: `[${type}${i}]`,
	      name: 'rate_temp_up',
	      type: 'binary'
	    },
	    rate_temp_up_small: {
	      group: `[${type}${i}]`,
	      name: 'rate_temp_up_small',
	      type: 'binary'
	    },
	    rateRange: {
	      group: `[${type}${i}]`,
	      name: 'rateRange',
	      type: '0.0..3.0'
	    },
	    reloop_andstop: {
	      group: `[${type}${i}]`,
	      name: 'reloop_andstop',
	      type: 'binary'
	    },
	    reloop_exit: {
	      group: `[${type}${i}]`,
	      name: 'reloop_exit',
	      type: 'binary'
	    },
	    repeat: {
	      group: `[${type}${i}]`,
	      name: 'repeat',
	      type: 'binary'
	    },
	    reset_key: {
	      group: `[${type}${i}]`,
	      name: 'reset_key',
	      type: 'binary'
	    },
	    reverse: {
	      group: `[${type}${i}]`,
	      name: 'reverse',
	      type: 'binary'
	    },
	    reverseroll: {
	      group: `[${type}${i}]`,
	      name: 'reverseroll',
	      type: 'binary'
	    },
	    slip_enabled: {
	      group: `[${type}${i}]`,
	      name: 'slip_enabled',
	      type: 'binary'
	    },
	    start: {
	      group: `[${type}${i}]`,
	      name: 'start',
	      type: 'binary'
	    },
	    start_play: {
	      group: `[${type}${i}]`,
	      name: 'start_play',
	      type: 'binary'
	    },
	    start_stop: {
	      group: `[${type}${i}]`,
	      name: 'start_stop',
	      type: 'binary'
	    },
	    stop: {
	      group: `[${type}${i}]`,
	      name: 'stop',
	      type: 'binary'
	    },
	    sync_enabled: {
	      group: `[${type}${i}]`,
	      name: 'sync_enabled',
	      type: 'binary'
	    },
	    sync_master: {
	      group: `[${type}${i}]`,
	      name: 'sync_master',
	      type: 'binary'
	    },
	    sync_mode: {
	      group: `[${type}${i}]`,
	      name: 'sync_mode',
	      type: 'binary'
	    },
	    sync_key: {
	      group: `[${type}${i}]`,
	      name: 'sync_key',
	      type: '?'
	    },
	    track_color: {
	      group: `[${type}${i}]`,
	      name: 'track_color',
	      type: 'number'
	    },
	    track_loaded: {
	      group: `[${type}${i}]`,
	      name: 'track_loaded',
	      type: 'binary'
	    },
	    track_samplerate: {
	      group: `[${type}${i}]`,
	      name: 'track_samplerate',
	      type: 'absolute value'
	    },
	    track_samples: {
	      group: `[${type}${i}]`,
	      name: 'track_samples',
	      type: 'absolute value'
	    },
	    volume: {
	      group: `[${type}${i}]`,
	      name: 'volume',
	      type: 'default'
	    },
	    mute: {
	      group: `[${type}${i}]`,
	      name: 'mute',
	      type: 'binary'
	    },
	    vinylcontrol_enabled: {
	      group: `[${type}${i}]`,
	      name: 'vinylcontrol_enabled',
	      type: 'binary'
	    },
	    vinylcontrol_cueing: {
	      group: `[${type}${i}]`,
	      name: 'vinylcontrol_cueing',
	      type: '0.0-2.0'
	    },
	    vinylcontrol_mode: {
	      group: `[${type}${i}]`,
	      name: 'vinylcontrol_mode',
	      type: '0.0-2.0'
	    },
	    vinylcontrol_status: {
	      group: `[${type}${i}]`,
	      name: 'vinylcontrol_status',
	      type: '0.0-3.0 (read-only)'
	    },
	    visual_bpm: {
	      group: `[${type}${i}]`,
	      name: 'visual_bpm',
	      type: '?'
	    },
	    visual_key: {
	      group: `[${type}${i}]`,
	      name: 'visual_key',
	      type: '?'
	    },
	    visual_key_distance: {
	      group: `[${type}${i}]`,
	      name: 'visual_key_distance',
	      type: '-0.5..0.5'
	    },
	    VuMeter: {
	      group: `[${type}${i}]`,
	      name: 'VuMeter',
	      type: 'default'
	    },
	    VuMeterL: {
	      group: `[${type}${i}]`,
	      name: 'VuMeterL',
	      type: 'default'
	    },
	    VuMeterR: {
	      group: `[${type}${i}]`,
	      name: 'VuMeterR',
	      type: 'default'
	    },
	    waveform_zoom: {
	      group: `[${type}${i}]`,
	      name: 'waveform_zoom',
	      type: '1.0 - 10.0'
	    },
	    waveform_zoom_up: {
	      group: `[${type}${i}]`,
	      name: 'waveform_zoom_up',
	      type: '?'
	    },
	    waveform_zoom_down: {
	      group: `[${type}${i}]`,
	      name: 'waveform_zoom_down',
	      type: '?'
	    },
	    waveform_zoom_set_default: {
	      group: `[${type}${i}]`,
	      name: 'waveform_zoom_set_default',
	      type: '?'
	    },
	    wheel: {
	      group: `[${type}${i}]`,
	      name: 'wheel',
	      type: '-3.0..3.0'
	    }
	  };
	};
	const createArrayChannelControlDefCreators = function (type, i) {
	  return {
	    hotcues: function (x) {
	      return {
	        activate: {
	          group: `[${type}${i}]`,
	          name: `hotcue_${x}_activate`,
	          type: 'binary'
	        },
	        clear: {
	          group: `[${type}${i}]`,
	          name: `hotcue_${x}_clear`,
	          type: 'binary'
	        },
	        color: {
	          group: `[${type}${i}]`,
	          name: `hotcue_${x}_color`,
	          type: 'binary'
	        },
	        enabled: {
	          group: `[${type}${i}]`,
	          name: `hotcue_${x}_enabled`,
	          type: 'read-only, binary'
	        },
	        goto: {
	          group: `[${type}${i}]`,
	          name: `hotcue_${x}_goto`,
	          type: 'binary'
	        },
	        gotoandplay: {
	          group: `[${type}${i}]`,
	          name: `hotcue_${x}_gotoandplay`,
	          type: 'binary'
	        },
	        gotoandstop: {
	          group: `[${type}${i}]`,
	          name: `hotcue_${x}_gotoandstop`,
	          type: 'binary'
	        },
	        position: {
	          group: `[${type}${i}]`,
	          name: `hotcue_${x}_position`,
	          type: 'positive integer'
	        },
	        set: {
	          group: `[${type}${i}]`,
	          name: `hotcue_${x}_set`,
	          type: 'binary'
	        }
	      };
	    },
	    beatjumps: function (x) {
	      return {
	        forward: {
	          group: `[${type}${i}]`,
	          name: `beatjump_${x}_forward`,
	          type: 'binary'
	        },
	        backward: {
	          group: `[${type}${i}]`,
	          name: `beatjump_${x}_backward`,
	          type: 'binary'
	        }
	      };
	    },
	    beatloops: function (x) {
	      return {
	        activate: {
	          group: `[${type}${i}]`,
	          name: `beatloop_${x}_activate`,
	          type: 'binary'
	        },
	        toggle: {
	          group: `[${type}${i}]`,
	          name: `beatloop_${x}_toggle`,
	          type: 'binary'
	        },
	        enabled: {
	          group: `[${type}${i}]`,
	          name: `beatloop_${x}_enabled`,
	          type: 'binary'
	        }
	      };
	    }
	  };
	};
	const beatjumps = [0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64];
	const beatloops = [0.03125, 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64];
	const createArrayChannelControlDef = function (array, createOneDef) {
	  return array.reduce(function (arr, i) {
	    return Object.assign(arr, {
	      [i]: createOneDef(i)
	    });
	  }, {});
	};
	const createChannelControlDef = function (i) {
	  const [name, number] = getChannelNameForOrdinal(i);
	  const simpleChannelControlDef = createSimpleChannelControlDef(name, number);
	  const arrayChannelControlDefCreators = createArrayChannelControlDefCreators(name, number);
	  return Object.assign(simpleChannelControlDef, {
	    beatjumps: createArrayChannelControlDef(beatjumps, arrayChannelControlDefCreators.beatjumps),
	    beatloops: createArrayChannelControlDef(beatloops, arrayChannelControlDefCreators.beatloops),
	    hotcues: createArrayChannelControlDef(array(map(function (x) {
	      return x + 1;
	    }, range(16))), arrayChannelControlDefCreators.hotcues)
	  });
	};
	array(map(function (i) {
	  return createChannelControlDef(i);
	}, range(8)));

	// effect parameters
	const numEffectParameters = 8;
	const createEffectParameterDef = function (rack, unit, effect, parameter) {
	  return {
	    value: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `parameter${parameter}`,
	      type: 'number'
	    },
	    link_inverse: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `parameter${parameter}_link_inverse`,
	      type: 'binary'
	    },
	    link_type: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `parameter${parameter}_link_type`,
	      type: 'number'
	    },
	    loaded: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `parameter${parameter}_loaded`,
	      type: 'binary'
	    },
	    type: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `parameter${parameter}_type`,
	      type: 'number'
	    },
	    button_value: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `button_parameter${parameter}`,
	      type: 'number'
	    },
	    button_loaded: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `button_parameter${parameter}_loaded`,
	      type: 'binary'
	    },
	    button_type: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `button_parameter${parameter}_type`,
	      type: 'number'
	    }
	  };
	};
	const numEffects = 3;
	const createEffectDef = function (rack, unit, effect) {
	  return {
	    clear: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `clear`,
	      type: 'binary'
	    },
	    effect_selector: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `effect_selector`,
	      type: 'number'
	    },
	    enabled: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `enabled`,
	      type: 'binary'
	    },
	    loaded: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `loaded`,
	      type: 'binary'
	    },
	    next_effect: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `next_effect`,
	      type: 'binary'
	    },
	    num_parameters: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `num_parameters`,
	      type: 'number'
	    },
	    num_parameterslots: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `num_parameterslots`,
	      type: 'number'
	    },
	    num_button_parameters: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `num_button_parameters`,
	      type: 'number'
	    },
	    num_button_parameterslots: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `num_button_parameterslots`,
	      type: 'number'
	    },
	    meta: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `meta`,
	      type: 'number'
	    },
	    prev_effect: {
	      group: `[${rack}_${unit}_${effect}]`,
	      name: `prev_effect`,
	      type: 'binary'
	    },
	    parameters: lazyArray(array(map(function (i) {
	      return lazy(function () {
	        return createEffectParameterDef(rack, unit, effect, i + 1);
	      });
	    }, range(numEffectParameters))))
	  };
	};

	// effect units
	const numEffectUnits = 4;
	const createEffectUnitDef = function (rack, unit) {
	  return {
	    chain_selector: {
	      group: `[${rack}_${unit}]`,
	      name: `chain_selector`,
	      type: 'number'
	    },
	    clear: {
	      group: `[${rack}_${unit}]`,
	      name: `clear`,
	      type: 'binary'
	    },
	    enabled: {
	      group: `[${rack}_${unit}]`,
	      name: `enabled`,
	      type: 'binary'
	    },
	    focused_effect: {
	      group: `[${rack}_${unit}]`,
	      name: `focused_effect`,
	      type: 'number'
	    },
	    mix: {
	      group: `[${rack}_${unit}]`,
	      name: `mix`,
	      type: 'number'
	    },
	    super1: {
	      group: `[${rack}_${unit}]`,
	      name: `super1`,
	      type: 'number'
	    },
	    num_effects: {
	      group: `[${rack}_${unit}]`,
	      name: `num_effects`,
	      type: 'number'
	    },
	    num_effectslots: {
	      group: `[${rack}_${unit}]`,
	      name: `num_effectslots`,
	      type: 'number'
	    },
	    effects: lazyArray(array(map(function (i) {
	      return lazy(function () {
	        return createEffectDef(rack, unit, `Effect${i + 1}`);
	      });
	    }, range(numEffects))))
	  };
	};

	// effect racks

	const createEffectRackDef = function (rack) {
	  const units = rack.startsWith('EqualizerRack') || rack.startsWith('QuickEffectRack') ? map(function (i) {
	    return `[Channel${i + 1}]`;
	  }, range(numDecks)) : map(function (i) {
	    return `EffectUnit${i + 1}`;
	  }, range(numEffectUnits));
	  return {
	    num_effectunits: {
	      group: `[${rack}]`,
	      name: `num_effectunits`,
	      type: 'number'
	    },
	    clear: {
	      group: `[${rack}]`,
	      name: `clear`,
	      type: 'binary'
	    },
	    effect_units: lazyArray(array(map(function (unit) {
	      return lazy(function () {
	        return createEffectUnitDef(rack, unit);
	      });
	    }, units)))
	  };
	};
	const numEqualizerRacks = 1;
	const root = {
	  master: masterControlDef,
	  playList: playListControlDef,
	  samplers: lazyArray(array(map(function (i) {
	    return lazy(function () {
	      return createSamplerControlDef(i);
	    });
	  }, range(numDecks + numSamplers)))),
	  channels: lazyArray(array(map(function (i) {
	    return lazy(function () {
	      return createChannelControlDef(i);
	    });
	  }, range(numDecks + numSamplers)))),
	  effectRacks: array(map(function (i) {
	    return createEffectRackDef(`EffectRack${i + 1}`);
	  }, range(numEqualizerRacks))),
	  quickEffectRacks: array(map(function (i) {
	    return createEffectRackDef(`QuickEffectRack${i + 1}`);
	  }, range(numEqualizerRacks))),
	  equalizerRacks: array(map(function (i) {
	    return createEffectRackDef(`EqualizerRack${i + 1}`);
	  }, range(numEqualizerRacks)))
	};
	const getValue = function (control) {
	  return engine.getValue(control.group, control.name);
	};
	const setValue = function (control, value) {
	  return engine.setValue(control.group, control.name, value);
	};
	const softTakeover = function (control, enable) {
	  return engine.softTakeover(control.group, control.name, enable);
	};
	const softTakeOverIgnoreNextValue = function (control) {
	  return engine.softTakeoverIgnoreNextValue(control.group, control.name);
	};
	const connect = function (control, cb) {
	  const {
	    group,
	    name
	  } = control;
	  return engine.makeConnection(group, name, function (value) {
	    cb({
	      value,
	      control
	    });
	  });
	};
	const disconnect = function (handle) {
	  if (handle.isConnected) {
	    handle.disconnect();
	  }
	};
	class ControlComponent extends Component {
	  constructor(control, softTakeover) {
	    super();
	    _defineProperty(this, "control", void 0);
	    _defineProperty(this, "_handle", void 0);
	    _defineProperty(this, "_softTakeover", void 0);
	    this.control = control;
	    this._handle = null;
	    this._softTakeover = softTakeover;
	  }
	  onMount() {
	    var _this = this;
	    if (!this._handle) {
	      this._handle = connect(this.control, function (data) {
	        _this.emit('update', data);
	      });
	      if (this._softTakeover != null) {
	        softTakeover(this.control, this._softTakeover);
	      }
	      const initialMessage = {
	        control: this.control,
	        value: getValue(this.control)
	      };
	      this.emit('update', initialMessage);
	    }
	  }
	  onUnmount() {
	    if (this._handle) {
	      if (this._softTakeover != null) {
	        softTakeOverIgnoreNextValue(this.control);
	      }
	      disconnect(this._handle);
	      this._handle = null;
	    }
	  }
	}

	const midiCallbackPrefix = '__midi';
	// mixxx currently doesn't support custom names for sysex callback handlers, see https://github.com/mixxxdj/mixxx/issues/11536
	const sysexCallbackPrefix = 'incomingData';
	class MidiDevice extends Component {
	  constructor(...args) {
	    super(...args);
	    _defineProperty(this, "controls", void 0);
	    // whether to listen for sysex messages
	    _defineProperty(this, "sysex", false);
	  }
	  init() {
	    this.mount();
	  }
	  shutdown() {
	    this.unmount();
	  }
	  onMount() {
	    var _this2 = this;
	    super.onMount();
	    const _this = this;
	    Object.values(this.controls).forEach(function (control) {
	      _this[`${midiCallbackPrefix}_${hexFormat(control.status, 2)}_${hexFormat(control.midino, 2)}`] = function (_channel, _control, value, _status) {
	        const message = {
	          value,
	          control
	        };
	        _this2.emit(control.name, message);
	      };
	    });
	    if (this.sysex) {
	      _this[sysexCallbackPrefix] = function (data) {
	        _this2.emit('sysex', data);
	      };
	    }
	  }
	  onUnmount() {
	    super.onUnmount();
	  }
	}
	let MidiComponent$1 = class MidiComponent extends Component {
	  constructor(device, control) {
	    var _this3;
	    super();
	    _this3 = this;
	    _defineProperty(this, "control", void 0);
	    _defineProperty(this, "_cb", void 0);
	    _defineProperty(this, "_device", void 0);
	    this.control = control;
	    this._device = device;
	    this._cb = function (data) {
	      _this3.emit('midi', data);
	    };
	  }
	  onMount() {
	    super.onMount();
	    this._device.on(this.control.name, this._cb);
	  }
	  onUnmount() {
	    this._device.removeListener(this.control.name, this._cb);
	    super.onUnmount();
	  }
	};
	const sendShortMsg = function (control, value) {
	  midi.sendShortMsg(control.status, control.midino, value);
	};
	const sendSysexMsg = midi.sendSysexMsg;

	class Timer extends Component {
	  constructor(task) {
	    super();
	    _defineProperty(this, "task", void 0);
	    _defineProperty(this, "_state", void 0);
	    this.task = task;
	    this._state = null;
	  }
	  start(interval) {
	    if (this._state == null) {
	      const started = Date.now();
	      const handle = engine.beginTimer(interval, this.task.bind(this));
	      this._state = {
	        handle,
	        started
	      };
	      return started;
	    }
	    return this._state.started;
	  }
	  end() {
	    const state = this._state;
	    if (state != null) {
	      engine.stopTimer(state.handle);
	      this._state = null;
	    }
	  }
	  onUnmount() {
	    this.end();
	    super.onUnmount();
	  }
	  restart(interval) {
	    if (this._state != null) {
	      this.end();
	    }
	    return this.start(interval);
	  }
	}

	const parseRGBColor = function (number) {
	  if (number === -1) {
	    return null;
	  }
	  const blue = number % 256;
	  const green = (number >> 8) % 256;
	  const red = (number >> 16) % 256;
	  return [red, green, blue];
	};
	class LaunchpadDevice extends MidiDevice {
	  sendColor(control, value) {
	    sendShortMsg(control, value);
	  }
	  clearColor(control) {
	    sendShortMsg(control, this.colors.black);
	  }
	  constructor() {
	    super();
	    _defineProperty(this, "colors", void 0);
	    _defineProperty(this, "supportsRGBColors", void 0);
	  }
	  onMount() {
	    super.onMount();
	  }
	  onUnmount() {
	    super.onUnmount();
	  }
	}
	class MidiComponent extends MidiComponent$1 {}

	class ModifierSidebar extends Component {
	  constructor(device) {
	    var _this;
	    super();
	    _this = this;
	    _defineProperty(this, "shift", void 0);
	    _defineProperty(this, "ctrl", void 0);
	    _defineProperty(this, "state", void 0);
	    _defineProperty(this, "shiftListener", void 0);
	    _defineProperty(this, "ctrlListener", void 0);
	    this.shift = new MidiComponent(device, device.controls.solo);
	    this.ctrl = new MidiComponent(device, device.controls.arm);
	    this.state = {
	      shift: false,
	      ctrl: false
	    };
	    const makeListener = function (button) {
	      return function (message) {
	        const {
	          value
	        } = message;
	        if (value) {
	          device.sendColor(button.control, device.colors.hi_red);
	        } else {
	          device.clearColor(button.control);
	        }
	        if (button.control.name === device.controls.solo.name) {
	          _this.state.shift = !!value;
	          _this.emit('shift', value);
	        } else {
	          _this.state.ctrl = !!value;
	          _this.emit('ctrl', value);
	        }
	      };
	    };
	    this.shiftListener = makeListener(this.shift);
	    this.ctrlListener = makeListener(this.ctrl);
	  }
	  onMount() {
	    this.shift.mount();
	    this.ctrl.mount();
	    this.shift.on('midi', this.shiftListener);
	    this.ctrl.on('midi', this.ctrlListener);
	  }
	  onUnmount() {
	    this.shift.removeListener('midi', this.shiftListener);
	    this.ctrl.removeListener('midi', this.ctrlListener);
	    this.shift.unmount();
	    this.ctrl.unmount();
	  }
	  getState() {
	    return this.state;
	  }
	}
	const modes = function (ctx, n, c, s, cs) {
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
	const retainAttackMode = function (modifier, cb) {
	  let state = {
	    shift: false,
	    ctrl: false
	  };
	  return function (data) {
	    if (data.value) {
	      state = Object.assign(state, modifier.getState());
	    }
	    return cb(state, data);
	  };
	};

	var flatMapExports = {};
	var flatMap$3 = {
	  get exports(){ return flatMapExports; },
	  set exports(v){ flatMapExports = v; },
	};

	var isArray = isArray$4;
	var lengthOfArrayLike$1 = lengthOfArrayLike$7;
	var doesNotExceedSafeInteger = doesNotExceedSafeInteger$2;
	var bind = functionBindContext;

	// `FlattenIntoArray` abstract operation
	// https://tc39.github.io/proposal-flatMap/#sec-FlattenIntoArray
	var flattenIntoArray$1 = function (target, original, source, sourceLen, start, depth, mapper, thisArg) {
	  var targetIndex = start;
	  var sourceIndex = 0;
	  var mapFn = mapper ? bind(mapper, thisArg) : false;
	  var element, elementLen;

	  while (sourceIndex < sourceLen) {
	    if (sourceIndex in source) {
	      element = mapFn ? mapFn(source[sourceIndex], sourceIndex, original) : source[sourceIndex];

	      if (depth > 0 && isArray(element)) {
	        elementLen = lengthOfArrayLike$1(element);
	        targetIndex = flattenIntoArray$1(target, original, element, elementLen, targetIndex, depth - 1) - 1;
	      } else {
	        doesNotExceedSafeInteger(targetIndex + 1);
	        target[targetIndex] = element;
	      }

	      targetIndex++;
	    }
	    sourceIndex++;
	  }
	  return targetIndex;
	};

	var flattenIntoArray_1 = flattenIntoArray$1;

	var $ = _export;
	var flattenIntoArray = flattenIntoArray_1;
	var aCallable = aCallable$4;
	var toObject = toObject$6;
	var lengthOfArrayLike = lengthOfArrayLike$7;
	var arraySpeciesCreate = arraySpeciesCreate$3;

	// `Array.prototype.flatMap` method
	// https://tc39.es/ecma262/#sec-array.prototype.flatmap
	$({ target: 'Array', proto: true }, {
	  flatMap: function flatMap(callbackfn /* , thisArg */) {
	    var O = toObject(this);
	    var sourceLen = lengthOfArrayLike(O);
	    var A;
	    aCallable(callbackfn);
	    A = arraySpeciesCreate(O, 0);
	    A.length = flattenIntoArray(A, O, O, sourceLen, 0, 1, callbackfn, arguments.length > 1 ? arguments[1] : undefined);
	    return A;
	  }
	});

	var path = path$7;

	var entryVirtual$1 = function (CONSTRUCTOR) {
	  return path[CONSTRUCTOR + 'Prototype'];
	};

	var entryVirtual = entryVirtual$1;

	var flatMap$2 = entryVirtual('Array').flatMap;

	var isPrototypeOf = objectIsPrototypeOf;
	var method = flatMap$2;

	var ArrayPrototype = Array.prototype;

	var flatMap$1 = function (it) {
	  var own = it.flatMap;
	  return it === ArrayPrototype || (isPrototypeOf(ArrayPrototype, it) && own === ArrayPrototype.flatMap) ? method : own;
	};

	var parent = flatMap$1;

	var flatMap = parent;

	(function (module) {
		module.exports = flatMap;
	} (flatMap$3));

	var _flatMapInstanceProperty = /*@__PURE__*/getDefaultExportFromCjs(flatMapExports);

	const colors$2 = ['green', 'red'];
	const make$k = function ({
	  jumps,
	  vertical = false
	}, gridPosition, deck) {
	  const bindings = {};
	  const spec = _flatMapInstanceProperty(jumps).call(jumps, function (j) {
	    return [[j, -1], [j, 1]];
	  });
	  const onMidi = function (k, j, d) {
	    return function ({
	      bindings,
	      state,
	      context: {
	        modifier,
	        device
	      }
	    }) {
	      return retainAttackMode(modifier, function (mode, {
	        value
	      }) {
	        modes(mode, function () {
	          if (!state.mode) {
	            if (value) {
	              setValue(deck.beatjump, j[state.set] * d);
	            }
	          } else {
	            if (value) {
	              const currentJump = j[state.set] * d;
	              setValue(deck.beatjump, currentJump);
	              if (state.pressing != null) {
	                device.sendColor(bindings[state.pressing].control, device.colors[`lo_${colors$2[state.set]}`]);
	              }
	              device.sendColor(bindings[k].control, device.colors[`hi_${colors$2[state.set]}`]);
	              state.pressing = k;
	              state.diff = state.diff + currentJump;
	            } else {
	              if (state.pressing === k) {
	                device.sendColor(bindings[k].control, device.colors[`lo_${colors$2[state.set]}`]);
	                state.pressing = null;
	                setValue(deck.beatjump, -state.diff);
	                state.diff = 0;
	              }
	            }
	          }
	        }, function () {
	          if (value) {
	            state.set = posMod(state.set + 1, 2);
	            const prefix = state.mode ? 'lo' : 'hi';
	            for (let b = 0; b < spec.length; ++b) {
	              device.sendColor(bindings[b].control, device.colors[`${prefix}_${colors$2[state.set]}`]);
	            }
	          }
	        }, function () {
	          if (value) {
	            state.mode = !state.mode;
	            const prefix = state.mode ? 'lo' : 'hi';
	            for (let b = 0; b < spec.length; ++b) {
	              device.sendColor(bindings[b].control, device.colors[`${prefix}_${colors$2[state.set]}`]);
	            }
	          }
	        });
	      });
	    };
	  };
	  const onMount = function (k) {
	    return function ({
	      bindings,
	      state,
	      context: {
	        device
	      }
	    }) {
	      return function () {
	        const prefix = state.mode ? 'lo' : 'hi';
	        device.sendColor(bindings[k].control, device.colors[`${prefix}_${colors$2[state.set]}`]);
	      };
	    };
	  };
	  spec.forEach(function ([jump, dir], i) {
	    bindings[i] = {
	      type: 'button',
	      target: vertical ? [gridPosition[0] + i % 2, gridPosition[1] + ~~(i / 2)] : [gridPosition[0] + ~~(i / 2), gridPosition[1] + i % 2],
	      midi: onMidi(i, jump, dir),
	      mount: onMount(i)
	    };
	  });
	  return {
	    bindings,
	    state: {
	      mode: false,
	      pressing: null,
	      diff: 0,
	      set: 0
	    }
	  };
	};
	var makeBeatjump = make$k;

	const onAttack = function (handler) {
	  return function (m) {
	    if (m.value) handler(m);
	  };
	};

	const make$j = function (params, gridPosition, deck) {
	  const {
	    loops,
	    rows
	  } = params;
	  const bindings = {};
	  const onMidi = function (loop) {
	    return function ({
	      context
	    }) {
	      return onAttack(function () {
	        const {
	          modifier
	        } = context;
	        modes(modifier.getState(), function () {
	          return setValue(deck.beatloops[loop].toggle, 1);
	        });
	      });
	    };
	  };
	  const onUpdate = function (i) {
	    return function ({
	      context,
	      bindings
	    }) {
	      return function ({
	        value
	      }) {
	        const {
	          device
	        } = context;
	        const color = value ? device.colors.hi_red : device.colors.lo_red;
	        device.sendColor(bindings[`b.${i}`].control, color);
	      };
	    };
	  };
	  loops.forEach(function (loop, i) {
	    const dx = i % rows;
	    const dy = ~~(i / rows);
	    bindings[`b.${i}`] = {
	      type: 'button',
	      target: [gridPosition[0] + dx, gridPosition[1] + dy],
	      midi: onMidi(loop)
	    };
	    bindings[`c.${loop}`] = {
	      type: 'control',
	      target: deck.beatloops[loop].enabled,
	      update: onUpdate(i)
	    };
	  });
	  return {
	    bindings,
	    state: {}
	  };
	};
	var makeBeatloop = make$j;

	const make$i = function (_, gridPosition, deck) {
	  return {
	    state: {},
	    bindings: {
	      cue: {
	        type: 'button',
	        target: gridPosition,
	        midi: function ({
	          context: {
	            modifier
	          }
	        }) {
	          return retainAttackMode(modifier, function (mode, {
	            value
	          }) {
	            modes(mode, function () {
	              setValue(deck.cue_default, value ? 1 : 0);
	            }, function () {
	              return value && setValue(deck.cue_set, 1);
	            });
	          });
	        }
	      },
	      cueIndicator: {
	        type: 'control',
	        target: deck.cue_indicator,
	        update: function ({
	          bindings: {
	            cue: {
	              control
	            }
	          },
	          context: {
	            device
	          }
	        }) {
	          return function ({
	            value
	          }) {
	            if (value) {
	              device.sendColor(control, device.colors.hi_red);
	            } else if (!value) {
	              device.clearColor(control);
	            }
	          };
	        }
	      }
	    }
	  };
	};
	var makeCue = make$i;

	const make$h = function (_, gridPosition, deck) {
	  const steps = {
	    back: {
	      normal: deck.beats_translate_earlier,
	      ctrl: deck.beats_adjust_slower
	    },
	    forth: {
	      normal: deck.beats_translate_later,
	      ctrl: deck.beats_adjust_faster
	    }
	  };
	  const onGrid = function (dir) {
	    return function ({
	      context: {
	        device,
	        modifier
	      },
	      bindings
	    }) {
	      return function ({
	        value
	      }) {
	        if (!value) {
	          device.clearColor(bindings[dir].control);
	        } else {
	          modes(modifier.getState(), function () {
	            device.sendColor(bindings[dir].control, device.colors.hi_yellow);
	            setValue(steps[dir].normal, 1);
	          }, function () {
	            device.sendColor(bindings[dir].control, device.colors.hi_amber);
	            setValue(steps[dir].ctrl, 1);
	          });
	        }
	      };
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
	    state: {}
	  };
	};
	var makeGrid = make$h;

	const make$g = function ({
	  cues,
	  rows,
	  start = 0
	}, gridPosition, deck, theme) {
	  const onHotcueMidi = function (i) {
	    return function ({
	      context: {
	        modifier
	      },
	      bindings
	    }) {
	      return function ({
	        value
	      }) {
	        modes(modifier.getState(), function () {
	          setValue(deck.hotcues[1 + i + start].activate, value ? 1 : 0);
	        }, function () {
	          if (value) {
	            if (getValue(bindings[`cue.${i}`].control)) {
	              setValue(deck.hotcues[1 + i + start].clear, 1);
	            } else {
	              setValue(deck.hotcues[1 + i + start].set, 1);
	            }
	          }
	        });
	      };
	    };
	  };
	  const onHotcueColorChanged = function (i) {
	    return function ({
	      context: {
	        device
	      },
	      bindings
	    }) {
	      return function ({
	        value
	      }) {
	        const color = parseRGBColor(value);
	        if (device.supportsRGBColors) {
	          device.sendRGBColor(bindings[`midi.${i}`].control, color == null ? theme.fallbackHotcueColor : color);
	        }
	      };
	    };
	  };
	  const onHotcueEnabled = function (i) {
	    return function ({
	      context: {
	        device
	      },
	      bindings
	    }) {
	      return function ({
	        value
	      }) {
	        if (value) {
	          if (device.supportsRGBColors) {
	            const color = parseRGBColor(getValue(deck.hotcues[1 + i + start].color));
	            device.sendRGBColor(bindings[`midi.${i}`].control, color == null ? theme.fallbackHotcueColor : color);
	          } else {
	            device.sendColor(bindings[`midi.${i}`].control, device.colors.lo_yellow);
	          }
	        } else {
	          device.clearColor(bindings[`midi.${i}`].control);
	        }
	      };
	    };
	  };
	  const bindings = {};
	  for (const i of range(cues)) {
	    const dx = i % rows;
	    const dy = ~~(i / rows);
	    bindings[`midi.${i}`] = {
	      type: 'button',
	      target: [gridPosition[0] + dx, gridPosition[1] + dy],
	      midi: onHotcueMidi(i)
	    };
	    bindings[`cue.${i}`] = {
	      type: 'control',
	      target: deck.hotcues[1 + i + start].enabled,
	      update: onHotcueEnabled(i)
	    };
	    bindings[`color.${i}`] = {
	      type: 'control',
	      target: deck.hotcues[1 + i + start].color,
	      update: onHotcueColorChanged(i)
	    };
	  }
	  return {
	    bindings,
	    state: {}
	  };
	};
	var makeHotcue = make$g;

	const make$f = function (_, gridPosition, deck) {
	  return {
	    state: {},
	    bindings: {
	      button: {
	        type: 'button',
	        target: gridPosition,
	        midi: function ({
	          context: {
	            modifier
	          },
	          bindings
	        }) {
	          return onAttack(function () {
	            modes(modifier.getState(), function () {
	              setValue(bindings.keylock.control, Number(!getValue(bindings.keylock.control)));
	            }, function () {
	              setValue(deck.key, getValue(deck.key) - 1);
	            }, function () {
	              setValue(deck.key, getValue(deck.key) + 1);
	            }, function () {
	              setValue(deck.reset_key, 1);
	            });
	          });
	        }
	      },
	      keylock: {
	        type: 'control',
	        target: deck.keylock,
	        update: function ({
	          context: {
	            device
	          },
	          bindings
	        }) {
	          return function ({
	            value
	          }) {
	            if (value) {
	              device.sendColor(bindings.button.control, device.colors.hi_red);
	            } else {
	              device.clearColor(bindings.button.control);
	            }
	          };
	        }
	      }
	    }
	  };
	};
	var makeKey = make$f;

	const colors$1 = ['green', 'red'];
	const make$e = function ({
	  shifts,
	  rows
	}, gridPosition, deck) {
	  const bindings = {};
	  const temporaryChange = function (i, value, bindings, state, device) {
	    if (value) {
	      const base = state.on === -1 ? getValue(deck.key) : state.base;
	      if (state.on !== -1) {
	        device.sendColor(bindings[state.on].control, device.colors[`lo_${colors$1[state.set]}`]);
	      }
	      device.sendColor(bindings[i].control, device.colors[`hi_${colors$1[state.set]}`]);
	      setValue(deck.key, (base + shifts[i][state.set]) % 12 + 12);
	      state.on = i;
	      state.base = base;
	    } else {
	      if (state.on === i) {
	        device.sendColor(bindings[i].control, device.colors[`lo_${colors$1[state.set]}`]);
	        setValue(deck.key, state.base);
	        state.on = -1;
	      }
	    }
	  };
	  const onMidi = function (i) {
	    return function ({
	      context: {
	        modifier,
	        device
	      },
	      bindings,
	      state
	    }) {
	      return retainAttackMode(modifier, function (mode, {
	        value
	      }) {
	        modes(mode, function () {
	          return temporaryChange(i, value, bindings, state, device);
	        }, function () {
	          if (value) {
	            state.set = posMod(state.set + 1, 2);
	            for (let i = 0; i < shifts.length; ++i) {
	              device.sendColor(bindings[i].control, device.colors[`lo_${colors$1[state.set]}`]);
	            }
	          }
	        });
	      });
	    };
	  };
	  shifts.forEach(function (_, i) {
	    const dx = i % rows;
	    const dy = ~~(i / rows);
	    const position = [gridPosition[0] + dx, gridPosition[1] + dy];
	    bindings[i] = {
	      type: 'button',
	      target: position,
	      midi: onMidi(i),
	      mount: function ({
	        context: {
	          device
	        },
	        bindings,
	        state
	      }) {
	        return function () {
	          device.sendColor(bindings[i].control, device.colors[`lo_${colors$1[state.set]}`]);
	        };
	      }
	    };
	  });
	  return {
	    bindings,
	    state: {
	      on: -1,
	      base: -1,
	      set: 0
	    }
	  };
	};
	var makeKeyshift = make$e;

	const make$d = function (_, gridPosition, deck) {
	  const onStateChanged = function (loaded, playing, bindings, device) {
	    if (loaded && playing) {
	      device.sendColor(bindings.button.control, device.colors.lo_red);
	    } else if (loaded) {
	      device.sendColor(bindings.button.control, device.colors.lo_yellow);
	    } else {
	      device.sendColor(bindings.button.control, device.colors.lo_green);
	    }
	  };
	  return {
	    state: {},
	    bindings: {
	      samples: {
	        type: 'control',
	        target: deck.track_samples,
	        update: function ({
	          bindings,
	          context: {
	            device
	          }
	        }) {
	          return function ({
	            value
	          }) {
	            return onStateChanged(value, getValue(bindings.play.control), bindings, device);
	          };
	        }
	      },
	      play: {
	        type: 'control',
	        target: deck.play,
	        update: function ({
	          bindings,
	          context: {
	            device
	          }
	        }) {
	          return function ({
	            value
	          }) {
	            return onStateChanged(getValue(bindings.samples.control), value, bindings, device);
	          };
	        }
	      },
	      button: {
	        type: 'button',
	        target: gridPosition,
	        midi: function ({
	          bindings,
	          context: {
	            modifier
	          }
	        }) {
	          return onAttack(function () {
	            modes(modifier.getState(), function () {
	              if (!getValue(bindings.samples.control)) {
	                setValue(deck.LoadSelectedTrack, 1);
	              }
	            }, function () {
	              return setValue(deck.LoadSelectedTrack, 1);
	            }, function () {
	              return setValue(deck.eject, 1);
	            });
	          });
	        }
	      }
	    }
	  };
	};
	var makeLoad = make$d;

	const SMALL_SAMPLES = 125;
	const make$c = function (_, gridPosition, deck) {
	  const map = {
	    in: [deck.loop_in, deck.loop_start_position],
	    out: [deck.loop_out, deck.loop_end_position]
	  };
	  const onMidi = function (dir) {
	    return function ({
	      context: {
	        modifier
	      }
	    }) {
	      return onAttack(function (_) {
	        modes(modifier.getState(), function () {
	          setValue(map[dir][0], 1);
	          setValue(map[dir][0], 0);
	        }, function () {
	          const ctrl = map[dir][1];
	          setValue(ctrl, getValue(ctrl) - SMALL_SAMPLES);
	        }, function () {
	          const ctrl = map[dir][1];
	          setValue(ctrl, getValue(ctrl) + SMALL_SAMPLES);
	        });
	      });
	    };
	  };
	  return {
	    state: {},
	    bindings: {
	      in: {
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
	var makeLoopIo = make$c;

	const make$b = function (_, gridPosition, deck) {
	  const onMount = function (k) {
	    return function ({
	      context: {
	        device
	      },
	      bindings
	    }) {
	      return function () {
	        device.sendColor(bindings[k].control, device.colors.lo_yellow);
	      };
	    };
	  };
	  const onMidi = function (k) {
	    return function (_) {
	      return onAttack(function (_) {
	        return setValue(deck[`loop_${k}`], 1);
	      });
	    };
	  };
	  return {
	    state: {},
	    bindings: {
	      halve: {
	        type: 'button',
	        target: gridPosition,
	        mount: onMount('halve'),
	        midi: onMidi('halve')
	      },
	      double: {
	        type: 'button',
	        target: [gridPosition[0] + 1, gridPosition[1]],
	        mount: onMount('double'),
	        midi: onMidi('double')
	      }
	    }
	  };
	};
	var makeLoopMultiply = make$b;

	const make$a = function ({
	  jumps,
	  vertical = false
	}, gridPosition, deck) {
	  const bindings = {};
	  const onMidi = function (k, j, d) {
	    return function ({
	      context: {
	        modifier,
	        device
	      },
	      bindings,
	      state
	    }) {
	      return retainAttackMode(modifier, function (mode, {
	        value
	      }) {
	        modes(mode, function () {
	          if (!state.mode) {
	            if (value) {
	              setValue(deck.loop_move, j[state.set] * d);
	            }
	          } else {
	            if (value) {
	              const currentJump = j[state.set] * d;
	              setValue(deck.loop_move, currentJump);
	              if (state.pressing != null) {
	                device.sendColor(bindings[state.pressing].control, device.colors[`lo_${state.color[state.set]}`]);
	              }
	              device.sendColor(bindings[k].control, device.colors[`hi_${state.color[state.set]}`]);
	              state.pressing = k;
	              state.diff = state.diff + currentJump;
	            } else {
	              if (state.pressing === k) {
	                device.sendColor(bindings[k].control, device.colors[`lo_${state.color[state.set]}`]);
	                state.pressing = null;
	                setValue(deck.loop_move, -state.diff);
	                state.diff = 0;
	              }
	            }
	          }
	        }, function () {
	          if (value) {
	            state.set = posMod(state.set + 1, 2);
	            const prefix = state.mode ? 'lo' : 'hi';
	            for (let b = 0; b < spec.length; ++b) {
	              device.sendColor(bindings[b].control, device.colors[`${prefix}_${state.color[state.set]}`]);
	            }
	          }
	        }, function () {
	          if (value) {
	            state.mode = !state.mode;
	            const prefix = state.mode ? 'lo' : 'hi';
	            for (let b = 0; b < spec.length; ++b) {
	              device.sendColor(bindings[b].control, device.colors[`${prefix}_${state.color[state.set]}`]);
	            }
	          }
	        });
	      });
	    };
	  };
	  const onMount = function (k) {
	    return function ({
	      context: {
	        device
	      },
	      bindings,
	      state
	    }) {
	      return function () {
	        const prefix = state.mode ? 'lo' : 'hi';
	        device.sendColor(bindings[k].control, device.colors[`${prefix}_${state.color[state.set]}`]);
	      };
	    };
	  };
	  const spec = _flatMapInstanceProperty(jumps).call(jumps, function (j) {
	    return [[j, 1], [j, -1]];
	  });
	  spec.forEach(function ([jump, dir], i) {
	    bindings[i] = {
	      type: 'button',
	      target: vertical ? [gridPosition[0] + i % 2, gridPosition[1] + ~~(i / 2)] : [gridPosition[0] + ~~(i / 2), gridPosition[1] + i % 2],
	      midi: onMidi(i, jump, dir),
	      mount: onMount(i)
	    };
	  });
	  return {
	    bindings,
	    state: {
	      mode: false,
	      pressing: null,
	      diff: 0,
	      set: 0,
	      color: ['green', 'red']
	    }
	  };
	};
	var makeLoopjump = make$a;

	const make$9 = function ({
	  amount
	}, button, deck) {
	  const onMidi = function (dir) {
	    return function ({
	      context: {
	        modifier
	      }
	    }) {
	      return onAttack(function (_) {
	        return modes(modifier.getState(), function () {
	          return setValue(deck.loop_move, dir * amount);
	        });
	      });
	    };
	  };
	  return {
	    state: {},
	    bindings: {
	      back: {
	        type: 'button',
	        target: button,
	        midi: onMidi(-1),
	        mount: function ({
	          context: {
	            device
	          },
	          bindings
	        }) {
	          return function () {
	            device.sendColor(bindings.back.control, device.colors.hi_yellow);
	          };
	        }
	      },
	      forth: {
	        type: 'button',
	        target: [button[0] + 1, button[1]],
	        midi: onMidi(1),
	        mount: function ({
	          context: {
	            device
	          },
	          bindings
	        }) {
	          return function () {
	            device.sendColor(bindings.forth.control, device.colors.hi_yellow);
	          };
	        }
	      }
	    }
	  };
	};
	var makeLoopjumpSmall = make$9;

	const make$8 = function (_, gridPosition, deck) {
	  const rateEpsilon = 1e-3;
	  const getDirection = function (rate) {
	    if (rate < -rateEpsilon) {
	      return 'up';
	    } else if (rate > rateEpsilon) {
	      return 'down';
	    } else {
	      return '';
	    }
	  };
	  const onNudgeMidi = function (dir) {
	    return function ({
	      context: {
	        modifier,
	        device
	      },
	      bindings,
	      state
	    }) {
	      return retainAttackMode(modifier, function (mode, {
	        value
	      }) {
	        if (value) {
	          state[dir] = true;
	          if (state.down && state.up) {
	            setValue(deck.rate, 0);
	          } else {
	            modes(mode, function () {
	              device.sendColor(bindings[dir].control, device.colors.hi_yellow);
	              setValue(deck[`rate_temp_${dir}`], 1);
	            }, function () {
	              device.sendColor(bindings[dir].control, device.colors.hi_red);
	              setValue(deck[`rate_perm_${dir}`], 1);
	            }, function () {
	              device.sendColor(bindings[dir].control, device.colors.lo_yellow);
	              setValue(deck[`rate_temp_${dir}_small`], 1);
	            }, function () {
	              device.sendColor(bindings[dir].control, device.colors.lo_red);
	              setValue(deck[`rate_perm_${dir}_small`], 1);
	            });
	          }
	        } else {
	          state[dir] = false;
	          if (getDirection(getValue(bindings.rate.control)) === dir) {
	            device.sendColor(bindings[dir].control, device.colors.lo_orange);
	          } else {
	            device.clearColor(bindings[dir].control);
	          }
	          modes(mode, function () {
	            return setValue(deck[`rate_temp_${dir}`], 0);
	          }, undefined, function () {
	            return setValue(deck[`rate_temp_${dir}_small`], 0);
	          });
	        }
	      });
	    };
	  };
	  const onRate = function ({
	    context: {
	      device
	    },
	    bindings,
	    state
	  }) {
	    return function ({
	      value
	    }) {
	      let up = device.colors.black;
	      let down = device.colors.black;
	      const rate = getDirection(value);
	      if (rate === 'down') {
	        down = device.colors.lo_orange;
	      } else if (rate === 'up') {
	        up = device.colors.lo_orange;
	      }
	      if (!state.down) {
	        device.sendColor(bindings.down.control, down);
	      }
	      if (!state.up) {
	        device.sendColor(bindings.up.control, up);
	      }
	    };
	  };
	  return {
	    bindings: {
	      down: {
	        type: 'button',
	        target: gridPosition,
	        midi: onNudgeMidi('down')
	      },
	      up: {
	        type: 'button',
	        target: [gridPosition[0] + 1, gridPosition[1]],
	        midi: onNudgeMidi('up')
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
	var makeNudge = make$8;

	const make$7 = function (_, gridPosition, deck) {
	  return {
	    state: {},
	    bindings: {
	      pfl: {
	        type: 'control',
	        target: deck.pfl,
	        update: function ({
	          context: {
	            device
	          },
	          bindings
	        }) {
	          return function ({
	            value
	          }) {
	            return value ? device.sendColor(bindings.button.control, device.colors.hi_green) : device.clearColor(bindings.button.control);
	          };
	        }
	      },
	      button: {
	        type: 'button',
	        target: gridPosition,
	        midi: function ({
	          context: {
	            modifier
	          },
	          bindings
	        }) {
	          return onAttack(function (_) {
	            return modes(modifier.getState(), function () {
	              return setValue(bindings.pfl.control, Number(!getValue(bindings.pfl.control)));
	            });
	          });
	        }
	      }
	    }
	  };
	};
	var makePfl = make$7;

	const make$6 = function (_, gridPosition, deck) {
	  return {
	    state: {},
	    bindings: {
	      playIndicator: {
	        type: 'control',
	        target: deck.play_indicator,
	        update: function ({
	          bindings,
	          context: {
	            device
	          }
	        }) {
	          return function ({
	            value
	          }) {
	            if (value) {
	              device.sendColor(bindings.play.control, device.colors.hi_red);
	            } else if (!value) {
	              device.clearColor(bindings.play.control);
	            }
	          };
	        }
	      },
	      play: {
	        type: 'button',
	        target: gridPosition,
	        midi: function ({
	          context: {
	            modifier
	          }
	        }) {
	          return onAttack(function () {
	            modes(modifier.getState(), function () {
	              return setValue(deck.play, Number(!getValue(deck.play)));
	            }, function () {
	              return setValue(deck.start_play, 1);
	            }, function () {
	              return setValue(deck.start_stop, 1);
	            });
	          });
	        }
	      }
	    }
	  };
	};
	var makePlay = make$6;

	const make$5 = function (_, gridPosition, deck) {
	  return {
	    state: {},
	    bindings: {
	      quantize: {
	        type: 'control',
	        target: deck.quantize,
	        update: function ({
	          bindings,
	          context: {
	            device
	          }
	        }) {
	          return function ({
	            value
	          }) {
	            return value ? device.sendColor(bindings.button.control, device.colors.hi_orange) : device.clearColor(bindings.button.control);
	          };
	        }
	      },
	      button: {
	        type: 'button',
	        target: gridPosition,
	        midi: function ({
	          bindings,
	          context: {
	            modifier
	          }
	        }) {
	          return onAttack(function () {
	            return modes(modifier.getState(), function () {
	              return setValue(bindings.quantize.control, Number(!getValue(bindings.quantize.control)));
	            });
	          });
	        }
	      }
	    }
	  };
	};
	var makeQuantize = make$5;

	const make$4 = function (_, gridPosition, deck) {
	  return {
	    state: {},
	    bindings: {
	      button: {
	        type: 'button',
	        target: gridPosition,
	        midi: function ({
	          context: {
	            modifier
	          }
	        }) {
	          return onAttack(function () {
	            modes(modifier.getState(), function () {
	              return setValue(deck.reloop_exit, 1);
	            }, function () {
	              return setValue(deck.reloop_andstop, 1);
	            });
	          });
	        }
	      },
	      control: {
	        type: 'control',
	        target: deck.loop_enabled,
	        update: function ({
	          context: {
	            device
	          },
	          bindings
	        }) {
	          return function ({
	            value
	          }) {
	            if (value) {
	              device.sendColor(bindings.button.control, device.colors.hi_green);
	            } else {
	              device.sendColor(bindings.button.control, device.colors.lo_green);
	            }
	          };
	        }
	      }
	    }
	  };
	};
	var makeReloop = make$4;

	const make$3 = function (_, gridPosition, deck) {
	  const onMidi = function ({
	    bindings,
	    state,
	    context: {
	      modifier,
	      device
	    }
	  }) {
	    return retainAttackMode(modifier, function (mode, {
	      value
	    }) {
	      modes(mode, function () {
	        if (value) {
	          setValue(bindings.control.control, Number(!getValue(bindings.control.control)));
	        } else {
	          if (state.mode) {
	            setValue(bindings.control.control, Number(!getValue(bindings.control.control)));
	          }
	        }
	      }, function () {
	        if (value) {
	          state.mode = !state.mode;
	          const color = state.mode ? 'orange' : 'red';
	          device.sendColor(bindings.button.control, device.colors[`lo_${color}`]);
	        }
	      });
	    });
	  };
	  return {
	    bindings: {
	      control: {
	        type: 'control',
	        target: deck.slip_enabled,
	        update: function ({
	          bindings,
	          state,
	          context: {
	            device
	          }
	        }) {
	          return function ({
	            value
	          }) {
	            const color = state.mode ? 'orange' : 'red';
	            if (value) {
	              device.sendColor(bindings.button.control, device.colors[`hi_${color}`]);
	            } else {
	              device.sendColor(bindings.button.control, device.colors[`lo_${color}`]);
	            }
	          };
	        }
	      },
	      button: {
	        type: 'button',
	        target: gridPosition,
	        midi: onMidi,
	        mount: function ({
	          bindings,
	          state,
	          context: {
	            device
	          }
	        }) {
	          return function () {
	            const color = state.mode ? 'orange' : 'red';
	            device.sendColor(bindings.button.control, device.colors[`lo_${color}`]);
	          };
	        }
	      }
	    },
	    state: {
	      mode: true
	    }
	  };
	};
	var makeSlip = make$3;

	const make$2 = function (_, gridPosition, deck) {
	  return {
	    state: {},
	    bindings: {
	      sync: {
	        type: 'button',
	        target: gridPosition,
	        midi: function ({
	          bindings,
	          context: {
	            modifier
	          }
	        }) {
	          return onAttack(function () {
	            modes(modifier.getState(), function () {
	              if (getValue(bindings.syncMode.control)) {
	                setValue(deck.sync_enabled, 0);
	              } else {
	                setValue(deck.sync_enabled, 1);
	              }
	            }, function () {
	              if (getValue(bindings.syncMode.control) === 2) {
	                setValue(deck.sync_master, 0);
	              } else {
	                setValue(deck.sync_master, 1);
	              }
	            });
	          });
	        }
	      },
	      syncMode: {
	        type: 'control',
	        target: deck.sync_mode,
	        update: function ({
	          bindings,
	          context: {
	            device
	          }
	        }) {
	          return function ({
	            value
	          }) {
	            if (value === 0) {
	              device.clearColor(bindings.sync.control);
	            } else if (value === 1) {
	              device.sendColor(bindings.sync.control, device.colors.hi_orange);
	            } else if (value === 2) {
	              device.sendColor(bindings.sync.control, device.colors.hi_red);
	            }
	          };
	        }
	      }
	    }
	  };
	};
	var makeSync = make$2;

	class Bpm extends eventemitter3Exports {
	  constructor(max) {
	    super();
	    _defineProperty(this, "tapTime", void 0);
	    _defineProperty(this, "taps", void 0);
	    _defineProperty(this, "max", void 0);
	    if (max == null) {
	      max = 8;
	    }
	    this.tapTime = 0;
	    this.taps = [];
	    this.max = max;
	  }
	  reset() {
	    this.taps = [];
	  }
	  tap() {
	    const now = Date.now();
	    const tapDelta = now - this.tapTime;
	    this.tapTime = now;
	    if (tapDelta > 2000) {
	      // reset if longer than two seconds between taps
	      this.taps = [];
	    } else {
	      this.taps.push(60000 / tapDelta);
	      if (this.taps.length > this.max) this.taps.shift(); // Keep the last n samples for averaging
	      let sum = 0;
	      this.taps.forEach(function (v) {
	        sum += v;
	      });
	      const avg = sum / this.taps.length;
	      this.emit('tap', avg);
	    }
	  }
	}

	const make$1 = function (_, gridPosition, deck) {
	  const tempoBpm = new Bpm();
	  tempoBpm.on('tap', function (avg) {
	    setValue(deck.bpm, avg);
	  });
	  return {
	    state: {},
	    bindings: {
	      tap: {
	        type: 'button',
	        target: gridPosition,
	        midi: function ({
	          context: {
	            modifier
	          }
	        }) {
	          return onAttack(function () {
	            modes(modifier.getState(), function () {
	              return tempoBpm.tap();
	            }, function () {
	              return setValue(deck.bpm_tap, 1);
	            }, function () {
	              return setValue(deck.beats_translate_curpos, 1);
	            }, function () {
	              return setValue(deck.beats_translate_match_alignment, 1);
	            });
	          });
	        }
	      },
	      beat: {
	        type: 'control',
	        target: deck.beat_active,
	        update: function ({
	          context: {
	            device
	          },
	          bindings
	        }) {
	          return function ({
	            value
	          }) {
	            if (value) {
	              device.sendColor(bindings.tap.control, device.colors.hi_red);
	            } else {
	              device.clearColor(bindings.tap.control);
	            }
	          };
	        }
	      }
	    }
	  };
	};
	var makeTap = make$1;

	const index$1 = {
	  beatjump: makeBeatjump,
	  beatloop: makeBeatloop,
	  cue: makeCue,
	  grid: makeGrid,
	  hotcue: makeHotcue,
	  key: makeKey,
	  keyshift: makeKeyshift,
	  load: makeLoad,
	  loopIo: makeLoopIo,
	  loopMultiply: makeLoopMultiply,
	  loopjump: makeLoopjump,
	  loopjumpSmall: makeLoopjumpSmall,
	  nudge: makeNudge,
	  pfl: makePfl,
	  play: makePlay,
	  quantize: makeQuantize,
	  reloop: makeReloop,
	  slip: makeSlip,
	  sync: makeSync,
	  tap: makeTap
	};
	var makeControlTemplateIndex = index$1;

	const make = function (_, gridPosition, sampler, theme) {
	  const onStateChanged = function (state, device, bindings) {
	    const color = state.color == null ? theme.fallbackTrackColor : state.color;
	    if (!state.loaded) {
	      device.clearColor(bindings.button.control);
	    } else if (!state.playing) {
	      if (device.supportsRGBColors) {
	        device.sendRGBColor(bindings.button.control, color.map(function (x) {
	          return ~~(x / 4);
	        }));
	      } else {
	        device.sendColor(bindings.button.control, device.colors.lo_red);
	      }
	    } else {
	      if (device.supportsRGBColors) {
	        device.sendRGBColor(bindings.button.control, color);
	      } else {
	        device.sendColor(bindings.button.control, device.colors.hi_red);
	      }
	    }
	  };
	  return {
	    state: {
	      playing: false,
	      loaded: false,
	      color: null
	    },
	    bindings: {
	      button: {
	        type: 'button',
	        target: gridPosition,
	        midi: function ({
	          context: {
	            modifier
	          },
	          state
	        }) {
	          return function ({
	            value
	          }) {
	            if (value) {
	              modes(modifier.getState(), function () {
	                if (!state.loaded) {
	                  setValue(sampler.LoadSelectedTrack, 1);
	                } else {
	                  setValue(sampler.cue_gotoandplay, 1);
	                }
	              }, function () {
	                if (state.playing) {
	                  setValue(sampler.stop, 1);
	                } else if (state.loaded) {
	                  setValue(sampler.eject, 1);
	                }
	              });
	            }
	          };
	        }
	      },
	      playing: {
	        type: 'control',
	        target: sampler.play_latched,
	        update: function ({
	          context: {
	            device
	          },
	          bindings,
	          state
	        }) {
	          return function ({
	            value
	          }) {
	            state.playing = !!value;
	            onStateChanged(state, device, bindings);
	          };
	        }
	      },
	      loaded: {
	        type: 'control',
	        target: sampler.track_loaded,
	        update: function ({
	          context: {
	            device
	          },
	          bindings,
	          state
	        }) {
	          return function ({
	            value
	          }) {
	            state.loaded = !!value;
	            onStateChanged(state, device, bindings);
	          };
	        }
	      },
	      colorChanged: {
	        type: 'control',
	        target: sampler.track_color,
	        update: function ({
	          context: {
	            device
	          },
	          bindings,
	          state
	        }) {
	          return function ({
	            value
	          }) {
	            state.color = parseRGBColor(value);
	            onStateChanged(state, device, bindings);
	          };
	        }
	      }
	    }
	  };
	};
	var makeSamplerPad = make;

	const controlListeners = ['update', 'mount', 'unmount'];
	const midiListeners = ['midi', 'mount', 'unmount'];
	const nameOf = function (x, y) {
	  return `${7 - y},${x}`;
	};
	class Control extends Component {
	  constructor(ctx, controlTemplate) {
	    super();
	    _defineProperty(this, "bindings", void 0);
	    _defineProperty(this, "bindingTemplates", void 0);
	    _defineProperty(this, "state", void 0);
	    _defineProperty(this, "context", void 0);
	    const bindings = {};
	    for (const k in controlTemplate.bindings) {
	      const bt = controlTemplate.bindings[k];
	      bindings[k] = bt.type == 'control' ? new ControlComponent(bt.target) : new MidiComponent(ctx.device, ctx.device.controls[nameOf(...bt.target)]);
	    }
	    this.bindingTemplates = controlTemplate.bindings;
	    this.bindings = bindings;
	    this.state = controlTemplate.state;
	    this.context = ctx;
	  }
	  onMount() {
	    var _this = this;
	    super.onMount();
	    Object.keys(this.bindings).forEach(function (k) {
	      const b = _this.bindings[k];
	      if (b instanceof ControlComponent) {
	        const bt = _this.bindingTemplates[k];
	        controlListeners.forEach(function (event) {
	          const listener = bt[event];
	          if (listener != null) {
	            b.addListener(event, listener(_this));
	          }
	        });
	      } else {
	        const bt = _this.bindingTemplates[k];
	        midiListeners.forEach(function (event) {
	          const listener = bt[event];
	          if (listener) {
	            b.addListener(event, listener(_this));
	          }
	        });
	        // add a default handler to clear the button LED
	        b.addListener('unmount', function () {
	          _this.context.device.clearColor(b.control);
	        });
	      }
	    });
	    Object.values(this.bindings).forEach(function (b) {
	      return b.mount();
	    });
	  }
	  onUnmount() {
	    const bs = Object.values(this.bindings);
	    bs.forEach(function (b) {
	      return b.unmount();
	    });
	    bs.forEach(function (b) {
	      return b.removeAllListeners();
	    });
	    super.onUnmount();
	  }
	}
	const isDeckPresetConf = function (p) {
	  return 'deck' in p;
	};
	class Preset extends Component {
	  constructor(ctx, presetTemplate) {
	    super();
	    _defineProperty(this, "controls", void 0);
	    this.controls = presetTemplate.controls.map(function (c) {
	      return new Control(ctx, c);
	    });
	  }
	  onMount() {
	    super.onMount();
	    for (const control of this.controls) {
	      control.mount();
	    }
	  }
	  onUnmount() {
	    for (const control of this.controls) {
	      control.unmount();
	    }
	    super.onUnmount();
	  }
	}
	const tr = function (a, b) {
	  return [a[0] + b[0], a[1] + b[1]];
	};
	const makeDeckPresetTemplate = function (conf, gridPosition, deck, theme) {
	  return {
	    controls: conf.deck.map(function ({
	      pos,
	      control: {
	        type,
	        params
	      }
	    }) {
	      return makeControlTemplateIndex[type](params, tr(gridPosition, pos), deck, theme);
	    })
	  };
	};
	const makeSamplerPalettePresetTemplate = function ({
	  samplerPalette: {
	    n,
	    offset,
	    rows
	  }
	}, gridPosition, _startingChannel, theme) {
	  return {
	    controls: array(map(function (i) {
	      const dy = 7 - ~~(i / rows);
	      const dx = i % rows;
	      return makeSamplerPad({}, tr(gridPosition, [dx, dy]), root.samplers[i + offset], theme);
	    }, range(Math.min(n, getValue(root.master.num_samplers)))))
	  };
	};
	const makePresetTemplate = function (conf, gridPosition, channel, theme) {
	  if (isDeckPresetConf(conf)) {
	    return makeDeckPresetTemplate(conf, gridPosition, root.channels[channel], theme);
	  } else {
	    return makeSamplerPalettePresetTemplate(conf, gridPosition, channel, theme);
	  }
	};

	const longInterval = 240;
	const mediumInterval = 120;
	const shortInterval = 60;
	const minInterval = 30;
	const autoscrolled = function (binding) {
	  let started = null;
	  let interval = null;
	  let timer = null;
	  binding.on('midi', function (data) {
	    // unsafe cast: timer should be initialized at this point
	    timer = timer;
	    if (data.value) {
	      interval = longInterval;
	      started = timer.start(interval);
	    } else {
	      timer.end();
	    }
	  });
	  binding.on('mount', function () {
	    timer = new Timer(function () {
	      binding.emit('scroll');
	      // unsafe cast: interval should be initialized at this point
	      interval = interval;
	      // unsafe cast: timer should be initialized at this point
	      timer = timer;
	      // unsafe cast: started should be initialized at this point
	      started = started;
	      if (interval > minInterval) {
	        const current = Date.now();
	        if (interval === longInterval && current - started > 1500) {
	          interval = mediumInterval;
	          timer.restart(interval);
	        } else if (interval === mediumInterval && current - started > 3000) {
	          interval = shortInterval;
	          timer.restart(interval);
	        } else if (interval === shortInterval && current - started > 6000) {
	          interval = minInterval;
	          timer.restart(interval);
	        }
	      }
	    });
	  });
	  binding.on('unmount', function () {
	    return timer.unmount();
	  });
	  return binding;
	};
	class PlaylistSidebar extends Component {
	  constructor(device) {
	    super();
	    _defineProperty(this, "buttons", void 0);
	    _defineProperty(this, "controls", void 0);
	    const onScroll = function (control) {
	      return function () {
	        setValue(control, 1);
	      };
	    };
	    const onMidi = function (control, color = device.colors.hi_yellow) {
	      return function (message) {
	        if (message.value) {
	          setValue(control, 1);
	          device.sendColor(message.control, device.colors.hi_red);
	        } else {
	          device.sendColor(message.control, color);
	        }
	      };
	    };
	    const onMount = function (color = device.colors.hi_yellow) {
	      return function (button) {
	        device.sendColor(button.control, color);
	      };
	    };
	    const onUnmount = function (button) {
	      device.clearColor(button.control);
	    };
	    const btns = [new MidiComponent(device, device.controls.vol), new MidiComponent(device, device.controls.pan), new MidiComponent(device, device.controls.snda), new MidiComponent(device, device.controls.sndb), new MidiComponent(device, device.controls.stop), new MidiComponent(device, device.controls.trkon)];
	    const controls = [new ControlComponent(masterControlDef.maximize_library)];
	    const prevPlaylist = autoscrolled(btns[0]);
	    const nextPlaylist = autoscrolled(btns[1]);
	    const toggleItem = btns[2];
	    const prevTrack = autoscrolled(btns[3]);
	    const nextTrack = autoscrolled(btns[4]);
	    const toggleLibrary = btns[5];
	    const toggleLibraryControl = controls[0];
	    prevPlaylist.on('scroll', onScroll(playListControlDef.SelectPrevPlaylist));
	    prevPlaylist.on('midi', onMidi(playListControlDef.SelectPrevPlaylist));
	    prevPlaylist.on('mount', onMount());
	    prevPlaylist.on('unmount', onUnmount);
	    nextPlaylist.on('scroll', onScroll(playListControlDef.SelectNextPlaylist));
	    nextPlaylist.on('midi', onMidi(playListControlDef.SelectNextPlaylist));
	    nextPlaylist.on('mount', onMount());
	    nextPlaylist.on('unmount', onUnmount);
	    prevTrack.on('scroll', onScroll(playListControlDef.SelectPrevTrack));
	    prevTrack.on('midi', onMidi(playListControlDef.SelectPrevTrack));
	    prevTrack.on('mount', onMount());
	    prevTrack.on('unmount', onUnmount);
	    nextTrack.on('scroll', onScroll(playListControlDef.SelectNextTrack));
	    nextTrack.on('midi', onMidi(playListControlDef.SelectNextTrack));
	    nextTrack.on('mount', onMount());
	    nextTrack.on('unmount', onUnmount);
	    toggleItem.on('midi', onMidi(playListControlDef.ToggleSelectedSidebarItem, device.colors.hi_green));
	    toggleItem.on('mount', onMount(device.colors.hi_green));
	    toggleItem.on('unmount', onUnmount);
	    toggleLibraryControl.on('update', function (m) {
	      if (m.value) {
	        device.sendColor(toggleLibrary.control, device.colors.hi_red);
	      } else {
	        device.sendColor(toggleLibrary.control, device.colors.hi_green);
	      }
	    });
	    toggleLibrary.on('midi', function (m) {
	      if (m.value) {
	        const t = getValue(masterControlDef.maximize_library);
	        setValue(masterControlDef.maximize_library, 1 - t);
	      }
	    });
	    toggleLibrary.on('unmount', onUnmount);
	    this.buttons = btns;
	    this.controls = controls;
	  }
	  onMount() {
	    super.onMount();
	    this.buttons.forEach(function (button) {
	      return button.mount();
	    });
	    this.controls.forEach(function (control) {
	      return control.mount();
	    });
	  }
	  onUnmount() {
	    this.controls.forEach(function (control) {
	      return control.unmount();
	    });
	    this.buttons.forEach(function (button) {
	      return button.unmount();
	    });
	    super.onUnmount();
	  }
	}

	const onMidi = function (layout, channel, modifier) {
	  return retainAttackMode(modifier, function (mode, {
	    value
	  }) {
	    const selected = layout.chord;
	    modes(mode, function () {
	      if (!value && selected.length) {
	        const diff = reorganize(layout.getLayout(), selected);
	        layout.updateLayout(diff);
	        layout.removeChord();
	      } else if (value) {
	        layout.addToChord(channel);
	      }
	    }, function () {
	      if (value) {
	        if (selected.length) layout.removeChord();
	        const diff = cycle(channel, layout.getLayout(), 1, layout.presets);
	        layout.updateLayout(diff);
	      }
	    }, function () {
	      if (value) {
	        if (selected.length) layout.removeChord();
	        const diff = cycle(channel, layout.getLayout(), -1, layout.presets);
	        layout.updateLayout(diff);
	      }
	    });
	  });
	};
	const buttons = ['up', 'down', 'left', 'right', 'session', 'user1', 'user2', 'mixer'];
	class App extends Component {
	  // state variables

	  constructor(device, conf) {
	    var _this;
	    super();
	    _this = this;
	    _defineProperty(this, "conf", void 0);
	    _defineProperty(this, "bindings", void 0);
	    _defineProperty(this, "modifier", void 0);
	    _defineProperty(this, "presets", void 0);
	    _defineProperty(this, "playlistSidebar", void 0);
	    _defineProperty(this, "chord", void 0);
	    _defineProperty(this, "layout", void 0);
	    _defineProperty(this, "mountedPresets", void 0);
	    _defineProperty(this, "device", void 0);
	    this.conf = conf;
	    this.device = device;
	    this.modifier = new ModifierSidebar(device);
	    this.playlistSidebar = new PlaylistSidebar(device);
	    this.bindings = buttons.map(function (v, i) {
	      const binding = new MidiComponent(_this.device, _this.device.controls[v]);
	      return [binding, onMidi(_this, i, _this.modifier)];
	    });
	    this.presets = cycled(conf.presets);
	    this.chord = [];
	    this.layout = {};
	    this.mountedPresets = {};
	  }
	  getLayout() {
	    const res = [];
	    for (const k in this.layout) {
	      res.push(this.layout[k]);
	    }
	    return res;
	  }
	  updateLayout(diff) {
	    var _this2 = this;
	    const removedChannels = diff[0].map(function (block) {
	      return block.channel;
	    });
	    removedChannels.forEach(function (ch) {
	      delete _this2.layout[ch];
	      _this2.device.clearColor(_this2.bindings[ch][0].control);
	      _this2.mountedPresets[ch].unmount();
	    });
	    const addedBlocks = diff[1];
	    addedBlocks.forEach(function (block) {
	      _this2.layout[block.channel] = block;
	      if (block.index) {
	        _this2.device.sendColor(_this2.bindings[block.channel][0].control, _this2.device.colors.hi_orange);
	      } else {
	        _this2.device.sendColor(_this2.bindings[block.channel][0].control, _this2.device.colors.hi_green);
	      }
	      const ctx = {
	        modifier: _this2.modifier,
	        device: _this2.device
	      };
	      const presetTemplate = makePresetTemplate(_this2.presets[block.size][block.index], block.offset, block.channel, _this2.conf.theme);
	      const preset = new Preset(ctx, presetTemplate);
	      _this2.mountedPresets[block.channel] = preset;
	      _this2.mountedPresets[block.channel].mount();
	    });
	  }
	  removeChord() {
	    var _this3 = this;
	    const layout = this.getLayout();
	    this.chord.forEach(function (ch) {
	      const found = layout.findIndex(function (b) {
	        return b.channel === ch;
	      });
	      if (found === -1) {
	        _this3.device.clearColor(_this3.bindings[ch][0].control);
	      } else {
	        const block = layout[found];
	        if (block.index) {
	          _this3.device.sendColor(_this3.bindings[ch][0].control, _this3.device.colors.hi_orange);
	        } else {
	          _this3.device.sendColor(_this3.bindings[ch][0].control, _this3.device.colors.hi_green);
	        }
	      }
	      _this3.chord = [];
	    });
	  }
	  addToChord(channel) {
	    if (this.chord.length === 4) {
	      const rem = this.chord.shift();
	      const found = this.getLayout().findIndex(function (b) {
	        return b.channel === rem;
	      });
	      if (found === -1) {
	        this.device.clearColor(this.bindings[rem][0].control);
	      } else {
	        const layout = this.layout[String(found)];
	        if (layout.index) {
	          this.device.sendColor(this.bindings[rem][0].control, this.device.colors.hi_orange);
	        } else {
	          this.device.sendColor(this.bindings[rem][0].control, this.device.colors.hi_green);
	        }
	      }
	    }
	    this.chord.push(channel);
	    this.device.sendColor(this.bindings[channel][0].control, this.device.colors.hi_red);
	  }
	  onMount() {
	    this.modifier.mount();
	    this.playlistSidebar.mount();
	    this.bindings.forEach(function ([binding, midi]) {
	      binding.mount();
	      binding.on('midi', midi);
	    });
	    const diff = reorganize([], this.conf.initialSelection);
	    this.updateLayout(diff);
	  }
	  onUnmount() {
	    const diff = reorganize(this.getLayout(), []);
	    this.updateLayout(diff);
	    this.bindings.forEach(function ([binding, midi]) {
	      binding.removeListener('midi', midi);
	      binding.unmount();
	    });
	    this.playlistSidebar.unmount();
	    this.modifier.unmount();
	  }
	}
	const offsets = [[0, 0], [4, 0], [0, 4], [4, 4]];
	const cycled = function (presets) {
	  return {
	    grande: [...presets.grande, ...presets.tall, ...presets.short],
	    tall: [...presets.tall, ...presets.short],
	    short: presets.short
	  };
	};
	const blockEquals = function (a, b) {
	  return a.offset === b.offset && a.size === b.size && a.channel === b.channel && a.index === b.index;
	};
	const reorganize = function (current, selectedChannels) {
	  const next = function (chs) {
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
	          offset: offsets[0],
	          size: 'short',
	          channel: chs[0],
	          index: 0
	        }, {
	          offset: offsets[1],
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
	    const [neg, pos] = diff;
	    const matched = pos.findIndex(function (b) {
	      return blockEquals(block, b);
	    });
	    return matched === -1 ? [neg.concat([block]), pos] : [neg, pos.slice(0, matched).concat(pos.slice(matched + 1, pos.length))];
	  }, [[], next]);
	};
	const cycle = function (channel, current, dir, lengths) {
	  const matched = current.findIndex(function (block) {
	    return block.channel === channel;
	  });
	  if (matched === -1) {
	    return [[], []];
	  }
	  const nextIndex = posMod(current[matched].index + dir, lengths[current[matched].size].length);
	  if (nextIndex === current[matched].index) {
	    return [[], []];
	  }
	  return [[current[matched]], [Object.assign({}, current[matched], {
	    index: nextIndex
	  })]];
	};

	const useDevice = function (device) {
	  const app = new App(device, config);
	  device.addListener('mount', app.mount.bind(app));
	  device.addListener('unmount', app.unmount.bind(app));
	  return device;
	};
	const convertControlDef = function (name, [status, midino]) {
	  return {
	    name,
	    status,
	    midino
	  };
	};

	var device = "Launchpad MK2";
	var manufacturer = "Novation";
	var global$1 = "NLMK2";
	var controls = {
		up: [
			176,
			104
		],
		down: [
			176,
			105
		],
		left: [
			176,
			106
		],
		right: [
			176,
			107
		],
		session: [
			176,
			108
		],
		user1: [
			176,
			109
		],
		user2: [
			176,
			110
		],
		mixer: [
			176,
			111
		],
		vol: [
			144,
			89
		],
		pan: [
			144,
			79
		],
		snda: [
			144,
			69
		],
		sndb: [
			144,
			59
		],
		stop: [
			144,
			49
		],
		trkon: [
			144,
			39
		],
		solo: [
			144,
			29
		],
		arm: [
			144,
			19
		],
		"0,0": [
			144,
			81
		],
		"0,1": [
			144,
			82
		],
		"0,2": [
			144,
			83
		],
		"0,3": [
			144,
			84
		],
		"0,4": [
			144,
			85
		],
		"0,5": [
			144,
			86
		],
		"0,6": [
			144,
			87
		],
		"0,7": [
			144,
			88
		],
		"1,0": [
			144,
			71
		],
		"1,1": [
			144,
			72
		],
		"1,2": [
			144,
			73
		],
		"1,3": [
			144,
			74
		],
		"1,4": [
			144,
			75
		],
		"1,5": [
			144,
			76
		],
		"1,6": [
			144,
			77
		],
		"1,7": [
			144,
			78
		],
		"2,0": [
			144,
			61
		],
		"2,1": [
			144,
			62
		],
		"2,2": [
			144,
			63
		],
		"2,3": [
			144,
			64
		],
		"2,4": [
			144,
			65
		],
		"2,5": [
			144,
			66
		],
		"2,6": [
			144,
			67
		],
		"2,7": [
			144,
			68
		],
		"3,0": [
			144,
			51
		],
		"3,1": [
			144,
			52
		],
		"3,2": [
			144,
			53
		],
		"3,3": [
			144,
			54
		],
		"3,4": [
			144,
			55
		],
		"3,5": [
			144,
			56
		],
		"3,6": [
			144,
			57
		],
		"3,7": [
			144,
			58
		],
		"4,0": [
			144,
			41
		],
		"4,1": [
			144,
			42
		],
		"4,2": [
			144,
			43
		],
		"4,3": [
			144,
			44
		],
		"4,4": [
			144,
			45
		],
		"4,5": [
			144,
			46
		],
		"4,6": [
			144,
			47
		],
		"4,7": [
			144,
			48
		],
		"5,0": [
			144,
			31
		],
		"5,1": [
			144,
			32
		],
		"5,2": [
			144,
			33
		],
		"5,3": [
			144,
			34
		],
		"5,4": [
			144,
			35
		],
		"5,5": [
			144,
			36
		],
		"5,6": [
			144,
			37
		],
		"5,7": [
			144,
			38
		],
		"6,0": [
			144,
			21
		],
		"6,1": [
			144,
			22
		],
		"6,2": [
			144,
			23
		],
		"6,3": [
			144,
			24
		],
		"6,4": [
			144,
			25
		],
		"6,5": [
			144,
			26
		],
		"6,6": [
			144,
			27
		],
		"6,7": [
			144,
			28
		],
		"7,0": [
			144,
			11
		],
		"7,1": [
			144,
			12
		],
		"7,2": [
			144,
			13
		],
		"7,3": [
			144,
			14
		],
		"7,4": [
			144,
			15
		],
		"7,5": [
			144,
			16
		],
		"7,6": [
			144,
			17
		],
		"7,7": [
			144,
			18
		]
	};
	var def = {
		device: device,
		manufacturer: manufacturer,
		global: global$1,
		controls: controls
	};

	const colors = {
	  black: 0,
	  lo_red: 7,
	  hi_red: 5,
	  lo_green: 19,
	  hi_green: 17,
	  lo_amber: 43,
	  hi_amber: 41,
	  hi_orange: 84,
	  lo_orange: 61,
	  hi_yellow: 13,
	  lo_yellow: 15,
	  lo_white: 15,
	  hi_white: 13
	};
	class LaunchpadMK2Device extends LaunchpadDevice {
	  constructor() {
	    super();
	    _defineProperty(this, "supportsRGBColors", void 0);
	    _defineProperty(this, "controls", void 0);
	    _defineProperty(this, "colors", void 0);
	    this.controls = _Object$fromEntries(Object.entries(def.controls).map(function ([k, v]) {
	      return [k, convertControlDef(k, v)];
	    }));
	    this.colors = colors;
	    this.supportsRGBColors = true;
	  }
	  onMount() {
	    super.onMount();
	  }
	  sendRGBColor(control, color) {
	    sendSysexMsg([240, 0, 32, 41, 2, 24, 11, control.midino, ...color.map(function (x) {
	      return ~~(x / 4);
	    }), 247]);
	  }
	}
	var index = useDevice(new LaunchpadMK2Device());

	return index;

})();
