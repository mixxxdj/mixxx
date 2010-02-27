///////////////////////////////////////////////////////////////////////////////
//! \file HSS1394Types.h
//!
//! Common non-standard types used in HSS1394 interface specification.
//! 
//! Created by Don Goodeve (don@bearanascence.com), 28Apr2008, 10:20PST
//! this file Copyright (C) 2009, Stanton Group.
//! $Id: HSS1394Types.h 1563 2009-04-16 06:48:34Z don $
//-----------------------------------------------------------------------------
//! GNU Lesser Public License:
//! This program is free software: you can redistribute it and/or modify
//! it under the terms of the GNU Lesser General Public License as published 
//! by the Free Software Foundation, either version 3 of the License, or
//! (at your option) any later version.
//!
//! This program is distributed in the hope that it will be useful,
//! but WITHOUT ANY WARRANTY; without even the implied warranty of
//! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//! GNU Lesser General Public License for more details.
//!
//! You should have received a copy of the GNU Lesser General Public License
//! along with this program.  If not, see <http://www.gnu.org/licenses/>.
///////////////////////////////////////////////////////////////////////////////

#ifndef _HSS1394Types_
#define _HSS1394Types_

//*** Defines
//*****************************************************************************
#ifndef NULL
	#define NULL	((void*)0x0)
#endif

// Some useful constants
//! Conventional 'K' definition
#define HSS1394_k	(1024)
//! Conventional 'M' definition
#define HSS1394_M	(HSS1394_k*HSS1394_k)


//*** Types
//*****************************************************************************
namespace hss1394 {
	// THSS1394Tags
	//! Message tags used in HSS1394 protocol.
	//-----------------------------------------------------------------------------
	typedef enum {
		// Must match with defines in NodeHSS1394.cpp on embedded endpoint.
		kUserData		= 0x00,		//!< Data is 'user' data to be delivered to app
		kDebugData		= 0x01,		//!< Data is for debug channel
		kUserTagBase	= 0x10,		//!< Base of user special tag space
		kUserTagTop		= 0xEF,		//!< Top of user special tag space
		kReset			= 0xF0,		//!< Reset node message
		kChangeAddress	= 0xF1,		//!< Change address message tag
		kPing			= 0xF2,		//!< Ping message tag. Reply is a kPingResponse packet
		kPingResponse	= 0xF3,		//!< Ping response packet tag
		kEchoAsUserData = 0xF4,		//!< Debug tag. Echo data back 'as-if' sent from node
		kUndefined		= 0xFF		//!< Undefined tag
	}THSS1394Tags;


	// Useful bit-counted type definitions
	// Potentially Platform-dependent, however 32-bit assumptions can be made.
	//-----------------------------------------------------------------------------
	#ifdef __WINDOWS__
		typedef unsigned __int64 _u64;
	#else	// We assume Mac
		typedef uint64_t _u64;
	#endif

	// 32-bit machine/compiler assumption
	typedef unsigned int     uint;		//!< Machine native unsigned int
	typedef unsigned char    uint8;		//!< 8-bit unsigned int
	typedef unsigned short   uint16;	//!< 16-bit unsigned int
	typedef unsigned int     uint32;	//!< 32-bit unsigned int
	
	//! 48-bit unsigned integer
	class uint48 {
		public:
			uint32 mu32Low;		//!< LS32 of value
			uint16 mu16High;	//!< MS16 of value
			uint16 mu16Guard;	//!< Redundant 16-bit guard

			//! Construct zero-valued instance
			inline uint48(void) :
				mu16Guard(0x0), mu16High(0x0), mu32Low(0x0) {
			}

			//! Construct instance from MS16 and LS32 bits
			inline uint48(uint16 u16High, uint32 u32Low) :
				mu16Guard(0x0), mu16High(u16High), mu32Low(u32Low) {
			}
				
			//! Equivalence test
			inline bool operator==(const uint48 &other) const {
				return ((mu16High == other.mu16High) && (mu32Low == other.mu32Low));
			}

			//! Assignment operator
			inline uint48 &operator=(const uint48 &other) {
				mu16Guard = other.mu16Guard;
				mu16High = other.mu16High;
				mu32Low = other.mu32Low;
				return *this;
			}
	};

	//! 64-bit unsigned integer
	class uint64 {
		public:
			uint32 mu32Low;		//!< LS32 of value
			uint32 mu32High;	//!< MS32 of value

			//! Construct a zero-valued instance
			inline uint64(void) :
				mu32High(0x0), mu32Low(0x0) {
			}

			//! Construct instance by specifying MS32 and LS32
			inline uint64(uint32 u32High, uint32 u32Low) :
				mu32High(u32High), mu32Low(u32Low) {
			}

			//! Equvalence test
			inline bool operator==(const uint64 &other) const {
				return ((mu32High == other.mu32High) && (mu32Low == other.mu32Low));
			}

			//! Assignment operator
			inline uint64 &operator=(const uint64 &other) {
				mu32High = other.mu32High;
				mu32Low = other.mu32Low;
				return *this;
			}

			//! Partial assignment operator - sets MS32 of result as zero
			inline uint64 &operator=(const int other) {
				mu32High = 0x0;
				mu32Low = other;
				return *this;
			}
	};
	
};	// ::hss1394

#endif // _HSS1394Types_

