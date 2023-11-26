#include "nativeeventhandlerwin.h"
#if defined(__WINDOWS__)
#include <commctrl.h>
#include <windows.h>
#include <winuser.h>

#include "moc_nativeeventhandlerwin.cpp"

WindowsEventHandler::WindowsEventHandler(MixxxApplication* pApp)
        : m_pApp(pApp) {
}

bool WindowsEventHandler::nativeEventFilter(
        const QByteArray& eventType, void* message, long* result) {
    MSG* msg = (MSG*)message;
    if (msg && msg->message == WM_NCLBUTTONDOWN) {
        // Trigger the modal loop to prevent 500ms wait in Event Loop
        // when Windows setting "Show window contents while dragging" is enabled
        // and the user left-clicks and holds with the mouse on the titlebar of
        // a window
        RedrawWindow(
                msg->hwnd,
                nullptr,
                0,
                RDW_INTERNALPAINT);

        if (msg->wParam == HTCAPTION) {
            PostMessageW(msg->hwnd, WM_MOUSEMOVE, 0, 0);
        }

        DefSubclassProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);
    }

    return false;
}
#endif
