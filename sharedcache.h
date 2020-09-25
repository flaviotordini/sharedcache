#ifndef SHAREDCACHE_H
#define SHAREDCACHE_H

#include "http.h"
#include <QtCore>

class SharedCache : public QObject {
    Q_OBJECT

public:
    static QString hash(const QString &s);

    void setGroup(const QString &name) { group = name; }
    const QString &getGroup() { return group; }

    void setBaseUrl(const QString &url) { baseUrl = url; }
    const QString &getBaseUrl() { return baseUrl; }

    explicit SharedCache(QObject *parent = nullptr);
    HttpReply *value(const QString &key);
    HttpReply *insert(const QString &key, const QByteArray &value, const QByteArray &contentType);

private:
    QString baseUrl;
    QString group;
    Http http;
};

#endif // SHAREDCACHE_H
