#include "plotdata.h"

PlotData::PlotData(QVector<QPointF> *container) :
    _container(container)
{
}

QVector<QPointF> *PlotData::data()
{
    return _container;
}

size_t PlotData::size() const
{
    return static_cast<size_t>(_container->size());
}

QPointF PlotData::sample(size_t i) const
{
    return _container->at(static_cast<int>(i));
}

QRectF PlotData::boundingRect() const
{
    return qwtBoundingRect(*this);
}
