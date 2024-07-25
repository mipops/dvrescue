#ifndef CAPTURESATURATIONPLOTDATAMODEL_H
#define CAPTURESATURATIONPLOTDATAMODEL_H


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

class CaptureSaturationPlotDataModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int total READ total NOTIFY totalChanged)
    Q_PROPERTY(int last READ last NOTIFY lastChanged)

    Q_PROPERTY(QwtQuick2PlotCurve* satmaxCurve READ satmaxCurve WRITE setSatmaxCurve NOTIFY satmaxCurveChanged)
    Q_PROPERTY(QwtQuick2PlotCurve* notInPlayOrRecordSatmaxCurve READ notInPlayOrRecordSatmaxCurve WRITE setNotInPlayOrRecordSatmaxCurve NOTIFY notInPlayOrRecordSatmaxCurveChanged)

    Q_PROPERTY(QwtQuick2PlotCurve* satavgCurve READ satavgCurve WRITE setSatavgCurve NOTIFY satavgCurveChanged)
    Q_PROPERTY(QwtQuick2PlotCurve* notInPlayOrRecordSatavgCurve READ notInPlayOrRecordSatavgCurve WRITE setNotInPlayOrRecordSatavgCurve NOTIFY notInPlayOrRecordSatavgCurveChanged)
public:
    explicit CaptureSaturationPlotDataModel(QObject *parent = nullptr);
    ~CaptureSaturationPlotDataModel();

    int total() const;
    int last() const;

    struct GraphStats {
        int frameNumber;
        float satavgValue;
        float satmaxValue;
        bool playing;
    };

    QwtQuick2PlotCurve *satmaxCurve() const;
    void setSatmaxCurve(QwtQuick2PlotCurve *newCurve);

    QwtQuick2PlotCurve *notInPlayOrRecordSatmaxCurve() const;
    void setNotInPlayOrRecordSatmaxCurve(QwtQuick2PlotCurve *newNotInPlayOrRecordCurve);

    QwtQuick2PlotCurve *satavgCurve() const;
    void setSatavgCurve(QwtQuick2PlotCurve *newCurve);

    QwtQuick2PlotCurve *notInPlayOrRecordSatavgCurve() const;
    void setNotInPlayOrRecordSatavgCurve(QwtQuick2PlotCurve *newNotInPlayOrRecordCurve);

public Q_SLOTS:
    void update();
    void reset();

    void append(int frameNumber, float satavg, float satmax, bool playing);

Q_SIGNALS:
    void totalChanged();
    void lastChanged();

    void updated();
    void clearModel();

    void satmaxCurveChanged();
    void notInPlayOrRecordSatmaxCurveChanged();

    void satavgCurveChanged();
    void notInPlayOrRecordSatavgCurveChanged();
private:
    QList<std::tuple<int, GraphStats>> m_values;
    int m_total { 0 };
    int m_last { 0 };

    QwtQuick2PlotCurve *m_satmaxCurve = nullptr;
    QwtQuick2PlotCurve *m_notInPlayOrRecordSatmaxCurve = nullptr;

    QwtQuick2PlotCurve *m_satavgCurve = nullptr;
    QwtQuick2PlotCurve *m_notInPlayOrRecordSatavgCurve = nullptr;
};
#endif // CAPTURESATURATIONPLOTDATAMODEL_H
