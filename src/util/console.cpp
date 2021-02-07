#include "console.h"

#include <stdio.h>
#include <QtDebug>
#include "util/version.h"

#ifdef __WINDOWS__
#include <io.h> // Debug Console
#include <string.h>
#include <conio.h>
#include <strsafe.h>

typedef BOOL(WINAPI* pfGetCurrentConsoleFontEx)(HANDLE, BOOL, PCONSOLE_FONT_INFOEX);
typedef BOOL(WINAPI* pfSetCurrentConsoleFontEx)(HANDLE, BOOL, PCONSOLE_FONT_INFOEX);

Console::Console()
     : m_shouldResetCodePage(false),
       m_shouldResetConsoleTitle(false),
       m_shouldFreeConsole(false) {

    // Unlike Linux and MacOS Windows does not support windows and console
    // applications at the same time.
    //
    // We link Mixxx win with the /subsystem:windows flag, to avoid a new
    // console popping up when the program is started by a double-click.
    // If Mixxx is started from a command line like cmd.exe the output is not
    // shown by default.
    // Here we fixing that by detecting the console case and redirect to output
    // to it if not already redirected.

    // Note: GetFileType needs to be called before AttachConsole(),
    // else it always returns FILE_TYPE_CHAR
    DWORD typeStdIn = GetFileType(GetStdHandle(STD_INPUT_HANDLE));
    DWORD typeStdOut = GetFileType(GetStdHandle(STD_OUTPUT_HANDLE));
    DWORD typeStdErr = GetFileType(GetStdHandle(STD_ERROR_HANDLE));

    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        // we are started from a console process
        m_shouldFreeConsole = true;

        if (typeStdIn == FILE_TYPE_UNKNOWN) {
            // the input is not already redirected
            FILE* pStdin = stdin;
            if (freopen_s(&pStdin, "CONIN$", "r", stdin)) {
                qWarning() << "Could not open stdin. Error code:" << GetLastError();
            }
        }

        if (typeStdOut == FILE_TYPE_UNKNOWN) {
            // the output is not already redirected
            FILE* pStdout = stdout;
            if (freopen_s(&pStdout, "CONOUT$", "w", stdout)) {
                qWarning() << "Could not open stdout. Error code:" << GetLastError();
            } else {
                if (setvbuf(stdout, NULL, _IONBF, 0) != 0) {
                    qWarning() << "Setting no buffer for stdout failed.";
                }
            }
        }

        if (typeStdErr == FILE_TYPE_UNKNOWN) {
            // the error is not already redirected
            FILE* pStderr = stderr;
            if (freopen_s(&pStderr, "CONOUT$", "w", stderr)) {
                qWarning() << "Could not open stderr. Error code:" << GetLastError();
            } else {
                if (setvbuf(stdout, NULL, _IONBF, 0) != 0) {
                    qWarning() << "Setting no buffer for stderr failed.";
                }
            }
        }

        // Save current code page
        m_oldCodePage = GetConsoleOutputCP();
        m_shouldResetCodePage = true;

        // Verify using the unicode font "Consolas"
        HMODULE kernel32_dll = LoadLibraryW(L"kernel32.dll");
        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (kernel32_dll && hStdOut != INVALID_HANDLE_VALUE) {
            pfGetCurrentConsoleFontEx pfGCCFX =
                    (pfGetCurrentConsoleFontEx) GetProcAddress(kernel32_dll,
                            "GetCurrentConsoleFontEx"); // Supported from Windows Vista
            pfSetCurrentConsoleFontEx pfSCCFX =
                    (pfSetCurrentConsoleFontEx) GetProcAddress(kernel32_dll,
                            "SetCurrentConsoleFontEx"); // Supported from Windows Vista
            bool setFont = true;
            if (pfGCCFX) {
                CONSOLE_FONT_INFOEX font;
                if (pfGCCFX(hStdOut, FALSE, &font)) {
                    if (wcsncmp(font.FaceName, L"Consolas", 8) == 0) {
                        // Nothing to do
                        setFont = false;
                    }
                }
            }

            if (setFont) {
                if (pfSCCFX) {
                    // Use a unicode font
                    CONSOLE_FONT_INFOEX newFont;
                    newFont.cbSize = sizeof newFont;
                    newFont.nFont = 0;
                    newFont.dwFontSize.X = 0;
                    newFont.dwFontSize.Y = 14;
                    newFont.FontFamily = FF_DONTCARE;
                    newFont.FontWeight = FW_NORMAL;
                    wcscpy_s(newFont.FaceName, L"Consolas");
                    pfSCCFX(hStdOut, FALSE, &newFont);
                } else {
                    // This happens on Windows XP
                    qWarning() << "The console font may not support non ANSI characters." <<
                                  "In case of character issues switch to font \"Lucida Console\"";
                }
            }

            TCHAR szNewTitle[MAX_PATH];
            // Save current console title.
            if (GetConsoleTitle(m_oldTitle, MAX_PATH)) {
                // Build new console title string.
#ifdef UNICODE
                StringCchPrintf(szNewTitle, MAX_PATH, TEXT("%s : %s"),
                        m_oldTitle,  Version::applicationTitle().utf16());
#else
                StringCchPrintf(szNewTitle, MAX_PATH, TEXT("%s : %s"),
                        m_oldTitle,  Version::applicationTitle().toLocal8Bit().data());
#endif
                // Set console title to new title
                if (SetConsoleTitle(szNewTitle)) {
                    m_shouldResetConsoleTitle = true;
                } else {
                    qWarning() << "SetConsoleTitle failed" << GetLastError();
                }
            }
        }

        // Setup Windows console encoding
        // toLocal8Bit() returns the ANSI file encoding format
        // this does not necessarily match the OEM console encoding
        // https://www.microsoft.com/resources/msdn/goglobal/default.mspx
        // In case of a German Windows XP to 10 console encoding is cp850
        // where files encoding is cp1252
        // Qt has no solution for it https://bugreports.qt.io/browse/QTBUG-13303
        // http://stackoverflow.com/questions/1259084/what-encoding-code-page-is-cmd-exe-using
        // We try to change the console encoding to file encoding
        // For a German windows we expect
        // LOCALE_IDEFAULTANSICODEPAGE "1252" // ANSI Codepage used by Qt toLocal8Bit
        // LOCALE_IDEFAULTCODEPAGE "850" // OEM Codepage Console

        // set console to the default ANSI Code Page
        UINT defaultCodePage;
        if (GetLocaleInfo(LOCALE_USER_DEFAULT,
                          LOCALE_RETURN_NUMBER | LOCALE_IDEFAULTANSICODEPAGE,
                          reinterpret_cast<LPWSTR>(&defaultCodePage),
                          sizeof(defaultCodePage)) != 0) {
            SetConsoleOutputCP(defaultCodePage);
        } else {
            qWarning() << "GetLocaleInfo failed. Error code:" << GetLastError();
        }
    } else {
        // started by double click
        // no need to deal with a console
    }
}

Console::~Console() {
    // Reset Windows console to old code page
    // We need to stick with the unicode font since
    // changing back will destroy the console history
    if (m_shouldResetCodePage) {
        SetConsoleOutputCP(m_oldCodePage);
    }
    if (m_shouldResetConsoleTitle) {
        if (!SetConsoleTitle(m_oldTitle)) {
            qWarning() << "SetConsoleTitle failed" << GetLastError();
        }
    }
    if (m_shouldFreeConsole) {
        // Note: The console has already written the command on top of the output
        // because it was originally released due to the /subsystem:windows flag.
        // We may send a fake "Enter" key here, using SendInput() to get a new
        // command prompt, but this executes a user entry unconditionally or has other
        // bad side effects.
        FreeConsole();
    }
}

#else // __WINDOWS__

// Nothing to do on non Windows targets
Console::Console() {
}

Console::~Console() {
}

#endif // __WINDOWS__
