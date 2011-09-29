#ifndef FIFO_H
#define FIFO_H

#include "util/pa_ringbuffer.h"
#include "util.h"

template <class DataType>
class FIFO {
  public:
    explicit FIFO(int size) {
        m_data = new DataType[size];
        Q_ASSERT(PaUtil_InitializeRingBuffer(
            &m_ringBuffer, sizeof(DataType), size, m_data) == 0);
    }
    virtual ~FIFO() {
        delete [] m_data;
    }
    int readAvailable() const {
        return PaUtil_GetRingBufferReadAvailable(&m_ringBuffer);
    }
    int writeAvailable() const {
        return PaUtil_GetRingBufferWriteAvailable(&m_ringBuffer);
    }
    int read(DataType* pData, int count) {
        return PaUtil_ReadRingBuffer(&m_ringBuffer, pData, count);
    }
    int write(const DataType* pData, int count) {
        return PaUtil_WriteRingBuffer(&m_ringBuffer, pData, count);
    }
    void writeBlocking(const DataType* pData, int count) {
        int written = 0;
        while (written != count) {
            int i = write(pData, count);
            pData += i;
            written += i;
        }
    }
  private:
    DataType* m_data;
    PaUtilRingBuffer m_ringBuffer;
    DISALLOW_COPY_AND_ASSIGN(FIFO<DataType>);
};

#endif /* FIFO_H */
