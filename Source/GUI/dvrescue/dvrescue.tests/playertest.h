#ifndef PLAYERTEST_H
#define PLAYERTEST_H

#include <QObject>

class PlayerTest : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QString path2 READ path2 WRITE setPath2 NOTIFY pathChanged)
    QString m_path;

    QString m_path2;

public:
    explicit PlayerTest(QObject *parent = nullptr);

    const QString &path() const;
    void setPath(const QString &newPath);

    const QString &path2() const;
    void setPath2(const QString &newPath2);

private Q_SLOTS:
    void test();
    void testMultipleSources();

Q_SIGNALS:

    void pathChanged();
};

#endif // PLAYERTEST_H
