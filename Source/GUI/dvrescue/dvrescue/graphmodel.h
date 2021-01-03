#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include <QObject>
#include <QThread>
#include <QPair>
#include <QSet>
#include <xmlparser.h>

class QwtQuick2PlotCurve;

class GraphModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int total READ total NOTIFY totalChanged)

public:
    explicit GraphModel(QObject *parent = nullptr);
    ~GraphModel();

    int total() const;

    Q_INVOKABLE QString videoInfo(float x, float y);
    Q_INVOKABLE QString audioInfo(float x, float y);

    struct GraphStats {
        int frameNumber;
        float evenValue;
        float oddValue;
        float den;
    } Stats;

    void getInfo(QList<std::tuple<int, GraphStats>>& stats, float x, float y, int& frame, float& oddValue, float& evenValue);
    void getVideoInfo(float x, float y, int& frame, float& oddValue, float& evenValue);
    void getAudioInfo(float x, float y, int& frame, float& oddValue, float& evenValue);

public Q_SLOTS:
    void update(QwtQuick2PlotCurve *videoCurve, QwtQuick2PlotCurve* videoCurve2, QwtQuick2PlotCurve *audioCurve, QwtQuick2PlotCurve* audioCurve2);
    void reset(QwtQuick2PlotCurve *videoCurve, QwtQuick2PlotCurve* videoCurve2, QwtQuick2PlotCurve *audioCurve, QwtQuick2PlotCurve* audioCurve2);
    void populate(const QString& fileName);

Q_SIGNALS:
    void populated();
    void totalChanged(int total);

private:
    XmlParser* m_parser { nullptr };
    std::unique_ptr<QThread> m_thread;

    QList<std::tuple<int, GraphStats>> m_videoValues;
    QList<std::tuple<int, GraphStats>> m_audioValues;
    int m_lastFrame { 0 };
    int m_total { 0 };
};

#endif // GRAPHMODEL_H
