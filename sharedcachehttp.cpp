#include "sharedcachehttp.h"
#include "http.h"

#include "sharedcache.h"

namespace {

QString requestHash(const HttpRequest &req) {
    const char sep = '|';

    QString s;

    switch (req.operation) {
    case QNetworkAccessManager::GetOperation:
        s = QStringLiteral("GET");
        break;

    case QNetworkAccessManager::HeadOperation:
        s = QStringLiteral("HEAD");
        break;

    case QNetworkAccessManager::PostOperation:
        s = QStringLiteral("POST");
        break;

    default:
        qWarning() << "Unknown operation:" << req.operation;
    }

    s += sep + req.url.toString();

    if (!req.body.isEmpty()) s += sep + req.body;

    if (req.offset) s += sep + QString::number(req.offset);

    return SharedCache::hash(s);
}

} // namespace

SharedCacheHttp::SharedCacheHttp(Http &http) : http(http), cacheEmptyReplies(false) {}

HttpReply *SharedCacheHttp::request(const HttpRequest &req) {
    return new SharedCacheHttpReply(*this, http, req);
}

void SharedCacheHttp::setBaseUrl(const QString &url) {
    baseUrl = url;
    sharedCache.setBaseUrl(baseUrl);
}

void SharedCacheHttp::setGroup(const QString &name) {
    group = name;
    sharedCache.setGroup(group);
}

SharedCacheHttpReply::SharedCacheHttpReply(SharedCacheHttp &chttp,
                                           Http &http,
                                           const HttpRequest &req)
    : chttp(chttp), http(http), req(req) {
    hash = requestHash(req);
    auto reply = chttp.getSharedCache().value(hash);
    connect(reply, &HttpReply::finished, this, [this](auto &reply) {
        if (reply.statusCode() == 200) {
            qDebug() << "HIT" << this->chttp.getGroup() << hash;
            emit data(reply.body());
            emit finished(reply);
            deleteLater();
            return;
        } else {
            qDebug() << reply.statusCode() << reply.reasonPhrase() << this->req.url;
        }
        auto originReply = this->http.request(this->req);
        connect(originReply, &HttpReply::finished, this, [this](auto &reply) {
            bool success = reply.isSuccessful();
            if (success)
                emit data(reply.body());
            else
                emit error(reply.reasonPhrase());
            emit finished(reply);

            if (!success) {
                deleteLater();
                return;
            }

            const QByteArray body = reply.body();
            if (body.isEmpty() && !this->chttp.getCacheEmptyReplies()) {
                qWarning() << "Not caching empty response for" << reply.url();
                deleteLater();
                return;
            }

            auto putReply = this->chttp.getSharedCache().insert(hash.toUtf8(), body,
                                                                reply.header("Content-Type"));
            if (putReply)
                setParent(putReply);
            else
                deleteLater();
        });
    });
}
