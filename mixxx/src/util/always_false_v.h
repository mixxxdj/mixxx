#pragma once

// always false, used for static_assert and workaround for compilers without
// https://cplusplus.github.io/CWG/issues/2518.html
template<typename T>
static constexpr bool always_false_v = false;
