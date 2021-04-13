#ifndef DATAMODEL_H
#define DATAMODEL_H

#include <QObject>
#include <QThread>
#include <QPair>
#include <QSet>
#include <QMap>
#include <QVariant>
#include <xmlparser.h>

class QwtQuick2PlotCurve;
class QAbstractTableModel;
class QJSEngine;
class QXmlStreamAttributes;
class QXmlStreamAttributes;

struct MarkerInfo {
    Q_GADGET
    Q_PROPERTY(int frameNumber MEMBER frameNumber)
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QString type MEMBER type)
    Q_PROPERTY(QString icon MEMBER icon)
    Q_PROPERTY(QString recordingTime MEMBER recordingTime)
    Q_PROPERTY(QString timecode MEMBER timecode)

public:
    MarkerInfo() {
    }

    int frameNumber;
    QString type;
    QString name;
    QString icon;
    QString recordingTime;
    QString timecode;
};

Q_DECLARE_METATYPE(MarkerInfo);

class DataModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int total READ total NOTIFY totalChanged)

public:
    explicit DataModel(QObject *parent = nullptr);
    ~DataModel();

    int total() const;

    Q_INVOKABLE QString videoInfo(float x, float y);
    Q_INVOKABLE QString audioInfo(float x, float y);
    Q_INVOKABLE QVariantList getMarkers();
    Q_INVOKABLE int frameByIndex(int index);
    Q_INVOKABLE bool isSubstantialFrame(int index);
    Q_INVOKABLE int getLastSubstantialFrame(int index);
    Q_INVOKABLE QString getLastSubstantialFrameTransition(int index);
    Q_INVOKABLE int rowByFrame(int frame);

    struct GraphStats {
        int frameNumber;
        float evenValue;
        float oddValue;
        float den;
    };

    struct FrameStats {
        bool isSubstantial { false };
        int lastSubstantialFrame { -1 };
        QMap<QString, std::pair<QString, QString>> markers;
        QString recordingTime;
        QString timecode;
        QString videoInfo;
        QString audioInfo;
    };

    void getInfo(QList<std::tuple<int, GraphStats>>& stats, float x, float y, int& frame, float& oddValue, float& evenValue);
    void getVideoInfo(float x, float y, int& frame, float& oddValue, float& evenValue);
    void getAudioInfo(float x, float y, int& frame, float& oddValue, float& evenValue);

    static void setEngine(QJSEngine* jsEngine);

public Q_SLOTS:
    void update(QwtQuick2PlotCurve *videoCurve, QwtQuick2PlotCurve* videoCurve2, QwtQuick2PlotCurve *audioCurve, QwtQuick2PlotCurve* audioCurve2);
    void reset(QwtQuick2PlotCurve *videoCurve, QwtQuick2PlotCurve* videoCurve2, QwtQuick2PlotCurve *audioCurve, QwtQuick2PlotCurve* audioCurve2);
    void bind(QAbstractTableModel* model);

    void populate(const QString& fileName);

private Q_SLOTS:
    void onGotFrame(int frameNumber, const QXmlStreamAttributes& framesAttributes, const QXmlStreamAttributes& frameAttributes, int diff_seq_count,
                    int totalSta, int totalEvenSta, int totalAud, int totalEvenAud, bool captionOn, bool isSubstantional);
    void onDataRowCreated(const QVariantMap& map);

Q_SIGNALS:
    void populated();
    void error(const QString& errorString);
    void totalChanged(int total);
    void updated();

    void clearModel();
    void gotDataRow(const QVariant& row);
    void dataRowCreated(const QVariantMap& map);

private:
    XmlParser* m_parser { nullptr };
    std::unique_ptr<QThread> m_thread;
    QAbstractTableModel* m_model { nullptr };

    QList<std::tuple<int, GraphStats>> m_videoValues;
    QList<std::tuple<int, GraphStats>> m_audioValues;
    QList<std::tuple<int, FrameStats>> m_frames;
    QMap<int, int> m_rowByFrame;

    int m_lastSubstantialFrame { -1 };
    int m_lastFrame { 0 };
    int m_total { 0 };
};

#endif // DATAMODEL_H
