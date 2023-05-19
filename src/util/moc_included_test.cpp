#include "../mocs_compilation.cpp"

// QT_VERSION will be defined by any moc_<header_base>.cpp file included from mocs_compilation.cpp
// It is empty in case all moc files are included
#ifdef QT_VERSION
#error "Not all moc_<header_base>.cpp files included from their cpp sources. This is a Mixxx requirement to speed up the build process. See https://cmake.org/cmake/help/latest/prop_tgt/AUTOMOC.html for details."
#endif
