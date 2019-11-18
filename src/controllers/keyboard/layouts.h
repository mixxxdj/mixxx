/*************************************************************************
** This code was generated with layoutstool                             **
**                                                                      **
** WARNING: Changes to this file may be overridden by the tool!         **
**                                                                      **
**          If you want to add or delete layouts, please use the tool.  **
**          Layoutstool can be found in mixxx/scripts/layouts_tool and  **
**          build with build.sh. The executable will be placed in       **
**          mixxx/scripts/layouts_tool/bin                              **
**                                                                      **
** NOTE:    Layoutstool does only work on Linux (make sure you have GCC **
**          and CMake installed in order to successfully build and run  **
**          the tool.                                                   **
*************************************************************************/

#ifndef LAYOUTS_H
#define LAYOUTS_H

#include <string>

struct KbdKeyChar {
    char16_t character;
    bool isDead;
};

typedef const KbdKeyChar (*KeyboardLayoutPointer)[2];
KeyboardLayoutPointer getLayout(std::string layoutName);

#endif // LAYOUTS_H
