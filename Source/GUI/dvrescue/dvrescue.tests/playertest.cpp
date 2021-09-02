#include "playertest.h"
#include <QTest>
#include <QThread>
#include <QtAVPlayer/QtAVPlayer>

PlayerTest::PlayerTest(QObject *parent)
{
    qRegisterMetaType<QAVAudioFrame>();
}

void PlayerTest::test()
{
    QAVPlayer p;

    QFileInfo file(path());
    p.setSource(QUrl::fromLocalFile(file.absoluteFilePath()));

    QAVVideoFrame frame;
    QObject::connect(&p, &QAVPlayer::videoFrame, &p, [&frame](const QAVVideoFrame &f) { frame = f; });

    p.play();
    QTRY_VERIFY(frame);

    auto mapData = frame.map();
    QVERIFY(mapData.size > 0);
    QVERIFY(mapData.bytesPerLine[0] > 0);
    QVERIFY(mapData.bytesPerLine[1] > 0);
    QVERIFY(mapData.data[0] != nullptr);
    QVERIFY(mapData.data[1] != nullptr);
}

void PlayerTest::testMultipleSources()
{
    qDebug() << "testMultipleSources";

    QAVPlayer p;
    QAVAudioOutput out;
    QObject::connect(&p, &QAVPlayer::videoFrame, &p, [this](const QAVVideoFrame &f) {
        qDebug() << "got video frame";
    });

    QObject::connect(&p, &QAVPlayer::audioFrame, &p, [this, &out](const QAVAudioFrame &f) {
        qDebug() << "got audio frame";
        out.play(f);
    });

    QFileInfo file(path());
    qDebug() << "setSource: " << QUrl::fromLocalFile(file.absoluteFilePath());
    p.setSource(QUrl::fromLocalFile(file.absoluteFilePath()));
    p.play();
    QTest::qWait(100);

    QFileInfo file2(path2());

    qDebug() << "setSource: " << QUrl::fromLocalFile(file2.absoluteFilePath());
    p.setSource(QUrl::fromLocalFile(file2.absoluteFilePath()));
    p.play();
    QTest::qWait(100);
}

const QString &PlayerTest::path() const
{
    return m_path;
}

void PlayerTest::setPath(const QString &newPath)
{
    if (m_path == newPath)
        return;
    m_path = newPath;
    Q_EMIT pathChanged();
}

const QString &PlayerTest::path2() const
{
    return m_path2;
}

void PlayerTest::setPath2(const QString &newPath2)
{
    if (m_path2 == newPath2)
        return;
    m_path2 = newPath2;
    Q_EMIT pathChanged();
}
