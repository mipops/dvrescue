#include "plotdata.h"

SequentialPlotData::SequentialPlotData()
{
}

void SequentialPlotData::append(QPointF &&point)
{
    _container.append(point);
}

void SequentialPlotData::append(const QPointF &point)
{
    _container.append(point);
}

void SequentialPlotData::clear()
{
    _container.clear();
}

size_t SequentialPlotData::size() const
{
    return static_cast<size_t>(_container.size());
}

QPointF SequentialPlotData::sample(size_t i) const
{
    return _container.at(static_cast<int>(i));
}

QRectF SequentialPlotData::boundingRect() const
{
    return qwtBoundingRect(*this);
}
