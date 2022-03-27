#include "csvparser.h"
#include <QDebug>

CsvParser::CsvParser(QObject *parent)
    : QObject{parent}
{

}

const QChar newline(QChar('\n'));
const QChar cr('\r');
#ifdef __linux__
    const QString eol = newline;
#elif _WIN32
    const QString eol = QString(cr) + QString(newline);
#else
    const QString eol = newline;
#endif

void CsvParser::write(const QByteArray &data)
{
    write(QString::fromUtf8(data));
}

void CsvParser::write(const QString &data)
{
    auto indexOfNewLine = data.indexOf(eol);
    if(indexOfNewLine != -1) {
        auto splitted = data.split(eol);
        for(auto i = 0; i < splitted.length(); ++i) {
            m_buffer.append(splitted[i]);
            if(!m_buffer.isEmpty() && (i != (splitted.length() - 1) || data.endsWith('\r')))
            {
                onNewEntry(m_buffer);
                m_buffer.clear();
            }
        }
    } else {
        // incompleted single-entry
        m_buffer.append(data);
    }
}

void CsvParser::onNewEntry(const QString &entry)
{
    assert(entry.indexOf('\r') == -1);

    auto splitted = entry.split(',');
    if(m_columns.empty()) {
        if(entry.startsWith("FramePos"))
            setColumns(splitted);
    } else {
        bool ok = false;
        splitted[0].toInt(&ok, 10);
        if(!ok)
            return;

        auto now = QTime::currentTime();
        if(m_lastUpdateEmitted.msecsTo(now) > 25) {
            m_lastUpdateEmitted = now;
            Q_EMIT entriesReceived(splitted);
        }
    }
}

const QStringList &CsvParser::columns() const
{
    return m_columns;
}

void CsvParser::setColumns(const QStringList &newColumns)
{
    if (m_columns == newColumns)
        return;
    m_columns = newColumns;
    Q_EMIT columnsChanged(m_columns);
}
