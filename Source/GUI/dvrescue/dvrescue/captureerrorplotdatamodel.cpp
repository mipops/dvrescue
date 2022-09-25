#include "captureerrorplotdatamodel.h"
#include "qwtquick2plot.h"
#include "qwt_plot.h"
#include "plotdata.h"

CaptureErrorPlotDataModel::CaptureErrorPlotDataModel(QObject *parent) : QObject(parent)
{

}

CaptureErrorPlotDataModel::~CaptureErrorPlotDataModel()
{

}


int CaptureErrorPlotDataModel::total() const
{
    return m_total;
}

int CaptureErrorPlotDataModel::totalWithErrors() const
{
    return m_totalWithErrors;
}

int CaptureErrorPlotDataModel::last() const
{
    return m_last;
}

int CaptureErrorPlotDataModel::lastWithErrors() const
{
    return m_lastWithErrors;
}

void CaptureErrorPlotDataModel::update()
{
    if(m_values.size() == 0)
        return;

    m_evenCurve->plot()->plot()->setUpdatesEnabled(false);

    auto evenCurveData = static_cast<CircularPlotData*>(m_evenCurve->curve()->data());
    auto oddCurveData = static_cast<CircularPlotData*>(m_oddCurve->curve()->data());

    int maxFrame = 0;
    int minFrame = 0;
    float absMaxValue = 0;
    auto valuesCount = m_values.count();
    auto valuesCountWithError = 0;

    for(auto i = 0; i < m_values.count(); ++i) {
        auto& valueTuple = m_values.at(i);

        auto frameNumber = std::get<0>(valueTuple);
        maxFrame = qMax(maxFrame, frameNumber);

        auto value = std::get<1>(valueTuple);
        absMaxValue = std::max(std::abs(value.evenValue), std::abs(value.oddValue));

        evenCurveData->append(QPointF(frameNumber, value.evenValue));
        oddCurveData->append(QPointF(frameNumber, value.oddValue));

        if(!qFuzzyCompare(value.evenValue, 0.0f) || !qFuzzyCompare(value.oddValue, 0.0f)) {
            ++valuesCountWithError;
        }
    }

    minFrame = maxFrame - evenCurveData->maxSize();
    minFrame = qMax(0, minFrame);

    m_evenCurve->plot()->setXBottomAxisRange(QVector2D(minFrame, maxFrame));

    if(absMaxValue > 750 && m_evenCurve->plot()->yLeftAxisRange().y() < 900) {
        m_evenCurve->plot()->setYLeftAxisRange(QVector2D(-900, 900));
    }

    if(valuesCount != 0) {
        m_total += valuesCount;
        Q_EMIT totalChanged();
    }

    if(valuesCountWithError != 0) {
        m_totalWithErrors += valuesCountWithError;
        Q_EMIT totalWithErrorsChanged();
    }

    auto last = static_cast<int> (evenCurveData->size());
    auto lastWithErrors = 0;

    for(auto i = 0; i < evenCurveData->size(); ++i) {
        bool hasError = !qFuzzyCompare(evenCurveData->sample(i).y(), 0.0);
        if(!hasError)
            hasError = !qFuzzyCompare(oddCurveData->sample(i).y(), 0.0);

        if(hasError)
            ++lastWithErrors;
    }

    if(last != m_last) {
        m_last = last;
        Q_EMIT lastChanged();
    }

    if(lastWithErrors != m_lastWithErrors) {
        m_lastWithErrors = lastWithErrors;
        Q_EMIT lastWithErrorsChanged();
    }

    m_values.clear();

    m_evenCurve->plot()->plot()->setUpdatesEnabled(true);
    m_evenCurve->plot()->replotAndUpdate();

    Q_EMIT updated();
}

void CaptureErrorPlotDataModel::reset()
{
    static_cast<CircularPlotData*>(m_evenCurve->curve()->data())->clear();
    static_cast<CircularPlotData*>(m_oddCurve->curve()->data())->clear();

    m_values.clear();

    if(m_total != 0) {
        m_total = 0;
        Q_EMIT totalChanged();
    }

    if(m_totalWithErrors != 0) {
        m_totalWithErrors = 0;
        Q_EMIT totalWithErrorsChanged();
    }

    if(m_last != 0) {
        m_last = 0;
        Q_EMIT lastChanged();
    }

    if(m_lastWithErrors != 0) {
        m_lastWithErrors = 0;
        Q_EMIT lastWithErrorsChanged();
    }

    m_evenCurve->plot()->replotAndUpdate();
    Q_EMIT clearModel();
}

void CaptureErrorPlotDataModel::append(int frameNumber, float even, float odd)
{
    qDebug() << "CaptureErrorPlotDataModel::append: " << frameNumber << even << odd;

    even = std::isnan(even) ? 0 : even;
    odd = std::isnan(odd) ? 0 : odd;

    GraphStats value = {
        int(frameNumber),
        even,
        -odd,
    };

    m_values.push_back(std::make_tuple(frameNumber, value));
}

QwtQuick2PlotCurve *CaptureErrorPlotDataModel::evenCurve() const
{
    return m_evenCurve;
}

void CaptureErrorPlotDataModel::setEvenCurve(QwtQuick2PlotCurve *newEvenCurve)
{
    if (m_evenCurve == newEvenCurve)
        return;
    m_evenCurve = newEvenCurve;
    m_evenCurve->curve()->setData(new CircularPlotData());
    Q_EMIT evenCurveChanged();
}

QwtQuick2PlotCurve *CaptureErrorPlotDataModel::oddCurve() const
{
    return m_oddCurve;
}

void CaptureErrorPlotDataModel::setOddCurve(QwtQuick2PlotCurve *newOddCurve)
{
    if (m_oddCurve == newOddCurve)
        return;
    m_oddCurve = newOddCurve;
    m_oddCurve->curve()->setData(new CircularPlotData());
    Q_EMIT oddCurveChanged();
}
