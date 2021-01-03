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
    void parseFrames(QXmlStreamReader& xml, const QString& frameSize, const QString& chroma_subsampling);

Q_SIGNALS:
    void bytesProcessed(qint64 value);
    void finished();
    void gotFrame(uint frameNumber);
    void gotSta(uint frameNumber, uint t, uint n, uint n_even, float den);
    void gotAud(uint frameNumber, uint t, uint n, uint n_even, float den);
};

#endif // XMLPARSER_H
