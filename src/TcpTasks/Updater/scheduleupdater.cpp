#include "syncupdater.h"

bool SyncUpdater::scheduleUpdater()
{
    if ( _coreObject->Settings.SyncerSettings.doSync )
    {
        // schedule sync X seconds apart (prevent network/cpu spikes)
        connect(&_syncTicker, &QTimer::timeout, this, &SyncUpdater::onSyncTickerAction);
        _syncTicker.setInterval( _graceTime_ms );

        // set update timer
        connect(&_updaterTimer, &QTimer::timeout, [this]
        {
            // reset and start ticker
            _syncIndex = 0;
            _syncTicker.start();
        });

        _updaterTimer.setInterval( _coreObject->Settings.SyncerSettings.syncInterval_s * 1000);
        _updaterTimer.start();

        _coreObject->Log.logEvent("info", QStringLiteral("sync with other masterservers every %1 seconds")
                                .arg(_coreObject->Settings.SyncerSettings.syncInterval_s));

        // run immediately at startup
        _syncIndex = 0;
        _syncTicker.start();
    }

    if ( _coreObject->Settings.ExternalListSettings.doMerge )
    {
        connect(&_externalListTimer, &QTimer::timeout, this, &SyncUpdater::mergeExternalList);
        _externalListTimer.setInterval(_coreObject->Settings.ExternalListSettings.mergeInterval_s * 1000);
        _externalListTimer.start();

        _coreObject->Log.logEvent("info", QStringLiteral("merge external server list every %1 seconds")
                                .arg(_coreObject->Settings.ExternalListSettings.mergeInterval_s));
        mergeExternalList();
    }

    return true;
}
