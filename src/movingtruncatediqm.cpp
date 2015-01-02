#include "movingtruncatediqm.h"

MovingTruncatedIQM::MovingTruncatedIQM(const unsigned int listMaxSize)
    : m_iListMaxSize(listMaxSize),
      m_bChanged(true) {
}

MovingTruncatedIQM::~MovingTruncatedIQM() {};

double MovingTruncatedIQM::insert(double value) {
    m_bChanged = true;

    // Insert new value
    if (m_List.isEmpty()) {
        m_List.prepend(value);
        m_Queue.enqueue(m_List.begin());
    } else if (value < m_List.first()) {
        m_List.prepend(value);
        m_Queue.enqueue(m_List.begin());
    } else if (value >= m_List.last()) {
        m_List.append(value);
        m_Queue.enqueue(--m_List.end());
    } else {
        QLinkedList<double>::iterator i = m_List.begin()++;
        while (value >= *i) ++i;
        m_Queue.enqueue(m_List.insert(i, value));
        // (If value already exists in the list, the new instance
        // is appended next to the old ones: 2·-> 1 2 3 = 1 2 2· 3)
    }
    
    // If list is already full, first delete the oldest value:
    if (m_List.size() == m_iListMaxSize+1) {
        m_List.erase(m_Queue.dequeue());
    }
    return mean();
}

void MovingTruncatedIQM::clear() {
    m_bChanged = true;
    m_Queue.clear();
    m_List.clear();
}

double MovingTruncatedIQM::mean() {
    if (m_bChanged) {
        m_bChanged = false;
        if (m_List.size() <=4) {
            double d_sum = 0;
            foreach (double d, m_List) {
                d_sum += d;
            }
            m_dMean = d_sum/m_List.size();
        } else if (m_List.size()%4 == 0) {
            int quartileSize = m_List.size()/4;
            QLinkedList<double>::iterator i = m_List.begin();
            i=i+quartileSize;
            double d_sum = 0;
            for (int k=0; k < 2*quartileSize; ++k) {
                d_sum += *(i++);
            }
            m_dMean = d_sum / (2*quartileSize);
        } else {
            // http://en.wikipedia.org/wiki/Interquartile_mean#Dataset_not_divisible_by_four
            // TODO(Ferran Pujol): Make this more clear.
            double quartileSize = m_List.size()/4.0;
            double interQuartileRange = 2*quartileSize;
            int nFullValues = m_List.size()-2*(int)quartileSize - 2;
            double quartileWeight = (interQuartileRange-nFullValues)/2;
            
            QLinkedList<double>::iterator i = m_List.begin();
            i=i+((int)quartileSize);
            double d_sum = (*i)*quartileWeight;
            ++i;
            for (int k=0; k < nFullValues; ++k) {
                d_sum += *(i++);
            }
            d_sum += (*i)*quartileWeight;
            m_dMean = d_sum / interQuartileRange;
        }
    }
    return m_dMean;
}

int MovingTruncatedIQM::size() const {
    return m_List.size();
}

int MovingTruncatedIQM::listMaxSize() const {
    return m_iListMaxSize;
}
