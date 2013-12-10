#ifndef UTIL_H
#define UTIL_H

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

// Based on qFuzzyCompare but without the problems where p1 or p2 are zero.
Q_DECL_CONSTEXPR static inline bool floatCompare(double p1, double p2) {
    return (qAbs(p1 - p2) <= 0.000000001);
}

#endif /* UTIL_H */
