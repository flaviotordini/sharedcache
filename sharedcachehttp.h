#ifndef CHTTP_H
#define CHTTP_H

#include "http.h"
#include <QtNetwork>

#include "sharedcache.h"

class SharedCacheHttp : public Http {
public:
    SharedCacheHttp(Http &http = Http::instance());
    HttpReply *request(const HttpRequest &req);

    void setBaseUrl(const QString &url);
    const QString &getBaseUrl() { return baseUrl; }

    void setGroup(const QString &name);
    const QString &getGroup() { return group; }

    void setCacheEmptyReplies(bool value) { cacheEmptyReplies = value; }
    bool getCacheEmptyReplies() { return cacheEmptyReplies; }

    SharedCache &getSharedCache() { return sharedCache; }

private:
    SharedCache sharedCache;
    Http &http;
    QString group;
    QString baseUrl;
    bool cacheEmptyReplies;
};

class SharedCacheHttpReply : public HttpReply {
    Q_OBJECT

public:
    SharedCacheHttpReply(SharedCacheHttp &chttp, Http &http, const HttpRequest &req);

    QUrl url() const { return QUrl(); }
    int statusCode() const { return -1; }
    QByteArray body() const { return QByteArray(); }

private:
    SharedCacheHttp &chttp;
    Http &http;
    HttpRequest req;
    QString hash;
};

#endif // CHTTP_H
