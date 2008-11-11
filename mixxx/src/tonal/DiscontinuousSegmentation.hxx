#ifndef DiscontinuousSegmentation_hxx
#define DiscontinuousSegmentation_hxx

#include "Segmentation.hxx"

namespace CLAM
{
	class DiscontinuousSegmentation : public Segmentation
	{
	public:
		class InsertedOutOfBounds : public std::exception
		{
			public:
			const char * what() const throw () { return "Segmentation point inserted out of limits";}
		};
		class OffsetMissing : public std::exception
		{
			public:
			const char * what() const throw () { return "Odd number of segmentation points, every segment beggining must be followed by its ending";}
		};
		class MissplacedOnset : public std::exception
		{
			std::string _message;
			public:
			MissplacedOnset(unsigned missplacedOnset,
					double previousOffsetPosition,
					double intendedOnsetPosition)
			{
				std::ostringstream os;
				os << "Segment " << missplacedOnset
					<< " starts at " << intendedOnsetPosition
					<< " overlapping previous segment which ends at " << previousOffsetPosition;
				_message = os.str();
			}
			virtual ~MissplacedOnset() throw () {}
			const char * what() const throw () { return _message.c_str(); }
		};
		class MissplacedOffset : public std::exception
		{
			std::string _message;
			public:
			MissplacedOffset(unsigned missplacedOffset,
					double onsetPosition,
					double offsetPosition)
			{
				std::ostringstream os;
				os << "Segment " << missplacedOffset
					<< " starts at " << onsetPosition
					<< " but ends before that, at " << offsetPosition;
				_message = os.str();
			}
			virtual ~MissplacedOffset() throw () {}
			const char * what() const throw () { return _message.c_str(); }
		};
		typedef std::vector<double> TimePositions;
	public:
		DiscontinuousSegmentation(double maxPosition=0)
			: Segmentation(maxPosition)
		{
		}
		/**
		 * It will create a discontinuous segmentation where the onsets and offsets
		 * are specified on the ordered list of bounds [begin,end)
		 * @pre The onsets is a sorted container of bounds in between 0 and maxPosition
		 */
		
		DiscontinuousSegmentation(double maxPosition, const TData * begin, const TData * end)
			: Segmentation(maxPosition)
		{
			takeArray(begin, end);
		}
		/**
		 * take data from an array.
		 */
		void takeArray(const TData * begin, const TData * end)
		{
			double previousOffset=0.0;
			unsigned i=0;
			for (const TData* it=begin; it!=end; i++)
			{
				double onset = *it++;
				std::cout << onset << " " << std::flush;
				if (onset<previousOffset) throw MissplacedOnset(i,previousOffset,onset);
				if (it==end) throw OffsetMissing();
				double offset = *it++;
				std::cout << offset << " " << std::flush;
				if (offset<onset) throw MissplacedOffset(i, onset, offset);
				if (offset>_maxPosition) throw InsertedOutOfBounds();  //_maxPosition 
				_onsets.push_back(onset);
				_offsets.push_back(offset);
				_labels.push_back(""); // TODO: a constructor with not empty labels
				_selection.push_back(false);
				previousOffset=offset;
			}
		}
		/**
		 * Inserts a new border at timePosition.
		 */
		unsigned insert(double timePosition)
		{
			if (timePosition<0.0) throw InsertedOutOfBounds();
			if (timePosition>_maxPosition) throw InsertedOutOfBounds();
			TimePositions::iterator nextOffset = 
				std::lower_bound(_offsets.begin(), _offsets.end(), timePosition);
			if (nextOffset == _offsets.end()) // Beyond any existing segment
			{
				_onsets.push_back(timePosition);
				_offsets.push_back(_maxPosition);
				_labels.push_back("");
				_selection.push_back(false);
				return _onsets.size()-1;
			}
			// 'nextOffsetPosition' must be computed before the insertion to not invalidate iterators.
			unsigned nextOffsetPosition = nextOffset - _offsets.begin();
			if (_onsets[nextOffsetPosition]<=timePosition) // Just in the middle of a segment
			{
				_offsets.insert(nextOffset, timePosition);
				_onsets.insert(_onsets.begin()+nextOffsetPosition+1, timePosition);
				_labels.insert(_labels.begin()+nextOffsetPosition+1, "");
				_selection.insert(_selection.begin()+nextOffsetPosition+1, false);
				if (nextOffsetPosition<_current) _current++;
				return nextOffsetPosition+1;
			}
			else // In a gap before a segment
			{
				_offsets.insert(nextOffset, _onsets[nextOffsetPosition]);
				_onsets.insert(_onsets.begin()+nextOffsetPosition, timePosition);
				_labels.insert(_labels.begin()+nextOffsetPosition, "");
				_selection.insert(_selection.begin()+nextOffsetPosition, false);
				if (_current>=nextOffsetPosition) _current++;
				return nextOffsetPosition;
			}
			
		}
		/**
		 * Inserts a new border at timePosition and adds a label to the new segment.
		 */
		unsigned insert(double timePosition, std::string label)
		{
			unsigned segment = insert(timePosition);
			setLabel(segment, label);
			return segment;
		}
		/**
		 * Removes the specified segment.
		 * The previous segment is expanded to cover the region.
		 * When removing the first segment, the next segment is the one expanded to start at 0.
		 * When just a single element, no efect at all.
		 */
		void remove(unsigned segment)
		{
			_offsets.erase(_offsets.begin()+segment);
			_onsets.erase(_onsets.begin()+segment);
			_labels.erase(_labels.begin()+segment);
			_selection.erase(_selection.begin()+segment);
			if (_current!=0 && segment<=_current) _current--;
		}
		/**
		 * Returns the index of the segment whose offset is nearest 
		 * to the given time position, and within the tolerance.
		 * If no end of segment within the tolerance range an invalid
		 * segment is returned (nSegments)
		 */
		unsigned pickOffset(double timePosition, double tolerance) const
		{
			return pickPosition(_offsets, timePosition, tolerance);
		}
		/**
		 * Returns the index of the segment whose onset is nearest
		 * to the given time position, and within the tolerance.
		 * If no end of segment within the tolerance range an invalid
		 * segment is returned (nSegments)
		 */
		unsigned pickOnset(double timePosition, double tolerance) const
		{
			return pickPosition(_onsets, timePosition, tolerance);
		}
		/**
		 * Returns the index of the segment which body is on timePosition.
		 */
		unsigned pickSegmentBody(double timePosition) const
		{
			if (timePosition<0) return _offsets.size();
			TimePositions::const_iterator lowerBound =
				std::lower_bound(_offsets.begin(), _offsets.end(), timePosition);
			unsigned index = lowerBound-_offsets.begin();
			if (index==_offsets.size()) return index;
			if (_onsets[index]>timePosition) return _offsets.size();
			return index;
		}
		/**
		 * Performs a dragging movement for the Onset of the given
		 * segment in order to move it to the newTimePosition.
		 * Constraints for the segmentation mode are applied.
		 */
		void dragOnset(unsigned segment, double newTimePosition)
		{
			// The onset is attached to the previous offset
			if (segment>=_onsets.size()) return; // Invalid segment

			// Limit to the left to the previous onset or 0
			double leftBound = segment ? _offsets[segment-1] : 0;
			if (newTimePosition<leftBound)
				newTimePosition=leftBound;
			// Limit to the right to the own offset
			double rigthBound = _offsets[segment];
			if (newTimePosition>rigthBound)
				newTimePosition=rigthBound;

			// The offset and the next onset change together
			_onsets[segment]=newTimePosition;
		}
		/**
		 * Performs a dragging movement for the Offset of the given
		 * segment in order to move it to the newTimePosition.
		 * Constraints for the segmentation mode are applied.
		 */
		void dragOffset(unsigned segment, double newTimePosition)
		{
			if (segment>=_offsets.size()) return; // Invalid segment

			// Limit to the right to the next offset or max
			double rigthBound = segment+1==_offsets.size()? _maxPosition : _onsets[segment+1];
			if (newTimePosition>rigthBound)
				newTimePosition=rigthBound;
			// Limit to the left to the own onset
			double leftBound = _onsets[segment];
			if (newTimePosition<leftBound)
				newTimePosition=leftBound;

			// The offset and the next onset change together
			_offsets[segment]=newTimePosition;
		}
		/**
		* Fills a DataArray with the segmentation markers
		*/
		/*void fillArray(DataArray& segmentation) const
		{
			unsigned nSegments = _onsets.size();
			segmentation.Resize(nSegments*2);
			segmentation.SetSize(nSegments*2);
			for (unsigned i=0; i<nSegments; i++)
				{
					segmentation[i*2] = _onsets[i];
					segmentation[i*2+1] = _offsets[i];
				}
		}*/

		const char * GetClassName() const { return "DiscontinuousSegmentation"; }


	private:
		/**
		 * Returns the index of the time position which is nearest
		 * to the given time position and within the tolerance.
		 * If no end of segment within the tolerance range an invalid
		 * index is returned (nPositions)
		 * @pre positions is a sorted array
		 */
		unsigned pickPosition(const TimePositions & positions, double timePosition, double tolerance) const
		{
			TimePositions::const_iterator lowerBound = 
				std::lower_bound(positions.begin(), positions.end(), timePosition-tolerance);
			TimePositions::const_iterator upperBound = 
				std::upper_bound(lowerBound, positions.end(), timePosition+tolerance);

			if (lowerBound==upperBound) return positions.size(); // None found
	
			// Pick the closest in range
			unsigned lowerSegment = lowerBound - positions.begin();
			unsigned upperSegment = upperBound - positions.begin();
			double lastDifference = std::fabs(timePosition-positions[lowerSegment]);
			for (unsigned i=lowerSegment; i<upperSegment; i++)
			{
				double newDifference = std::fabs(timePosition-positions[i]);
				if (newDifference>lastDifference) break;
				lastDifference = newDifference;
				lowerSegment = i;
			}
			return lowerSegment;
		}
	};

}



#endif//DiscontinuousSegmentation_hxx

