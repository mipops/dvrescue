#ifndef PLOTDATA_H
#define PLOTDATA_H

#include <QObject>
#include <QPointF>
#include <QVector>

#include <qwt_series_data.h>


class SequentialPlotData : public QwtSeriesData<QPointF>
{
public:
    SequentialPlotData();

    void append(QPointF&& point);
    void append(const QPointF& point);
    void clear();
private:
      QVector<QPointF> _container;

      // QwtSeriesData interface
public:
      size_t size() const;
      QPointF sample(size_t i) const;
      QRectF boundingRect() const;
};

class CircularPlotData : public QwtSeriesData<QPointF>
{
public:
    CircularPlotData();

    void append(QPointF&& point);
    void append(const QPointF& point);
    void clear();

    int maxSize() const;
private:
      QVector<QPointF> _container;
      int _offset { 0 };
      int _maxSize { 9000 };

      // QwtSeriesData interface
public:
      size_t size() const;
      QPointF sample(size_t i) const;
      QRectF boundingRect() const;
};







#endif // PLOTDATA_H
