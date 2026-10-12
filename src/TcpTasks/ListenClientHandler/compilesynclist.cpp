#include "listenclienthandler.h"

QByteArray ListenClientHandler::compileSyncList(const QStringList &gamenameList,
                                                const int         &serverAge_s)
{
    // output list in \\gamename1\\ip:port ip:port\\gamename2\\ip:port ip:port\\final\\ format
    QByteArray compiledList;

    // go through list of gamenames
    QStringListIterator gamenameListNames(gamenameList);
    while ( gamenameListNames.hasNext() )
    {
        // retrieve servers from database (only verified, not from other syncs)
        QString gamename = gamenameListNames.next();
        QSqlQuery q = selectServerList( gamename, serverAge_s, false);
        if ( ! q.isActive() )
            _coreObject->Log.logEvent("warning", QStringLiteral("[SYNC-TRACE] cannot query export list for game=%1 to %2: %3")
                                      .arg(gamename, _clientLabel, q.lastError().text()));

        // identifier
        compiledList += QStringLiteral("\\%1\\").arg(gamename);
        int exported = 0;

        // iterate through resulting queries and add to compile list
        // ip-addresses are stored as text and do not need to be converted back and forth
        while (q.next())
        {
            // add to list -> ip:port
            const QString ip = q.value(0).toString();
            const QString port = q.value(1).toString();
            compiledList += QStringLiteral("%1:%2 ").arg(ip, port);
            _coreObject->Log.logEvent("sync", QStringLiteral("[SYNC-TRACE] exporting %1:%2 game=%3 to %4")
                                      .arg(ip, port, gamename, _clientLabel));
            exported++;

        } // while next

        _coreObject->Log.logEvent("sync", QStringLiteral("[SYNC-TRACE] export summary game=%1 count=%2 to %3")
                                  .arg(gamename, QString::number(exported), _clientLabel));
    }

    // terminator after list
    compiledList.append("\\final\\");

    // list compiled
    return compiledList;
}
