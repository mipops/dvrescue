#include "settingsutils.h"
#include <QSettings>

SettingsUtils::SettingsUtils(QObject *parent) : QObject(parent)
{

}

QStringList SettingsUtils::keys() const
{
    QSettings settings;
    return settings.allKeys();
}
