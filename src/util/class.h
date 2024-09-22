#pragma once

// DEPRECATED: Instead rely on "the rule of zero". See
// https://en.cppreference.com/w/cpp/language/rule_of_three#Rule_of_zero
// See also: Cpp Core Guidelines C.20, C.21 & C.22 as well as the clang-tidy rule
// `modernize-use-equals-delete`
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&) = delete;    \
    void operator=(const TypeName&) = delete;
