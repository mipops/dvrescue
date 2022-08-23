#ifndef CAPTUREERRORPLOTDATAMODEL_H
#define CAPTUREERRORPLOTDATAMODEL_H


#include <QObject>
#include <QThread>
#include <QPair>
#include <QSet>
#include <QMap>
#include <QVariant>
#include <qwtquick2plot.h>
#include <xmlparser.h>

class QAbstractTableModel;
class QJSEngine;
class QXmlStreamAttributes;
class QXmlStreamAttributes;

class CaptureErrorPlotDataModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int total READ total NOTIFY totalChanged)
    Q_PROPERTY(QwtQuick2PlotCurve* evenCurve READ evenCurve WRITE setEvenCurve NOTIFY evenCurveChanged)
    Q_PROPERTY(QwtQuick2PlotCurve* oddCurve READ oddCurve WRITE setOddCurve NOTIFY oddCurveChanged)

public:
    explicit CaptureErrorPlotDataModel(QObject *parent = nullptr);
    ~CaptureErrorPlotDataModel();

    int total() const;

    struct GraphStats {
        int frameNumber;
        float evenValue;
        float oddValue;
    };

    QwtQuick2PlotCurve *evenCurve() const;
    void setEvenCurve(QwtQuick2PlotCurve *newEvenCurve);

    QwtQuick2PlotCurve *oddCurve() const;
    void setOddCurve(QwtQuick2PlotCurve *newOddCurve);

public Q_SLOTS:
    void update();
    void reset();

    void append(int frameNumber, float even, float odd);

Q_SIGNALS:
    void totalChanged();
    void updated();
    void clearModel();
    void evenCurveChanged();
    void oddCurveChanged();

private:

    QList<std::tuple<int, GraphStats>> m_values;
    int m_total { 0 };
    QwtQuick2PlotCurve *m_evenCurve = nullptr;
    QwtQuick2PlotCurve *m_oddCurve = nullptr;
};
#endif // CAPTUREERRORPLOTDATAMODEL_H
