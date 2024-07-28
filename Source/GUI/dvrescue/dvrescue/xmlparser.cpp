#include "xmlparser.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QThread>
#include <QDebug>
#include <QTime>

XmlParser::XmlParser(QObject *parent) : QObject(parent)
{

}

void XmlParser::exec(QIODevice *device)
{
    try {

        QXmlStreamReader xml(device);
        if(xml.readNextStartElement())
        {
            if(xml.name() == QString("dvrescue"))
            {
                if(xml.readNextStartElement())
                {
                    qDebug() << "name: " << xml.name();

                    if(xml.name() == QString("creator")) {

                    }

                    xml.skipCurrentElement();
                }

                if(xml.readNextStartElement())
                {
                    qDebug() << "name: " << xml.name();

                    if(xml.name() == QString("media")) {
                        parseMedia(xml);
                    }

                    xml.skipCurrentElement();
                }
            }
        }

    }

    catch(std::exception& ex) {
        Q_EMIT error(ex.what());
    }

    catch(...) {
        Q_EMIT error("unexpected error");
    }

    Q_EMIT finished();
}

void XmlParser::exec(const QString &path)
{
    QFile file(path);
    if(file.open(QFile::ReadOnly))
    {
        exec(&file);
    }
}

void XmlParser::parseMedia(QXmlStreamReader &xml)
{
    QString ref;
    QString format;
    int size = 0;
    for(auto & attr : xml.attributes()) {
        if(attr.name() == QString("ref"))
            ref = attr.value().toString();
        else if(attr.name() == QString("format"))
            format = attr.value().toString();
        else if(attr.name() == QString("size"))
            size = attr.value().toInt();
    }

    Q_EMIT gotMedia(ref, format, size);

    bool firstFrames = true;
    while(xml.readNextStartElement())
    {
        if(xml.name() == QString("frames"))
        {
            auto attributes = xml.attributes();
            parseFrames(xml, attributes, firstFrames);

            if(firstFrames)
                firstFrames = false;
        }

        xml.skipCurrentElement();
    }
}

void XmlParser::parseFrames(QXmlStreamReader &xml, QXmlStreamAttributes& framesAttributes, bool firstFrames)
{
    QString size;
    QString chroma_subsampling;
    double frameDuration = 0;
    int count = 0;

    for(auto & attribute : framesAttributes) {
        if(attribute.name() == QString("size"))
            size = attribute.value().toString();
        else if(attribute.name() == QString("chroma_subsampling"))
            chroma_subsampling = attribute.value().toString();
        else if(attribute.name() == QString("video_rate")) {
            auto value = attribute.value().toString();
            if(value.contains("/")) {
                auto splitted = value.split("/");
                frameDuration = splitted[1].toDouble() / splitted[0].toDouble();
            } else {
                frameDuration = value.toULongLong() / 1000;
            }
        }
        else if(attribute.name() == QString("count"))
            count = attribute.value().toUInt();
    }

    // magic from Dave
    auto diff_seq_count = 12;
    if(size == "720x576" && chroma_subsampling != "4:2:2")
        diff_seq_count = 12;
    else if(size == "720x480" && chroma_subsampling != "4:2:2")
        diff_seq_count = 10;
    else if(size == "720x576" && chroma_subsampling == "4:2:2")
        diff_seq_count = 24;
    else if(size == "720x480" && chroma_subsampling == "4:2:2")
        diff_seq_count = 20;

    auto video_error_den = diff_seq_count * video_blocks_per_diff_seq;
    auto audio_error_den = diff_seq_count * audio_blocks_per_diff_seq;

    Q_EMIT gotFrames(count, diff_seq_count);

    auto captionOn = false;
    auto firstFrame = true;

    while(xml.readNextStartElement())
    {
        if(xml.name() == QString("frame")) {
            // qDebug() << "frame";

            auto frameNumber = 0;
            auto frameOffset = 0.0;
            auto frameAttributes = xml.attributes();
            for(auto & attribute : frameAttributes) {
                // qDebug() << "\t" << attribute.name() << "=" << attribute.value();
                if(attribute.name() == QString("n"))
                    frameNumber = attribute.value().toUInt();
                else if(attribute.name() == QString("pts")) {
                    auto value = attribute.value().toString();
                    auto splitted = value.split(":");

                    auto s = splitted[2].toDouble();
                    auto m = splitted[1].toLongLong();
                    auto h = splitted[0].toLongLong();

                    auto sms = s * 1000;
                    auto mms = m * 60 * 1000;
                    auto hms = h * 60 * 60 * 1000;

                    frameOffset = hms + mms + sms;
                }
            }

            if(framesAttributes.hasAttribute("captions") && framesAttributes.value("captions") == QString("p"))
            {
                if(frameAttributes.hasAttribute("caption"))
                {
                    auto caption = frameAttributes.value("caption");
                    if(caption == QString("on")) {
                        captionOn = true;
                    } else if(caption == QString("off")) {
                        captionOn = false;
                    }
                }
            }

            Q_EMIT gotFrame(frameNumber, frameOffset, frameDuration);

            int staCount = 0;
            int totalSta = 0;
            int totalEvenSta = 0;

            int totalAud = 0;
            int totalAudSta = 0;

            while(xml.readNextStartElement())
            {
                if(xml.name() == QString("dseq")) {
                    // qDebug() << "\t\t" << "dseq";

                    while(xml.readNextStartElement()) {
                        if(xml.name() == QString("sta")) {
                            // qDebug() << "\t\t\t" << "sta";
                        }
                        xml.skipCurrentElement();
                    }
                } else if(xml.name() == QString("sta")) {
                    // qDebug() << "\t\t" << "sta";

                    int t = 0;
                    int n = 0;
                    int n_even = 0;

                    for(auto & attribute : xml.attributes()) {
                        if(attribute.name() == QString("t"))
                            t = attribute.value().toUInt();
                        else if(attribute.name() == QString("n"))
                            n = attribute.value().toUInt();
                        else if(attribute.name() == QString("n_even"))
                            n_even = attribute.value().toUInt();
                    }

                    ++staCount;
                    totalSta += n;
                    totalEvenSta += n_even;
                    Q_EMIT gotSta(frameNumber, t, n, n_even, video_error_den);

                    xml.skipCurrentElement();
                } else if(xml.name() == QString("aud")) {
                    // qDebug() << "\t\t" << "aud";

                    int t = 0;
                    int n = 0;
                    int n_even = 0;

                    for(auto & attribute : xml.attributes()) {
                        if(attribute.name() == QString("t"))
                            t = attribute.value().toUInt();
                        else if(attribute.name() == QString("n"))
                            n = attribute.value().toUInt();
                        else if(attribute.name() == QString("n_even"))
                            n_even = attribute.value().toUInt();
                    }

                    totalAud += n;
                    totalAudSta += n_even;
                    Q_EMIT gotAud(frameNumber, t, n, n_even, audio_error_den);

                    xml.skipCurrentElement();
                }
            }

            auto isSubstantial = !firstFrames && firstFrame;

            Q_EMIT gotFrameAttributes(frameNumber, framesAttributes, frameAttributes, diff_seq_count, staCount, totalSta, totalEvenSta, totalAud, totalAudSta, captionOn, isSubstantial);
            Q_EMIT bytesProcessed(xml.device()->pos());

            if(firstFrame)
                firstFrame = false;
        }
    }
}
