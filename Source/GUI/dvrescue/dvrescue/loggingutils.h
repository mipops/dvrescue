#ifndef LOGGINGUTILS_H
#define LOGGINGUTILS_H

#include <QObject>
#include <QLoggingCategory>

class LoggingUtils : public QObject
{
    Q_OBJECT
public:
    explicit LoggingUtils(QObject *parent = nullptr);

    static void installFilter();
    static void uninstallFilter();

    static const QList<QLoggingCategory*>& allCategories() {
        return categories;
    }

    Q_INVOKABLE bool isDebugEnabled(const QString& category) const;

private:
    static void categoryFilter(QLoggingCategory* category);

    static QMap<QString, QLoggingCategory*> categoryByName;
    static QList<QLoggingCategory*> categories;

Q_SIGNALS:

};

#endif // LOGGINGUTILS_H
