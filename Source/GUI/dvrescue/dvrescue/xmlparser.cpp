#include "xmlparser.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QThread>
#include <QDebug>

XmlParser::XmlParser(QObject *parent) : QObject(parent)
{

}

void XmlParser::exec(QIODevice *device)
{
    try {

        QXmlStreamReader xml(device);
        if(xml.readNextStartElement())
        {
            if(xml.name() == "dvrescue")
            {
                if(xml.readNextStartElement())
                {
                    qDebug() << "name: " << xml.name();

                    if(xml.name() == "creator") {

                    }

                    xml.skipCurrentElement();
                }

                if(xml.readNextStartElement())
                {
                    qDebug() << "name: " << xml.name();

                    if(xml.name() == "media") {
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
        if(attr.name() == "ref")
            ref = attr.value().toString();
        else if(attr.name() == "format")
            format = attr.value().toString();
        else if(attr.name() == "size")
            size = attr.value().toInt();
    }

    Q_EMIT gotMedia(ref, format, size);

    bool firstFrames = true;
    while(xml.readNextStartElement())
    {
        if(xml.name() == "frames")
        {
            auto attributes = xml.attributes();
            parseFrames(xml, attributes, firstFrames);

            if(firstFrames)
                firstFrames = false;
        }
    }
}

void XmlParser::parseFrames(QXmlStreamReader &xml, QXmlStreamAttributes& framesAttributes, bool firstFrames)
{
    QString size;
    QString chroma_subsampling;
    int count = 0;

    for(auto & attribute : framesAttributes) {
        if(attribute.name() == "size")
            size = attribute.value().toString();
        else if(attribute.name() == "chroma_subsampling")
            chroma_subsampling = attribute.value().toString();
        else if(attribute.name() == "count")
            count = attribute.value().toUInt();
    }

    // magic from Dave
    auto diff_seq_count = 0;
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

    assert(diff_seq_count != 0);
    if(diff_seq_count == 0)
        throw std::exception("diff_seq_count = 0");

    auto captionOn = false;
    auto firstFrame = true;

    while(xml.readNextStartElement())
    {
        if(xml.name() == "frame") {
            // qDebug() << "frame";

            auto frameNumber = 0;
            auto frameAttributes = xml.attributes();
            for(auto & attribute : frameAttributes) {
                // qDebug() << "\t" << attribute.name() << "=" << attribute.value();
                if(attribute.name() == "n")
                    frameNumber = attribute.value().toUInt();
            }

            if(framesAttributes.hasAttribute("captions") && framesAttributes.value("captions") == "p")
            {
                if(frameAttributes.hasAttribute("caption"))
                {
                    auto caption = frameAttributes.value("caption");
                    if(caption == "on") {
                        captionOn = true;
                    } else if(caption == "off") {
                        captionOn = false;
                    }
                }
            }

            Q_EMIT gotFrame(frameNumber);

            int staCount = 0;
            int totalSta = 0;
            int totalEvenSta = 0;

            int totalAud = 0;
            int totalAudSta = 0;

            while(xml.readNextStartElement())
            {
                if(xml.name() == "dseq") {
                    // qDebug() << "\t\t" << "dseq";

                    while(xml.readNextStartElement()) {
                        if(xml.name() == "sta") {
                            // qDebug() << "\t\t\t" << "sta";
                        }
                        xml.skipCurrentElement();
                    }
                } else if(xml.name() == "sta") {
                    // qDebug() << "\t\t" << "sta";

                    int t = 0;
                    int n = 0;
                    int n_even = 0;

                    for(auto & attribute : xml.attributes()) {
                        if(attribute.name() == "t")
                            t = attribute.value().toUInt();
                        else if(attribute.name() == "n")
                            n = attribute.value().toUInt();
                        else if(attribute.name() == "n_even")
                            n_even = attribute.value().toUInt();
                    }

                    ++staCount;
                    totalSta += n;
                    totalEvenSta += n_even;
                    Q_EMIT gotSta(frameNumber, t, n, n_even, video_error_den);

                    xml.skipCurrentElement();
                } else if(xml.name() == "aud") {
                    // qDebug() << "\t\t" << "aud";

                    int t = 0;
                    int n = 0;
                    int n_even = 0;

                    for(auto & attribute : xml.attributes()) {
                        if(attribute.name() == "t")
                            t = attribute.value().toUInt();
                        else if(attribute.name() == "n")
                            n = attribute.value().toUInt();
                        else if(attribute.name() == "n_even")
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
