#include "../mocs_compilation.cpp"

// QT_VERSION will be defined by any moc_<header_base>.cpp file included from mocs_compilation.cpp
// It is empty in case all moc files are included, a requirement to speed up incremental builds.
// See https://cmake.org/cmake/help/latest/prop_tgt/AUTOMOC.html for details.
#ifdef QT_VERSION
#error mocs_compilation.cpp not empty. Move all #include "moc_<header_base>.cpp" lines from mocs_compilation.cpp to the cpp files of the related classes.
#endif
