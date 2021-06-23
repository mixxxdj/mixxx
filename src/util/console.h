#pragma once

#ifdef __WINDOWS__
#include <windows.h>
#include <tchar.h>
#endif

class Console {
  public:
    Console();
    ~Console();

  private:
#ifdef __WINDOWS__
    unsigned int m_oldCodePage;
    bool m_shouldResetCodePage;
    bool m_shouldResetConsoleTitle;
    bool m_shouldFreeConsole;
    TCHAR m_oldTitle[MAX_PATH];
#endif
};
