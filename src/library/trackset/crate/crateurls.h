#pragma once

#include "library/trackset/crate/crateid.h"
#include "util/urlhelper.h"

class CrateURLs {
  private:
    static const QString kUrlTemplate;

  public:
    // Returns the URL representing the specified, valid crateId,
    // or an empty QUrl if crateId is invalid.
    static QUrl toUrl(CrateId crateId) {
        return UrlHelper::urlFromTemplate(kUrlTemplate, crateId);
    }

    // Returns the CrateId represented by the specified QUrl,
    // or an invalid CrateId if the URL does not represent a crate.
    static CrateId parseCrateUrl(const QUrl& url) {
        return UrlHelper::idFromUrl<CrateId>(kUrlTemplate, url);
    }

    // Parses the list of URLs, and returns the corresponding list of crate ids.
    // Urls that do not represent crate references are ignored.
    static QList<CrateId> parseCrateUrls(const QList<QUrl>& urls) {
        return UrlHelper::idsFromUrls<CrateId>(kUrlTemplate, urls);
    }
};

const QString CrateURLs::kUrlTemplate = QStringLiteral("mixxx://library/crates");
