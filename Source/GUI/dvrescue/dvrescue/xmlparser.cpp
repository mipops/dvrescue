#include "xmlparser.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>

XmlParser::XmlParser(QObject *parent) : QObject(parent)
{

}

void XmlParser::exec(QIODevice *device)
{
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
    while(xml.readNextStartElement())
    {
        if(xml.name() == "frames")
        {
            parseFrames(xml);
        }
    }
}

void XmlParser::parseFrames(QXmlStreamReader &xml)
{
    while(xml.readNextStartElement())
    {
        if(xml.name() == "frame") {
            // qDebug() << "frame";

            auto frameNumber = 0;
            for(auto & attribute : xml.attributes()) {
                // qDebug() << "\t" << attribute.name() << "=" << attribute.value();
                if(attribute.name() == "n")
                    frameNumber = attribute.value().toUInt();
            }

            Q_EMIT gotFrame(frameNumber);

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

                    Q_EMIT gotSta(frameNumber, t, n, n_even);

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

                    Q_EMIT gotAud(frameNumber, t, n, n_even);

                    xml.skipCurrentElement();
                }
            }

            Q_EMIT bytesProcessed(xml.device()->pos());
        }
    }
}
