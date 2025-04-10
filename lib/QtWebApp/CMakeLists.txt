project(QtWebApp) 

find_package(Qt5 COMPONENTS Core Network REQUIRED)

set (CMAKE_CXX_STANDARD 11)

add_library(qtwebapp STATIC )

target_sources(
    qtwebapp
    PRIVATE
        logging/logmessage.cpp 
        logging/logger.cpp 
        logging/filelogger.cpp 
        logging/dualfilelogger.cpp
    PUBLIC
        logging/logglobal.h 
        logging/logmessage.h 
        logging/logger.h 
        logging/filelogger.h 
        logging/dualfilelogger.h
)

target_sources(
    qtwebapp
    PRIVATE
        httpserver/httpglobal.cpp 
        httpserver/httplistener.cpp 
        httpserver/httpconnectionhandler.cpp 
        httpserver/httpconnectionhandlerpool.cpp 
        httpserver/httprequest.cpp 
        httpserver/httpresponse.cpp 
        httpserver/httpcookie.cpp 
        httpserver/httprequesthandler.cpp 
        httpserver/httpsession.cpp 
        httpserver/httpsessionstore.cpp 
        httpserver/staticfilecontroller.cpp
    PUBLIC
        httpserver/httpglobal.h 
        httpserver/httplistener.h 
        httpserver/httpconnectionhandler.h 
        httpserver/httpconnectionhandlerpool.h 
        httpserver/httprequest.h 
        httpserver/httpresponse.h 
        httpserver/httpcookie.h 
        httpserver/httprequesthandler.h 
        httpserver/httpsession.h 
        httpserver/httpsessionstore.h 
        httpserver/staticfilecontroller.h
)

target_sources(
    qtwebapp
    PRIVATE
        templateengine/template.cpp 
        templateengine/templateloader.cpp 
        templateengine/templatecache.cpp
    PUBLIC
        templateengine/templateglobal.h
        templateengine/template.h 
        templateengine/templateloader.h 
        templateengine/templatecache.h
)



target_link_libraries(qtwebapp PRIVATE Qt5::Core Qt5::Network)
