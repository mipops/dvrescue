#ifndef SETTINGSUTILS_H
#define SETTINGSUTILS_H

#include <QObject>

class SettingsUtils : public QObject
{
    Q_OBJECT
public:
    explicit SettingsUtils(QObject *parent = nullptr);

    Q_INVOKABLE QStringList keys() const;
Q_SIGNALS:

};

#endif // SETTINGSUTILS_H
