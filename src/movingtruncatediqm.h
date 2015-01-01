#ifndef MOVINGTRUNCATEDIQM_H
#define MOVINGTRUNCATEDIQM_H

#include <QLinkedList>
#include <QQueue>

//! Truncated Interquartile mean

//! TruncatedIQM keeps an ordered list with the last n (_capacity_)
//! input doubles and calculates the mean discarding the lowest 25%
//! and the highest 25% values in order to reduce sensitivity to outliers.
//!
//! http://en.wikipedia.org/wiki/Interquartile_mean
class MovingTruncatedIQM {
  public:
    //! Constructs an empty MovingTruncatedIQM.
    MovingTruncatedIQM(const unsigned int listLength);
    virtual ~MovingTruncatedIQM();
    
    //! Inserts value to the list and returns the new truncated mean.
    double insert(double value);
    //! Empty the list.
    void clear();
    //! Returns the current truncated mean. Input list must not be empty.
    double mean();
    //! Returns how many values have been input.
    int size() const;
    //! Returns the maximum size of the input list.
    int listMaxSize() const;

  private:
    double m_dMean;
    int m_iListMaxSize;
    //! The list keeps input doubles ordered by value.
    QLinkedList<double> m_List;
    //! The queue keeps pointers to doubles in the list ordered
    //! by the order they were received.
    QQueue<QLinkedList<double>::iterator> m_Queue;

    //! sum() checks this to know if it has to recalculate the mean.
    bool m_bChanged;
};

#endif /* MovingTruncatedIQM_H */
