#pragma once

// DEPRECATED: Adhere to the rule "the rule of zero" or "the rule of five".
// See https://en.cppreference.com/w/cpp/language/rule_of_three
// Rationale: If a class deals with ownership, it should do nothing more and explicitly
// define the behavior of all special member functions.
// This macro is sprinkled into lots of big classes and ignores the move-related SMF's.
// So its common use violates both principles and thus it should not be used anymore!
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&) = delete;    \
    void operator=(const TypeName&) = delete;
