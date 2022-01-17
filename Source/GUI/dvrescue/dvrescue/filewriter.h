#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <QFile>
#include <QObject>

class FileWriter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)

    QString m_fileName;
    QFile file;

public:
    explicit FileWriter(QObject *parent = nullptr);
    ~FileWriter();

    Q_INVOKABLE void write(const QByteArray& bytes);

    QString fileName() const;
    void setFileName(const QString &newFileName);

    Q_INVOKABLE bool open();
    Q_INVOKABLE void close();

Q_SIGNALS:
    void fileNameChanged();
};

#endif // FILEWRITER_H
