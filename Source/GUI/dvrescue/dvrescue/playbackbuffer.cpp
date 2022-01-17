#include "playbackbuffer.h"

void BufferSequential::write(const QByteArray &data) {
    Buffer::write(data);
    Q_EMIT readyRead();
}
