#include "sharedcache.h"

namespace {

QString secureHash(const QString &group, const QString &hash) {
    QString s = group.toUpper() + hash.mid(group.length()) + hash + group.right(1);
    return QCryptographicHash::hash(s.toUtf8(), QCryptographicHash::Sha1).toHex();
}

} // namespace

QString SharedCache::hash(const QString &s) {
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(s.toUtf8());
    QByteArray hashBytes = hash.result();
    QByteArray bytes = QByteArray::number(*(qlonglong *)hashBytes.constData(), 36);
    return QString::fromLatin1(bytes);
}

SharedCache::SharedCache(QObject *parent) : QObject(parent) {
    http.setDefaultReadTimeout(5000);
    http.setMaxRetries(0);
    http.getRequestHeaders().insert("User-Agent", "C");
}

HttpReply *SharedCache::value(const QString &key) {
    QString url = baseUrl + group + QLatin1Char('/') + key.at(0) + QLatin1Char('/') + key.at(1) +
                  QLatin1Char('/') + key.mid(2);
    return http.get(url);
}

HttpReply *
SharedCache::insert(const QString &key, const QByteArray &value, const QByteArray &contentType) {
    QUrl url(baseUrl);

    QUrlQuery q;
    q.addQueryItem(QStringLiteral("g"), group);
    q.addQueryItem(QStringLiteral("h"), key);
    q.addQueryItem(QStringLiteral("s"), secureHash(group, key));
    // TODO e == gz
    url.setQuery(q);

    if (value.size() > 1014 * 1024) {
        qWarning() << "Not caching >1MB response for" << key;
        return nullptr;
    }

    auto reply = http.post(url, value, contentType);
    connect(reply, &HttpReply::finished, this, [url](auto &reply) {
        if (reply.statusCode() != 200)
            qDebug() << "Failed to cache" << url << reply.statusCode() << reply.reasonPhrase();
    });

    return reply;
}
