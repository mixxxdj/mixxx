#pragma once

#include <QMetaType>

namespace mixxx {

namespace network {

typedef int HttpStatusCode;

constexpr HttpStatusCode kHttpStatusCodeInvalid = -1;
constexpr HttpStatusCode kHttpStatusCodeOk = 200;
constexpr HttpStatusCode kHttpStatusCodeCreated = 201;
constexpr HttpStatusCode kHttpStatusCodeAccepted = 202;
constexpr HttpStatusCode kHttpStatusCodeNoContent = 204;

inline bool HttpStatusCode_isInformational(
        int statusCode) {
    return statusCode >= 100 && statusCode < 200;
}

inline bool HttpStatusCode_isSuccess(
        int statusCode) {
    return statusCode >= 200 && statusCode < 300;
}

inline bool HttpStatusCode_isRedirection(
        int statusCode) {
    return statusCode >= 300 && statusCode < 400;
}

inline bool HttpStatusCode_isClientError(
        int statusCode) {
    return statusCode >= 400 && statusCode < 500;
}

inline bool HttpStatusCode_isServerError(
        int statusCode) {
    return statusCode >= 500 && statusCode < 600;
}

inline bool HttpStatusCode_isCustomError(
        int statusCode) {
    return statusCode >= 900 && statusCode < 1000;
}

inline bool HttpStatusCode_isError(
        int statusCode) {
    return HttpStatusCode_isClientError(statusCode) ||
            HttpStatusCode_isServerError(statusCode) ||
            HttpStatusCode_isCustomError(statusCode);
}

inline bool HttpStatusCode_isValid(
        int statusCode) {
    return (statusCode >= 100 && statusCode < 600) ||
            HttpStatusCode_isCustomError(statusCode);
}

} // namespace network

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::network::HttpStatusCode);
