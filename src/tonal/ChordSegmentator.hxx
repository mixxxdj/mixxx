/*
 * Copyright (c) 2001-2006 MUSIC TECHNOLOGY GROUP (MTG)
 *                         UNIVERSITAT POMPEU FABRA
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef ChordSegmentator_hxx
#define ChordSegmentator_hxx

#include <iostream>
#include <fstream>
#include <cmath>
//#include "Array.hxx"
#include "DiscontinuousSegmentation.hxx"
#include "ChordCorrelator.hxx"
//#include "Assert.hxx"

namespace Simac
{

/**
 * ChordSegmentator divides the analysed audio data into segments
 * characterized by different chords. It takes as input the chord
 * correlation for each frame and decides whether this new data
 * signifies a chord change in the music. If so it opens a new
 * segment for the new chord, at the same time closing the 
 * previous segment.
 */
class ChordSegmentator
{
	CLAM::DiscontinuousSegmentation _segmentation;
	std::vector<unsigned> _chordIndexes;

	unsigned _currentSegment;
	bool _segmentOpen;
	unsigned _lastChord;

	unsigned _method;
	
	// Chord similarity method variables
	std::vector< std::vector<double> > _chordSimilarity;
	std::vector<double> _segmentChordCorrelation;
	unsigned _framesInSegment;
public:
	ChordSegmentator()
		: _segmentation(0)
		, _currentSegment(0)
		, _segmentOpen(false)
		, _lastChord(0)
		, _framesInSegment(0)
	{
		method(0);
	};
	~ChordSegmentator() {};

	void doIt(CLAM::TData & currentTime, const std::vector<double> & correlation, const unsigned firstCandidate, const unsigned secondCandidate) 
	{
		_segmentation.maxPosition(currentTime);
		switch(_method)
		{
			case 2:
				doItSimilarity(currentTime, correlation, firstCandidate, secondCandidate); 
				break;
			default:
				doItSimple(currentTime, correlation, firstCandidate, secondCandidate); 
		}
	}

	/** 
	 * Simple chord segmentation method
	 */
	void doItSimple(CLAM::TData & currentTime, const std::vector<double> & correlation, const unsigned firstCandidate, const unsigned secondCandidate) 
	{
		CLAM::TData firstCandidateWeight = correlation[firstCandidate];
		CLAM::TData noCandidateWeight = correlation[0];
		
		unsigned currentChord = firstCandidateWeight*0.6<=noCandidateWeight || noCandidateWeight<0.001 ?
				0 : firstCandidate;
		
		if(_segmentOpen)
		{
			if(!currentChord)
				closeSegment(currentTime);
			if(currentChord != _lastChord)
				closeSegment(currentTime);
		}
		if(!_segmentOpen)
		{	
			if(currentChord)
				openSegment(currentTime, currentChord);
		}
		
		_lastChord = currentChord;
		
		if(_segmentOpen)
			_segmentation.dragOffset(_currentSegment, currentTime);
	}

	/** 
	 * Chord similarity based segmentation method
	 */
	void doItSimilarity(CLAM::TData & currentTime, const std::vector<double> & correlation, const unsigned firstCandidate, const unsigned secondCandidate) 
	{
		CLAM::TData firstCandidateWeight = correlation[firstCandidate];
		CLAM::TData noCandidateWeight = correlation[0];
		
		unsigned currentChord = firstCandidateWeight*0.6<=noCandidateWeight || noCandidateWeight<0.001 ?
				0 : firstCandidate;

		unsigned segmentChord=0;
		
		if(_segmentOpen)
		{ 
			for(unsigned i=0; i<correlation.size(); i++) 
				_segmentChordCorrelation[i] += correlation[i]/correlation[0];
			_framesInSegment++;
			estimateChord(_segmentChordCorrelation, segmentChord);
			_chordIndexes[_currentSegment] = segmentChord;

			double segmentCorrelationDiffNew = (_segmentChordCorrelation[segmentChord] - _segmentChordCorrelation[currentChord]) / _framesInSegment;

			double similarity = _chordSimilarity[currentChord][segmentChord];
			
			double similarityThreshold = 0.67;
			double correlationThreshold = 0.3;
			
			if(!currentChord) 
			{
				closeSegment(currentTime);
				_framesInSegment = 0;
				for(unsigned i=0; i<correlation.size(); i++) 
					_segmentChordCorrelation[i] = 0;
			}

			if (similarity < similarityThreshold)
			{
				if(segmentCorrelationDiffNew > correlationThreshold)
				{
					closeSegment(currentTime);
					_framesInSegment = 0;
					for(unsigned i=0; i<correlation.size(); i++) 
						_segmentChordCorrelation[i] = 0;
				}
			}
			
		}
		if(!_segmentOpen && currentChord)
		{	
			openSegment(currentTime, currentChord);
			for(unsigned i=0; i<correlation.size(); i++) 
				_segmentChordCorrelation[i] = correlation[i]/correlation[0];
			_framesInSegment++;
			segmentChord=currentChord;
		}
		
		if(_segmentOpen)
			_segmentation.dragOffset(_currentSegment, currentTime);
	}

	void openSegment(CLAM::TData & currentTime, unsigned currentChord)
	{
		_chordIndexes.push_back(currentChord);
		_currentSegment = _segmentation.insert(currentTime);
		_segmentOpen = true;
	}
	void closeSegment(CLAM::TData & currentTime)
	{
		_segmentation.dragOffset(_currentSegment, currentTime);
		_segmentOpen = false;
		
		switch(_method)
		{
			case 1:
				changeChordIfSegmentTooSmall(_currentSegment);
				break;
			case 2:
				changeChordIfSegmentTooSmall(_currentSegment);
				break;
		}

		mergeSegmentIfIdenticalChordInPreviousSegment(_currentSegment);
	}	

	void changeChordIfSegmentTooSmall(unsigned & segment)
	{
		double minSegmentLength = 0.5;
		
		std::vector<double> onsets = _segmentation.onsets();
		std::vector<double> offsets = _segmentation.offsets();
		unsigned lastSegment = onsets.size();

		if(offsets[segment]-onsets[segment] < minSegmentLength)
		{
			if(segment<lastSegment)
				if(offsets[segment]==onsets[segment+1])
					_chordIndexes[segment] = _chordIndexes[segment+1];
			if(segment>0)
				if(onsets[segment]==offsets[segment-1])
					_chordIndexes[segment] = _chordIndexes[segment-1];
		}
	}
	void mergeSegmentIfIdenticalChordInPreviousSegment(unsigned & segment)
	{
		CLAM::TData time = _segmentation.offsets()[segment];
		if(segment>0)
		{
			if(_chordIndexes[segment] == _chordIndexes[segment-1]
				&& _segmentation.onsets()[segment] == _segmentation.offsets()[segment-1])
			{
				_segmentation.remove(segment);
				_chordIndexes.erase(_chordIndexes.begin()+segment);
				segment--;
				_segmentation.dragOffset(segment, time);
			}
		}
	}

	void closeLastSegment(CLAM::TData & currentTime )
	{
		_segmentation.maxPosition(currentTime);
		
		if (_lastChord != 0)
		{
			_segmentation.dragOffset(_currentSegment, currentTime);
			_segmentOpen = false;
		}
		
		switch(_method)
		{
			case 1:
				changeChordsForSmallSegments();
				joinSegmentsWithIdenticalChords();
				break;
		}
	}

	void eraseAllSegments()
	{
		while( _segmentation.onsets().size() )
		{
			_segmentation.remove(_segmentation.onsets().size()-1);
			_chordIndexes.pop_back();
		}
		_segmentation.maxPosition(0);
	}

	void estimateChord(const ChordCorrelator::ChordCorrelation & correlation, unsigned & estimatedChord)
	{
		double maxCorrelation = 0;
		double underMaxCorrelation = 0;
		unsigned maxIndex = 0;
		unsigned underMaxIndex = 0;
		for (unsigned i=0; i<correlation.size(); i++)
		{
			if (correlation[i]<underMaxCorrelation) continue;
			if (correlation[i]<maxCorrelation)
			{
				underMaxIndex=i;
				underMaxCorrelation=correlation[i];
				continue;
			}
			underMaxIndex=maxIndex;
			underMaxCorrelation=maxCorrelation;
			maxIndex=i;
			maxCorrelation=correlation[i];
		}
		estimatedChord = maxIndex;
	}
	
	//
	// Post Processing Functions
	//
	
	/**
	 * Finds segments shorter then minSegmentLength and
	 * assigns them the same chord as the chord in either
	 * the previous or the next segment.
	 */
	void changeChordsForSmallSegments()
	{
		for(unsigned segment=0; segment<_segmentation.onsets().size(); segment++)
			changeChordIfSegmentTooSmall(segment);
	}
	void joinSegmentsWithIdenticalChords()
	{
		for(unsigned segment=1; segment<_segmentation.onsets().size(); segment++)
			mergeSegmentIfIdenticalChordInPreviousSegment(segment);
	}

	const CLAM::DiscontinuousSegmentation & segmentation() const { return _segmentation; };
	const std::vector<unsigned> & chordIndexes() const { return _chordIndexes; };
	void method(unsigned method) 
	{ 
		_method=method; 
		if(method != 0 && method != 1 && method != 2)
			_method = 0;

		switch(_method)
		{
			case 2:
				ChordCorrelator chordCorrelator;
				_chordSimilarity = chordCorrelator.chordPatternsSimilarity();
				for(unsigned i=0; i<101; ++i)
					_segmentChordCorrelation.push_back(0);
				break;
		}
	}
};
}
#endif//ChordSegmentator
