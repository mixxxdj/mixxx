/**
  @file
  @author Stefan Frings
*/

#ifndef HTTPCOOKIE_H
#define HTTPCOOKIE_H

#include <QList>
#include <QByteArray>
#include "httpglobal.h"

namespace stefanfrings {

/**
  HTTP cookie as defined in RFC 2109.
  Supports some additional attributes of RFC6265bis.
*/

class DECLSPEC HttpCookie
{
public:

    /** Creates an empty cookie */
    HttpCookie();

    /**
      Create a cookie and set name/value pair.
      @param name name of the cookie
      @param value value of the cookie
      @param maxAge maximum age of the cookie in seconds. 0=discard immediately
      @param path Path for that the cookie will be sent, default="/" which means the whole domain
      @param comment Optional comment, may be displayed by the web browser somewhere
      @param domain Optional domain for that the cookie will be sent. Defaults to the current domain
      @param secure If true, the cookie will be sent by the browser to the server only on secure connections
      @param httpOnly If true, the browser does not allow client-side scripts to access the cookie
      @param sameSite Declare if the cookie can only be read by the same site, which is a stronger
             restriction than the domain. Allowed values: "Lax" and "Strict".
    */
    HttpCookie(const QByteArray name, const QByteArray value, const int maxAge,
               const QByteArray path="/", const QByteArray comment=QByteArray(),
               const QByteArray domain=QByteArray(), const bool secure=false,
               const bool httpOnly=false, const QByteArray sameSite=QByteArray());

    /**
      Create a cookie from a string.
      @param source String as received in a HTTP Cookie2 header.
    */
    HttpCookie(const QByteArray source);

    /** Convert this cookie to a string that may be used in a Set-Cookie header. */
    QByteArray toByteArray() const ;

    /**
      Split a string list into parts, where each part is delimited by semicolon.
      Semicolons within double quotes are skipped. Double quotes are removed.
    */
    static QList<QByteArray> splitCSV(const QByteArray source);

    /** Set the name of this cookie */
    void setName(const QByteArray name);

    /** Set the value of this cookie */
    void setValue(const QByteArray value);

    /** Set the comment of this cookie */
    void setComment(const QByteArray comment);

    /** Set the domain of this cookie */
    void setDomain(const QByteArray domain);

    /** Set the maximum age of this cookie in seconds. 0=discard immediately */
    void setMaxAge(const int maxAge);

    /** Set the path for that the cookie will be sent, default="/" which means the whole domain */
    void setPath(const QByteArray path);

    /** Set secure mode, so that the cookie will be sent by the browser to the server only on secure connections */
    void setSecure(const bool secure);

    /** Set HTTP-only mode, so that the browser does not allow client-side scripts to access the cookie */
    void setHttpOnly(const bool httpOnly);

    /**
     * Set same-site mode, so that the browser does not allow other web sites to access the cookie.
     * Allowed values: "Lax" and "Strict".
     */
    void setSameSite(const QByteArray sameSite);

    /** Get the name of this cookie */
    QByteArray getName() const;

    /** Get the value of this cookie */
    QByteArray getValue() const;

    /** Get the comment of this cookie */
    QByteArray getComment() const;

    /** Get the domain of this cookie */
    QByteArray getDomain() const;

    /** Get the maximum age of this cookie in seconds. */
    int getMaxAge() const;

    /** Set the path of this cookie */
    QByteArray getPath() const;

    /** Get the secure flag of this cookie */
    bool getSecure() const;

    /** Get the HTTP-only flag of this cookie */
    bool getHttpOnly() const;

    /** Get the same-site flag of this cookie */
    QByteArray getSameSite() const;

    /** Returns always 1 */
    int getVersion() const;

private:

    QByteArray name;
    QByteArray value;
    QByteArray comment;
    QByteArray domain;
    int maxAge;
    QByteArray path;
    bool secure;
    bool httpOnly;
    QByteArray sameSite;
    int version;

};

} // end of namespace

#endif // HTTPCOOKIE_H
