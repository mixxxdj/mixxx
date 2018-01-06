#include "engine/enginefilteriir.h"

#include <QMutex>
#include <QMutexLocker>


namespace {

// This statically initialized instance is safe, because the wrapped
// functions are only invoked when creating dynamically allocated
// objects at runtime after entering main().
QMutex fidlibMutex;

} // anonymous namespace

//static
FidFilter *EngineFilterIIRBase::fid_design(const char *spec, double rate, double freq0, double freq1,
                     int f_adj, char **descp) {
    QMutexLocker locked(&fidlibMutex);
    return ::fid_design(spec, rate, freq0, freq1, f_adj, descp);
}

//static
double EngineFilterIIRBase::fid_design_coef(double *coef, int n_coef, const char *spec,
                      double rate, double freq0, double freq1, int adj) {
    QMutexLocker locked(&fidlibMutex);
    return ::fid_design_coef(coef, n_coef, spec, rate, freq0, freq1, adj);
}
