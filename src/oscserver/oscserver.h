#ifndef __OSCSERVER_H__
#define __OSCSERVER_H__

#include <QRegularExpression>

#include "lo/lo.h"
#include "preferences/usersettings.h"
#include "control/controlobject.h"
#include "oscserver/defs_oscserver.h"

class OscServer {
public:
    OscServer(UserSettingsPointer& pConfig);
    ~OscServer();

private:
    static void quitServer();

    static void oscErrorHandler(int err, const char* msg, const char* path);
    static int oscMsgHandler(const char* path, const char* types, lo_arg** argv, int argc, void* data, void* userData);

    static lo_server_thread m_st;
};

#endif
