#include "capturesaturationplotdatamodel.h"
#include "qwtquick2plot.h"
#include "qwt_plot.h"
#include "plotdata.h"

CaptureSaturationPlotDataModel::CaptureSaturationPlotDataModel(QObject *parent) : QObject(parent)
{

}

CaptureSaturationPlotDataModel::~CaptureSaturationPlotDataModel()
{

}


int CaptureSaturationPlotDataModel::total() const
{
    return m_total;
}

int CaptureSaturationPlotDataModel::last() const
{
    return m_last;
}

void CaptureSaturationPlotDataModel::update()
{
    if(m_values.size() == 0)
        return;

    m_satmaxCurve->plot()->plot()->setUpdatesEnabled(false);

    auto satavgCurveData = static_cast<CircularPlotData*>(m_satavgCurve->curve()->data());
    auto notInPlayOrRecordSatavgCurveData = static_cast<CircularPlotData*>(m_notInPlayOrRecordSatavgCurve->curve()->data());

    auto satmaxCurveData = static_cast<CircularPlotData*>(m_satmaxCurve->curve()->data());
    auto notInPlayOrRecordSatmaxCurveData = static_cast<CircularPlotData*>(m_notInPlayOrRecordSatmaxCurve->curve()->data());

    int maxFrame = 0;
    int minFrame = 0;
    auto valuesCount = m_values.count();
    auto valuesCountWithError = 0;

    for(auto i = 0; i < m_values.count(); ++i) {
        auto& valueTuple = m_values.at(i);

        auto frameNumber = std::get<0>(valueTuple);
        maxFrame = qMax(maxFrame, frameNumber);
        auto value = std::get<1>(valueTuple);

        auto playing = value.playing;
        if(playing) {
            satavgCurveData->append(QPointF(frameNumber, value.satavgValue));
            // notInPlayOrRecordSatavgCurveData->append(QPointF(frameNumber, 0));

            satmaxCurveData->append(QPointF(frameNumber, value.satmaxValue));
            // notInPlayOrRecordSatmaxCurveData->append(QPointF(frameNumber, 0));
        } else {
            satavgCurveData->append(QPointF(frameNumber, value.satavgValue));
            // notInPlayOrRecordSatavgCurveData->append(QPointF(frameNumber, 900));

            satmaxCurveData->append(QPointF(frameNumber, value.satmaxValue));
            // notInPlayOrRecordSatmaxCurveData->append(QPointF(frameNumber, 900));
        }
    }

    minFrame = maxFrame - satmaxCurveData->maxSize();
    minFrame = qMax(0, minFrame);

    m_satmaxCurve->plot()->setXBottomAxisRange(QVector2D(minFrame, maxFrame));

    if(valuesCount != 0) {
        m_total += valuesCount;
        Q_EMIT totalChanged();
    }

    auto last = static_cast<int> (satmaxCurveData->size());

    if(last != m_last) {
        m_last = last;
        Q_EMIT lastChanged();
    }

    m_values.clear();

    m_satmaxCurve->plot()->plot()->setUpdatesEnabled(true);
    m_satmaxCurve->plot()->replotAndUpdate();

    Q_EMIT updated();
}

void CaptureSaturationPlotDataModel::reset()
{
    static_cast<CircularPlotData*>(m_satavgCurve->curve()->data())->clear();
    static_cast<CircularPlotData*>(m_notInPlayOrRecordSatavgCurve->curve()->data())->clear();

    static_cast<CircularPlotData*>(m_satmaxCurve->curve()->data())->clear();
    static_cast<CircularPlotData*>(m_notInPlayOrRecordSatmaxCurve->curve()->data())->clear();

    m_values.clear();

    if(m_total != 0) {
        m_total = 0;
        Q_EMIT totalChanged();
    }

    if(m_last != 0) {
        m_last = 0;
        Q_EMIT lastChanged();
    }

    m_satmaxCurve->plot()->replotAndUpdate();
    Q_EMIT clearModel();
}

void CaptureSaturationPlotDataModel::append(int frameNumber, float satavg, float satmax, bool playing)
{
    // qDebug() << "CaptureSaturationPlotDataModel::append: " << frameNumber << even << odd << playing;

    satavg = std::isnan(satavg) ? 0 : satavg;
    satmax = std::isnan(satmax) ? 0 : satmax;

    GraphStats value = {
        int(frameNumber),
        satavg,
        satmax,
        playing
    };

    m_values.push_back(std::make_tuple(frameNumber, value));
}

QwtQuick2PlotCurve *CaptureSaturationPlotDataModel::satmaxCurve() const
{
    return m_satmaxCurve;
}

void CaptureSaturationPlotDataModel::setSatmaxCurve(QwtQuick2PlotCurve *newCurve)
{
    if (m_satmaxCurve == newCurve)
        return;
    m_satmaxCurve = newCurve;
    m_satmaxCurve->curve()->setData(new CircularPlotData());
    Q_EMIT satmaxCurveChanged();
}

QwtQuick2PlotCurve *CaptureSaturationPlotDataModel::notInPlayOrRecordSatmaxCurve() const
{
    return m_notInPlayOrRecordSatmaxCurve;
}

void CaptureSaturationPlotDataModel::setNotInPlayOrRecordSatmaxCurve(QwtQuick2PlotCurve *newNotInPlayOrRecordCurve)
{
    if (m_notInPlayOrRecordSatmaxCurve == newNotInPlayOrRecordCurve)
        return;
    m_notInPlayOrRecordSatmaxCurve = newNotInPlayOrRecordCurve;
    m_notInPlayOrRecordSatmaxCurve->curve()->setData(new CircularPlotData());
    Q_EMIT notInPlayOrRecordSatmaxCurveChanged();
}

QwtQuick2PlotCurve *CaptureSaturationPlotDataModel::satavgCurve() const
{
    return m_satavgCurve;
}

void CaptureSaturationPlotDataModel::setSatavgCurve(QwtQuick2PlotCurve *newCurve)
{
    if (m_satavgCurve == newCurve)
        return;
    m_satavgCurve = newCurve;
    m_satavgCurve->curve()->setData(new CircularPlotData());
    Q_EMIT satavgCurveChanged();
}

QwtQuick2PlotCurve *CaptureSaturationPlotDataModel::notInPlayOrRecordSatavgCurve() const
{
    return m_notInPlayOrRecordSatavgCurve;
}

void CaptureSaturationPlotDataModel::setNotInPlayOrRecordSatavgCurve(QwtQuick2PlotCurve *newNotInPlayOrRecordCurve)
{
    if (m_notInPlayOrRecordSatavgCurve == newNotInPlayOrRecordCurve)
        return;
    m_notInPlayOrRecordSatavgCurve = newNotInPlayOrRecordCurve;
    m_notInPlayOrRecordSatavgCurve->curve()->setData(new CircularPlotData());
    Q_EMIT notInPlayOrRecordSatavgCurveChanged();
}
