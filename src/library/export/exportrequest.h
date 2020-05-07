#pragma once

#include <QSet>

#include "library/crate/crateid.h"

namespace mixxx {

/// The ExportRequest struct is the base class for all types of request to export
/// the Mixxx library to some external location.  An export to a specific kind of
/// external location is likely to need additional information to be specified in
/// the request, which can be achieved by sub-classing this struct.
struct ExportRequest {
    /// Indicates whether to export only selected crates (if set to true), or
    /// whether to export the whole Mixxx library (if set to false).
    bool exportSelectedCrates;

    /// Set of crates to export, if `exportSelectedCrates` is set to true.
    QSet<CrateId> crateIdsToExport;
};

} // namespace mixxx
