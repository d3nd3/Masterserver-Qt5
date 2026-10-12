#include "syncupdater.h"

void SyncUpdater::onSyncTickerAction()
{
    // exists?
    if ( _syncIndex < _coreObject->Settings.SyncerSettings.syncServers.length() )
    {
        // retrieve sync settings from config
        SyncServer syncServ = _coreObject->Settings.SyncerSettings.syncServers.at(_syncIndex);
        _syncIndex++;

        // execute sync with remote masterserver
        _coreObject->Log.logEvent("sync", QStringLiteral("[SYNC-TRACE] starting outgoing sync to %1:%2")
                                  .arg(syncServ.remoteAddress, QString::number(syncServ.listenPort)));
        new SyncClient(_coreObject, syncServ.remoteAddress, syncServ.listenPort);
    }
    else
    {
        // last entry. turn off timer
        if ( _coreObject->Settings.SyncerSettings.syncServers.isEmpty() )
            _coreObject->Log.logEvent("warning", QStringLiteral("[SYNC-TRACE] no outgoing sync peers are configured"));
        _syncTicker.stop();
    }
}
