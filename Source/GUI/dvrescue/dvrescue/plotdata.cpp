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

CircularPlotData::CircularPlotData()
{

}

void CircularPlotData::append(QPointF &&point)
{
    if(_container.size() < _maxSize) {
        _container.append(point);
    } else {
        _container[_offset] = point;
        _offset = (_offset + 1) % _maxSize;
    }
}

void CircularPlotData::append(const QPointF &point)
{
    if(_container.size() < _maxSize) {
        _container.append(point);
    } else {
        _container[_offset] = point;
        _offset = (_offset + 1) % _maxSize;
    }
}

void CircularPlotData::clear()
{
    _offset = 0;
    _container.clear();
}

int CircularPlotData::maxSize() const
{
    return _maxSize;
}

size_t CircularPlotData::size() const
{
    return static_cast<size_t>(_container.size());
}

QPointF CircularPlotData::sample(size_t i) const
{
    if(i < 0)
        return QPointF(i, 0);

    return _container.at(static_cast<int>((i + _offset) % _maxSize));
}

QRectF CircularPlotData::boundingRect() const
{
    return qwtBoundingRect(*this);
}
