#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <QIODevice>
#include <QObject>

class QXmlStreamReader;
class XmlParser : public QObject
{
    Q_OBJECT
public:
    explicit XmlParser(QObject *parent = nullptr);

    void exec(QIODevice* device);
    void exec(const QString& path);

private:
    void parseMedia(QXmlStreamReader& xml);
    void parseFrames(QXmlStreamReader& xml);

Q_SIGNALS:
    void bytesProcessed(qint64 value);
    void gotFrame(uint frameNumber);
    void gotSta(uint frameNumber, uint t, uint n, uint n_even);
    void gotAud(uint frameNumber, uint t, uint n, uint n_even);
};

#endif // XMLPARSER_H
