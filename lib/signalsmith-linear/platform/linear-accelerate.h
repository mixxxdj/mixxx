//#define ACCELERATE_NEW_LAPACK

// If possible, only include vecLib, since JUCE has conflicts with vImage
#if defined(__has_include) && __has_include(<vecLib/vecLib.h>)
#	include <vecLib/vecLib.h>
#else
#	include <Accelerate/Accelerate.h>
#endif

#include <cstring> // std::memcpy
#include <iostream> // We log to stderr when an expression is unoptimised

namespace signalsmith { namespace linear {

template<class T>
static std::string typeName() {
	return typeid(T).name();
}
template<>
std::string typeName<float>() {
	return "float";
}
template<>
std::string typeName<double>() {
	return "double";
}
template<>
std::string typeName<std::complex<float>>() {
	return "complex<float>";
}
template<>
std::string typeName<std::complex<double>>() {
	return "complex<double>";
}
template<>
std::string typeName<const std::complex<float>>() {
	return "const complex<float>";
}
template<>
std::string typeName<const std::complex<double>>() {
	return "const complex<double>";
}
static size_t _basicFillWarningCounter = 1;
static void basicFillWarningReset() {
	// All warnings will print again
	++_basicFillWarningCounter;
}
template<class Expr>
static void basicFillWarning() {
	static size_t counter = 0;
	if (counter != _basicFillWarningCounter) {
		counter = _basicFillWarningCounter;
		std::cerr << "used basic .fill() for " << Expr::name() << " -> " << typeName<decltype(std::declval<Expr>().get(0))>() << "\n";
	}
}

template<>
struct LinearImpl<true> : public LinearImplBase<true> {
	using Base = LinearImplBase<true>;

	LinearImpl() : Base(this), cached(*this) {
		basicFillWarningReset();
	}

	template<class V>
	void reserve(size_t) {}
	
	template<>
	void reserve<float>(size_t size) {
		cached.reserveFloats(size*4);
	}
	template<>
	void reserve<double>(size_t size) {
		cached.reserveDoubles(size*4);
	}

	template<class Pointer, class Expr>
	void fill(Pointer pointer, Expr expr, size_t size) {
		fillExpr(pointer, expr, size);
	}

	template<class Pointer, class Expr>
	void fill(Pointer pointer, Expression<Expr> expr, size_t size) {
		return fillExpr(pointer, (Expr &)expr, size);
	};
	template<class Pointer, class Expr>
	void fill(Pointer pointer, WritableExpression<Expr> expr, size_t size) {
		return fillExpr(pointer, (Expr &)expr, size);
	};

private:
	// Most generic fill
	template<class Pointer, class Expr>
	void fillExpr(Pointer pointer, Expr expr, size_t size) {
		fillBasic(pointer, expr, size);
	}

	template<class Pointer, class Expr>
	void fillBasic(Pointer pointer, Expr expr, size_t size) {
		basicFillWarning<Expr>();
		for (size_t i = 0; i < size; ++i) {
			pointer[i] = expr.get(i);
		}
	}
	template<class V, class Expr>
	void fillBasic(SplitPointer<V> pointer, Expr expr, size_t size) {
		basicFillWarning<Expr>();
		using Complex = typename SplitPointer<V>::Complex;
		for (size_t i = 0; i < size; ++i) {
			Complex c = expr.get(i);
			pointer.real[i] = c.real();
			pointer.imag[i] = c.imag();
		}
	}
	
	template<typename V>
	void clear(V *v, size_t size) {
		for (size_t i = 0; i < size; ++i) v[i] = 0;
	}
	void clear(float *v, size_t size) {
		vDSP_vclr(v, 1, size);
	}
	void clear(double *v, size_t size) {
		vDSP_vclrD(v, 1, size);
	}
	// Filling a split-complex vector with real values won't hit the specialisations below, so we handle it here
	template<class Expr>
	ItemType<Expr, float, void> fillBasic(SplitPointer<float> pointer, Expr expr, size_t size) {
		fillExpr(pointer.real, expr, size);
		clear(pointer.imag, size);
	}
	template<class Expr>
	ItemType<Expr, double, void> fillBasic(SplitPointer<double> pointer, Expr expr, size_t size) {
		fillExpr(pointer.real, expr, size);
		clear(pointer.imag, size);
	}
	
	// Copying from existing pointer
	void fillExpr(RealPointer<float> pointer, expression::ReadableReal<float> expr, size_t size) {
		std::memcpy(pointer, expr.pointer, size*sizeof(float));
	}
	void fillExpr(RealPointer<float> pointer, WritableReal<float> expr, size_t size) {
		std::memcpy(pointer, expr.pointer, size*sizeof(float));
	}
	void fillExpr(RealPointer<double> pointer, expression::ReadableReal<double> expr, size_t size) {
		std::memcpy(pointer, expr.pointer, size*sizeof(double));
	}
	void fillExpr(RealPointer<double> pointer, WritableReal<double> expr, size_t size) {
		std::memcpy(pointer, expr.pointer, size*sizeof(double));
	}
	void fillExpr(ComplexPointer<float> pointer, expression::ReadableComplex<float> expr, size_t size) {
		std::memcpy(pointer, expr.pointer, size*sizeof(std::complex<float>));
	}
	void fillExpr(ComplexPointer<float> pointer, WritableComplex<float> expr, size_t size) {
		std::memcpy(pointer, expr.pointer, size*sizeof(std::complex<float>));
	}
	void fillExpr(ComplexPointer<double> pointer, expression::ReadableComplex<double> expr, size_t size) {
		std::memcpy(pointer, expr.pointer, size*sizeof(std::complex<double>));
	}
	void fillExpr(ComplexPointer<double> pointer, WritableComplex<double> expr, size_t size) {
		std::memcpy(pointer, expr.pointer, size*sizeof(std::complex<double>));
	}

	// Filling with a constant
	template<class V>
	void fillExpr(RealPointer<float> pointer, expression::ConstantExpr<V> expr, size_t size) {
		float v = expr.value;
		vDSP_vfill(&v, pointer, 1, size);
	}
	template<class V>
	void fillExpr(RealPointer<double> pointer, expression::ConstantExpr<V> expr, size_t size) {
		double v = expr.value;
		vDSP_vfillD(&v, pointer, 1, size);
	}
	template<class V>
	void fillExpr(ComplexPointer<float> pointer, expression::ConstantExpr<V> expr, size_t size) {
		std::complex<float> v = expr.value;
		auto vSplit = dspSplit(&v);
		auto pSplit = dspSplit(pointer);
		vDSP_zvfill(&vSplit, &pSplit, 2, size);
	}
	template<class V>
	void fillExpr(ComplexPointer<double> pointer, expression::ConstantExpr<V> expr, size_t size) {
		std::complex<double> v = expr.value;
		auto vSplit = dspSplit(&v);
		auto pSplit = dspSplit(pointer);
		vDSP_zvfillD(&vSplit, &pSplit, 2, size);
	}
	template<class V>
	void fillExpr(SplitPointer<float> pointer, expression::ConstantExpr<V> expr, size_t size) {
		std::complex<float> v = expr.value;
		float vr = v.real(), vi = v.imag();
		auto vSplit = dspSplit({&vr, &vi});
		auto pSplit = dspSplit(pointer);
		vDSP_zvfill(&vSplit, &pSplit, 1, size);
	}
	template<class V>
	void fillExpr(SplitPointer<double> pointer, expression::ConstantExpr<V> expr, size_t size) {
		std::complex<double> v = expr.value;
		double vr = v.real(), vi = v.imag();
		auto vSplit = dspSplit({&vr, &vi});
		auto pSplit = dspSplit(pointer);
		vDSP_zvfillD(&vSplit, &pSplit, 1, size);
	}

// Forwards .fillExpr() to .fillName(), but doesn't define that
#define SIGNALSMITH_AUDIO_LINEAR_OP1_R(Name) \
	template<class A> \
	void fillExpr(RealPointer<float> pointer, expression::Name<A> expr, size_t size) { \
		fill##Name(pointer, expr, size); \
	} \
	template<class A> \
	void fillExpr(RealPointer<double> pointer, expression::Name<A> expr, size_t size) { \
		fill##Name(pointer, expr, size); \
	} \
	template<class Expr> \
	void fill##Name(RealPointer<float> pointer, Expr expr, size_t size) { \
		fillBasic(pointer, expr, size); \
	} \
	template<class Expr> \
	void fill##Name(RealPointer<double> pointer, Expr expr, size_t size) { \
		fillBasic(pointer, expr, size); \
	}
#define SIGNALSMITH_AUDIO_LINEAR_OP1_C(Name) \
	template<class A> \
	void fillExpr(ComplexPointer<float> pointer, expression::Name<A> expr, size_t size) { \
		fill##Name(pointer, expr, size); \
	} \
	template<class A> \
	void fillExpr(ComplexPointer<double> pointer, expression::Name<A> expr, size_t size) { \
		fill##Name(pointer, expr, size); \
	} \
	template<class Expr> \
	void fill##Name(ComplexPointer<float> pointer, Expr expr, size_t size) { \
		fillBasic(pointer, expr, size); \
	} \
	template<class Expr> \
	void fill##Name(ComplexPointer<double> pointer, Expr expr, size_t size) { \
		fillBasic(pointer, expr, size); \
	} \
	template<class A> \
	void fillExpr(SplitPointer<float> pointer, expression::Name<A> expr, size_t size) { \
		fill##Name(pointer, expr, size); \
	} \
	template<class A> \
	void fillExpr(SplitPointer<double> pointer, expression::Name<A> expr, size_t size) { \
		fill##Name(pointer, expr, size); \
	} \
	template<class Expr> \
	void fill##Name(SplitPointer<float> pointer, Expr expr, size_t size) { \
		fillBasic(pointer, expr, size); \
	} \
	template<class Expr> \
	void fill##Name(SplitPointer<double> pointer, Expr expr, size_t size) { \
		fillBasic(pointer, expr, size); \
	}
// R -> R operators
#define SIGNALSMITH_AUDIO_LINEAR_TREE1_RR(Name, vDSP_func) \
	template<class A> \
	ItemType<A, float, void> fill##Name(RealPointer<float> pointer, expression::Name<A> expr, size_t size) { \
		auto floats = cached.floatScope(); \
		auto *a = floats.real(expr.a, size); \
		vDSP_func(a, 1, pointer, 1, size); \
	} \
	template<class A> \
	ItemType<A, double, void> fill##Name(RealPointer<double> pointer, expression::Name<A> expr, size_t size) { \
		auto doubles = cached.doubleScope(); \
		auto *a = doubles.real(expr.a, size); \
		vDSP_func##D(a, 1, pointer, 1, size); \
	}
// C -> R operators
#define SIGNALSMITH_AUDIO_LINEAR_TREE1_RC(Name, vDSP_func) \
	template<class A> \
	ItemType<A, std::complex<float>, void> fill##Name(RealPointer<float> pointer, expression::Name<A> expr, size_t size) { \
		auto floats = cached.floatScope(); \
		auto a = dspSplit(floats.split(expr.a, size)); \
		vDSP_func(&a, 1, pointer, 1, size); \
	} \
	template<class A> \
	ItemType<A, std::complex<double>, void> fill##Name(RealPointer<double> pointer, expression::Name<A> expr, size_t size) { \
		auto doubles = cached.doubleScope(); \
		auto a = dspSplit(doubles.split(expr.a, size)); \
		vDSP_func##D(&a, 1, pointer, 1, size); \
	}
// C -> C operators
#define SIGNALSMITH_AUDIO_LINEAR_TREE1_CC(Name, vDSP_func) \
	template<class A> \
	ItemType<A, std::complex<float>, void> fill##Name(ComplexPointer<float> pointer, expression::Name<A> expr, size_t size) { \
		auto floats = cached.floatScope(); \
		auto a = dspSplit(floats.complex(expr.a, size)); \
		auto p = dspSplit(pointer); \
		vDSP_func(&a, 2, &p, 2, size); \
	} \
	template<class A> \
	ItemType<A, std::complex<double>, void> fill##Name(ComplexPointer<double> pointer, expression::Name<A> expr, size_t size) { \
		auto doubles = cached.doubleScope(); \
		auto a = dspSplit(doubles.complex(expr.a, size)); \
		auto p = dspSplit(pointer); \
		vDSP_func##D(&a, 2, &p, 2, size); \
	}\
	template<class A> \
	ItemType<A, std::complex<float>, void> fill##Name(SplitPointer<float> pointer, expression::Name<A> expr, size_t size) { \
		auto floats = cached.floatScope(); \
		auto a = dspSplit(floats.split(expr.a, size)); \
		auto p = dspSplit(pointer); \
		vDSP_func(&a, 1, &p, 1, size); \
	} \
	template<class A> \
	ItemType<A, std::complex<double>, void> fill##Name(SplitPointer<double> pointer, expression::Name<A> expr, size_t size) { \
		auto doubles = cached.doubleScope(); \
		auto a = dspSplit(doubles.split(expr.a, size)); \
		auto p = dspSplit(pointer); \
		vDSP_func##D(&a, 1, &p, 1, size); \
	}
	SIGNALSMITH_AUDIO_LINEAR_OP1_R(Abs)
	SIGNALSMITH_AUDIO_LINEAR_TREE1_RC(Abs, vDSP_zvabs);
	SIGNALSMITH_AUDIO_LINEAR_OP1_R(Neg)
	SIGNALSMITH_AUDIO_LINEAR_OP1_C(Neg)
	SIGNALSMITH_AUDIO_LINEAR_TREE1_RR(Neg, vDSP_vneg);
	SIGNALSMITH_AUDIO_LINEAR_TREE1_CC(Neg, vDSP_zvneg);
#undef SIGNALSMITH_AUDIO_LINEAR_TREE1_RR
#undef SIGNALSMITH_AUDIO_LINEAR_TREE1_RC
#undef SIGNALSMITH_AUDIO_LINEAR_TREE1_CC

// vForce stuff
// R -> R operators
#define SIGNALSMITH_AUDIO_LINEAR_TREE1_RR(Name, vForce_float, vForce_double) \
	template<class A> \
	ItemType<A, float, void> fill##Name(RealPointer<float> pointer, expression::Name<A> expr, size_t size) { \
		auto floats = cached.floatScope(); \
		auto *a = floats.real(expr.a, size, pointer); \
		int intSize = int(size); \
		vForce_float(pointer, a, &intSize); \
	} \
	template<class A> \
	ItemType<A, double, void> fill##Name(RealPointer<double> pointer, expression::Name<A> expr, size_t size) { \
		auto doubles = cached.doubleScope(); \
		auto *a = doubles.real(expr.a, size, pointer); \
		int intSize = int(size); \
		vForce_double(pointer, a, &intSize); \
	}
	SIGNALSMITH_AUDIO_LINEAR_TREE1_RR(Abs, vvfabsf, vvfabs);
	SIGNALSMITH_AUDIO_LINEAR_OP1_R(Exp)
	SIGNALSMITH_AUDIO_LINEAR_TREE1_RR(Exp, vvexpf, vvexp);
	SIGNALSMITH_AUDIO_LINEAR_OP1_R(Exp2)
	SIGNALSMITH_AUDIO_LINEAR_TREE1_RR(Exp2, vvexp2f, vvexp2);
	SIGNALSMITH_AUDIO_LINEAR_OP1_R(Log)
	SIGNALSMITH_AUDIO_LINEAR_TREE1_RR(Log, vvlogf, vvlog);
	SIGNALSMITH_AUDIO_LINEAR_OP1_R(Log2)
	SIGNALSMITH_AUDIO_LINEAR_TREE1_RR(Log2, vvlog2f, vvlog2);
	SIGNALSMITH_AUDIO_LINEAR_OP1_R(Log10)
	SIGNALSMITH_AUDIO_LINEAR_TREE1_RR(Log10, vvlog10f, vvlog10);
	SIGNALSMITH_AUDIO_LINEAR_OP1_R(Sqrt)
	SIGNALSMITH_AUDIO_LINEAR_TREE1_RR(Sqrt, vvsqrtf, vvsqrt);
	SIGNALSMITH_AUDIO_LINEAR_OP1_R(Ceil)
	SIGNALSMITH_AUDIO_LINEAR_TREE1_RR(Ceil, vvceilf, vvceil);
	SIGNALSMITH_AUDIO_LINEAR_OP1_R(Floor)
	SIGNALSMITH_AUDIO_LINEAR_TREE1_RR(Floor, vvfloorf, vvfloor);
#undef SIGNALSMITH_AUDIO_LINEAR_OP1_R

// Forwards .fillExpr() to .fillName(), but doesn't define that
#define SIGNALSMITH_AUDIO_LINEAR_OP2_R(Name) \
public: \
	template<class A, class B> \
	void fillExpr(RealPointer<float> pointer, expression::Name<A, B> expr, size_t size) { \
		fill##Name(pointer, expr, size); \
	} \
	template<class A, class B> \
	void fillExpr(RealPointer<double> pointer, expression::Name<A, B> expr, size_t size) { \
		fill##Name(pointer, expr, size); \
	} \
private:
// R x R -> R operators where the vDSP function arguments are the other way around for some reason
#define SIGNALSMITH_AUDIO_LINEAR_TREE2FLIP_RRR(Name, vDSP_func) \
	template<class A, class B> \
	void fill##Name(RealPointer<float> pointer, expression::Name<A, B> expr, size_t size) { \
		auto floats = cached.floatScope(); \
		auto *a = floats.real(expr.a, size, pointer); \
		auto *b = floats.real(expr.b, size); \
		vDSP_func(b, 1, a, 1, pointer, 1, size); \
	} \
	template<class A, class B> \
	void fill##Name(RealPointer<double> pointer, expression::Name<A, B> expr, size_t size) { \
		auto doubles = cached.doubleScope(); \
		auto *a = doubles.real(expr.a, size, pointer); \
		auto *b = doubles.real(expr.b, size); \
		vDSP_func##D(b, 1, a, 1, pointer, 1, size); \
	}
#define SIGNALSMITH_AUDIO_LINEAR_TREE2COMM_RRkR(Name, vDSP_func) \
	template<class A, class V> \
	void fill##Name(RealPointer<float> pointer, expression::Name<A, expression::ConstantExpr<V>> expr, size_t size) { \
		auto floats = cached.floatScope(); \
		auto *a = floats.real(expr.a, size, pointer); \
		float b = expr.b.value; \
		vDSP_func(a, 1, &b, pointer, 1, size); \
	} \
	template<class A, class V> \
	void fill##Name(RealPointer<double> pointer, expression::Name<A, expression::ConstantExpr<V>> expr, size_t size) { \
		auto doubles = cached.doubleScope(); \
		auto *a = doubles.real(expr.a, size, pointer); \
		double b = expr.b.value; \
		vDSP_func##D(a, 1, &b, pointer, 1, size); \
	} \
	template<class V, class B> \
	void fill##Name(RealPointer<float> pointer, expression::Name<expression::ConstantExpr<V>, B> expr, size_t size) { \
		auto floats = cached.floatScope(); \
		float a = expr.a.value; \
		auto *b = floats.real(expr.b, size, pointer); \
		vDSP_func(b, 1, &a, pointer, 1, size); \
	} \
	template<class V, class B> \
	void fill##Name(RealPointer<double> pointer, expression::Name<expression::ConstantExpr<V>, B> expr, size_t size) { \
		auto doubles = cached.doubleScope(); \
		double a = expr.a.value; \
		auto *b = doubles.real(expr.b, size, pointer); \
		vDSP_func##D(b, 1, &a, pointer, 1, size); \
	}
	SIGNALSMITH_AUDIO_LINEAR_OP2_R(Add)
	SIGNALSMITH_AUDIO_LINEAR_TREE2FLIP_RRR(Add, vDSP_vadd);
	SIGNALSMITH_AUDIO_LINEAR_TREE2COMM_RRkR(Add, vDSP_vsadd);
	SIGNALSMITH_AUDIO_LINEAR_OP2_R(Sub)
	SIGNALSMITH_AUDIO_LINEAR_TREE2FLIP_RRR(Sub, vDSP_vsub);
	template<class A, class V>
	void fillSub(RealPointer<float> pointer, expression::Sub<A, expression::ConstantExpr<V>> expr, size_t size) {
		expression::ConstantExpr<V> negB{-expr.b.value};
		fillAdd(pointer, expression::makeAdd(expr.a, negB), size);
	}
	template<class A, class V>
	void fillSub(RealPointer<double> pointer, expression::Sub<A, expression::ConstantExpr<V>> expr, size_t size) {
		expression::ConstantExpr<V> negB{-expr.b.value};
		fillAdd(pointer, expression::makeAdd(expr.a, negB), size);
	}
	template<class A, class V>
	void fillSub(RealPointer<float> pointer, expression::Sub<expression::ConstantExpr<V>, A> expr, size_t size) {
		expression::ConstantExpr<V> negA{-expr.a.value};
		fillNeg(pointer, expression::makeNeg(expression::makeAdd(expr.b, negA)), size);
	}
	template<class A, class V>
	void fillSub(RealPointer<double> pointer, expression::Sub<expression::ConstantExpr<V>, A> expr, size_t size) {
		expression::ConstantExpr<V> negA{-expr.a.value};
		fillNeg(pointer, expression::makeNeg(expression::makeAdd(expr.b, negA)), size);
	}
	SIGNALSMITH_AUDIO_LINEAR_OP2_R(Mul)
	SIGNALSMITH_AUDIO_LINEAR_TREE2FLIP_RRR(Mul, vDSP_vmul);
	SIGNALSMITH_AUDIO_LINEAR_TREE2COMM_RRkR(Mul, vDSP_vsmul);
	SIGNALSMITH_AUDIO_LINEAR_OP2_R(Div)
	template<class A, class V>
	void fillDiv(RealPointer<float> pointer, expression::Div<A, expression::ConstantExpr<V>> expr, size_t size) {
		expression::ConstantExpr<V> recipB{1.0f/expr.b.value};
		fillMul(pointer, expression::makeMul(expr.a, recipB), size);
	}
	template<class A, class V>
	void fillDiv(RealPointer<double> pointer, expression::Sub<A, expression::ConstantExpr<V>> expr, size_t size) {
		expression::ConstantExpr<V> recipB{1.0/expr.b.value};
		fillMul(pointer, expression::makeMul(expr.a, recipB), size);
	}
#undef SIGNALSMITH_AUDIO_LINEAR_TREE2FLIP_RRR
#undef SIGNALSMITH_AUDIO_LINEAR_TREE2COMM_RRkR

// R x R -> R operators in vForce
#define SIGNALSMITH_AUDIO_LINEAR_TREE2_RRR(Name, vForce_float, vForce_double) \
	template<class A, class B> \
	void fill##Name(RealPointer<float> pointer, expression::Name<A, B> expr, size_t size) { \
		auto floats = cached.floatScope(); \
		auto *a = floats.real(expr.a, size, pointer); \
		auto *b = floats.real(expr.b, size); \
		int intSize = int(size); \
		vForce_float(pointer, a, b, &intSize); \
	} \
	template<class A, class B> \
	void fill##Name(RealPointer<double> pointer, expression::Name<A, B> expr, size_t size) { \
		auto doubles = cached.doubleScope(); \
		auto *a = doubles.real(expr.a, size, pointer); \
		auto *b = doubles.real(expr.b, size); \
		int intSize = int(size); \
		vForce_double(pointer, a, b, &intSize); \
	}
	SIGNALSMITH_AUDIO_LINEAR_TREE2_RRR(Div, vvdivf, vvdiv);
#undef SIGNALSMITH_AUDIO_LINEAR_TREE2_RRR
#undef SIGNALSMITH_AUDIO_LINEAR_OP2_R

// Forwards .fillName() to .fillNameName2(), but doesn't define that
#define SIGNALSMITH_AUDIO_LINEAR_Op3L_R(Name, NameL) \
	template<class A, class B, class C> \
	void fill##Name(RealPointer<float> pointer, expression::Name<expression::NameL<A, B>, C> expr, size_t size) { \
		fill##Name##NameL(pointer, expr, size); \
	} \
	template<class A, class B, class C> \
	void fill##Name(RealPointer<double> pointer, expression::Name<expression::NameL<A, B>, C> expr, size_t size) { \
		fill##Name##NameL(pointer, expr, size); \
	}
// (A $ B) $ C
#define SIGNALSMITH_AUDIO_LINEAR_TREE3L_RRR(Name, NameL, vDSP_func) \
	template<class A, class B, class C> \
	void fill##Name##NameL(RealPointer<float> pointer, expression::Name<expression::NameL<A, B>, C> expr, size_t size) { \
		auto floats = cached.floatScope(); \
		auto *a = floats.real(expr.a.a, size); \
		auto *b = floats.real(expr.a.b, size); \
		auto *c = floats.real(expr.b, size, pointer); \
		vDSP_func(a, 1, b, 1, c, 1, pointer, 1, size); \
	} \
	template<class A, class B, class C> \
	void fill##Name##NameL(RealPointer<double> pointer, expression::Name<expression::NameL<A, B>, C> expr, size_t size) { \
		auto doubles = cached.doubleScope(); \
		auto *a = doubles.real(expr.a.a, size); \
		auto *b = doubles.real(expr.a.b, size); \
		auto *c = doubles.real(expr.b, size, pointer); \
		vDSP_func##D(a, 1, b, 1, c, 1, pointer, 1, size); \
	}
	SIGNALSMITH_AUDIO_LINEAR_Op3L_R(Mul, Add)
	SIGNALSMITH_AUDIO_LINEAR_TREE3L_RRR(Mul, Add, vDSP_vam)

#undef SIGNALSMITH_AUDIO_LINEAR_TREE3L_RRR
#undef SIGNALSMITH_AUDIO_LINEAR_Op3L_R

//	SIGNALSMITH_AUDIO_LINEAR_TREE3COMMUTATIVE_RRR(Add, Mul, vDSP_vam, 0)
//	SIGNALSMITH_AUDIO_LINEAR_TREE3COMMUTATIVE_RRR(Mul, Add, vDSP_vma, 1)
//	SIGNALSMITH_AUDIO_LINEAR_TREE3COMMUTATIVE_RRR(Sub, Mul, vDSP_vsbm, 2)
//	SIGNALSMITH_AUDIO_LINEAR_TREE3L_RRR(Mul, Sub, vDSP_vmsb, 3)

// Forwards .fillExpr() to .fillName(), but doesn't define that
#define SIGNALSMITH_AUDIO_LINEAR_OP2_C(Name) \
public: \
	template<class A, class B> \
	void fillExpr(ComplexPointer<float> pointer, expression::Name<A, B> expr, size_t size) { \
		fill##Name(pointer, expr, size); \
	} \
	template<class A, class B> \
	void fillExpr(ComplexPointer<double> pointer, expression::Name<A, B> expr, size_t size) { \
		fill##Name(pointer, expr, size); \
	} \
	template<class A, class B> \
	void fillExpr(SplitPointer<float> pointer, expression::Name<A, B> expr, size_t size) { \
		fill##Name(pointer, expr, size); \
	} \
	template<class A, class B> \
	void fillExpr(SplitPointer<double> pointer, expression::Name<A, B> expr, size_t size) { \
		fill##Name(pointer, expr, size); \
	} \
private:
//	SIGNALSMITH_AUDIO_LINEAR_OP2_C(Add)
//	SIGNALSMITH_AUDIO_LINEAR_OP2_C(Sub)
//	SIGNALSMITH_AUDIO_LINEAR_OP2_C(Mul)
//	SIGNALSMITH_AUDIO_LINEAR_OP2_C(Div)

protected:
	CachedResults<LinearImpl, 32> cached;

	static DSPSplitComplex dspSplit(ConstSplitPointer<float> x) {
		DSPSplitComplex dsp;
		dsp.realp = (float *)x.real;
		dsp.imagp = (float *)x.imag;
		return dsp;
	}
	static DSPSplitComplex dspSplit(ConstComplexPointer<float> x) {
		DSPSplitComplex dsp;
		dsp.realp = (float *)x;
		dsp.imagp = (float *)x + 1;
		return dsp;
	}
	static DSPDoubleSplitComplex dspSplit(ConstSplitPointer<double> x) {
		DSPDoubleSplitComplex dsp;
		dsp.realp = (double *)x.real;
		dsp.imagp = (double *)x.imag;
		return dsp;
	}
	static DSPDoubleSplitComplex dspSplit(ConstComplexPointer<double> x) {
		DSPDoubleSplitComplex dsp;
		dsp.realp = (double *)x;
		dsp.imagp = (double *)x + 1;
		return dsp;
	}
};

}}; // namespace
