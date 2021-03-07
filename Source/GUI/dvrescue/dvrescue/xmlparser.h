#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <QIODevice>
#include <QObject>

constexpr auto video_blocks_per_diff_seq = 135;
constexpr auto audio_blocks_per_diff_seq = 9;

class QXmlStreamReader;
class QXmlStreamAttributes;
class XmlParser : public QObject
{
    Q_OBJECT
public:
    explicit XmlParser(QObject *parent = nullptr);

    void exec(QIODevice* device);
    void exec(const QString& path);

private:
    void parseMedia(QXmlStreamReader& xml);
    void parseFrames(QXmlStreamReader& xml, QXmlStreamAttributes& framesAttributes, bool firstFrames);

Q_SIGNALS:
    void bytesProcessed(qint64 value);
    void finished();
    void gotMedia(QString ref, QString format, int size);
    void gotFrame(uint frameNumber);
    void gotFrames(uint count, uint diff_per_seq_count);
    void gotFrameAttributes(uint frameNumber, const QXmlStreamAttributes& framesAttributes, const QXmlStreamAttributes& frameAttributes,
                            int diff_seq_count, int staCount, int totalSta, int totalEvenSta, int totalAud, int totalEvenAud, bool captionOn, bool isSubstantial);
    void gotSta(uint frameNumber, uint t, uint n, uint n_even, float den);
    void gotAud(uint frameNumber, uint t, uint n, uint n_even, float den);
};

#endif // XMLPARSER_H
