#include "syncupdater.h"

#include <QHostAddress>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

SyncUpdater::SyncUpdater(const QSharedPointer<CoreObject> &coreObject)
{
    // create local access
    this->_coreObject = coreObject;
}

void SyncUpdater::mergeExternalList()
{
    const auto &list = _coreObject->Settings.ExternalListSettings;
    const QUrl url(list.url);

    if ( ! url.isValid() || (url.scheme() != "http" and url.scheme() != "https") )
    {
        _coreObject->Log.logEvent("warning", QStringLiteral("invalid external server list URL: %1").arg(list.url));
        return;
    }

    if ( ! _coreObject->SupportedGames.contains(list.gameName) )
    {
        _coreObject->Log.logEvent("warning", QStringLiteral("external server list uses unsupported game %1").arg(list.gameName));
        return;
    }

    QNetworkRequest request(url);
    QNetworkReply *reply = _networkManager.get(request);
    const QString gameName = list.gameName;

    connect(reply, &QNetworkReply::finished, this, [this, reply, gameName]
    {
        if ( reply->error() != QNetworkReply::NoError )
        {
            _coreObject->Log.logEvent("warning", QStringLiteral("unable to download external server list: %1")
                                      .arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        int merged = 0;
        QSqlDatabase::database().transaction();

        // The first two columns are IP and gamespy port; ignore hostport and all remaining fields.
        const QList<QByteArray> lines = reply->readAll().split('\n');
        for (const QByteArray &line : lines)
        {
            const QList<QByteArray> fields = line.trimmed().split('\\');
            if ( fields.size() < 2 )
                continue;

            const QString ip = QString::fromLatin1(fields.at(0)).trimmed();
            bool validPort = false;
            const unsigned short gamespyPort = fields.at(1).trimmed().toUShort(&validPort);
            if ( ! validPort || gamespyPort == 0 || QHostAddress(ip).isNull() )
                continue;

            if ( updateServer(ip, gamespyPort, gameName, false, false) ||
                 insertServer(ip, gamespyPort, gameName, false) )
                merged++;
        }

        QSqlDatabase::database().commit();
        _coreObject->Log.logEvent("sync", QStringLiteral("merged %1 servers from external list").arg(merged));
        reply->deleteLater();
    });
}
