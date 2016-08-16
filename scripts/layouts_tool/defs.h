#ifndef DEFS_H
#define DEFS_H

static const int LAYOUT_LEN = 48;

// See figure 2 at: http://www.charvolant.org/~doug/xkb/html/node5.html
enum keycodes {
    // Digit row
    TLDE = 49,  // `
    AE01 = 10,  // 1
    AE02,       // 2
    AE03,       // 3
    AE04,       // 4
    AE05,       // 5
    AE06,       // 6
    AE07,       // 7
    AE08,       // 8
    AE09,       // 9
    AE10,       // 0
    AE11,       // -
    AE12,       // =

    // Upper row
    AD01 = 24,  // q
    AD02,       // w
    AD03,       // e
    AD04,       // r
    AD05,       // t
    AD06,       // y
    AD07,       // u
    AD08,       // i
    AD09,       // o
    AD10,       // p
    AD11,       // [
    AD12,       // ]

    // Home row
    AC01 = 38,  // a
    AC02,       // s
    AC03,       // d
    AC04,       // f
    AC05,       // g
    AC06,       // h
    AC07,       // j
    AC08,       // k
    AC09,       // l
    AC10,       // ;
    AC11,       // '
    BKSL = 51,  // \

    // Lower row
    LSGT = 94,  // / (key between z and shift)
    AB01 = 52,  // z
    AB02,       // x
    AB03,       // c
    AB04,       // v
    AB05,       // b
    AB06,       // n
    AB07,       // m
    AB08,       // ,
    AB09,       // .
    AB10        // /
};

#endif // DEFS_H
