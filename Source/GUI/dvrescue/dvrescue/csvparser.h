#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <QDateTime>
#include <QObject>

class CsvParser : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList columns READ columns WRITE setColumns NOTIFY columnsChanged)
    QStringList m_columns;

public:
    explicit CsvParser(QObject *parent = nullptr);

    const QStringList &columns() const;
    void setColumns(const QStringList &newColumns);

public Q_SLOTS:
    void write(const QByteArray& data);
    void write(const QString& data);

protected:
    void onNewEntry(const QString& entry);

Q_SIGNALS:
    void entriesReceived(const QStringList& entries);
    void columnsChanged(const QStringList& columns);

private:
    QString m_buffer;
};

#endif // CSVPARSER_H
