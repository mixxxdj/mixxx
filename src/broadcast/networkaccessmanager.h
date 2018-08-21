
#include <QNetworkAccessManager>

#include "util/singleton.h"

class NetworkAccessManager : public Singleton<QNetworkAccessManager> {};
