///////////////////////////////////////////////////////////////////////////////
//! \file HSS1394.h
//!
//! C++ API file for 1394 High-Speed Serial interface. This interface is designed
//! to provide a low-latency data transport between endpoints in an IEEE1394
//! network that are configured with a HSS1394 endpoint. This interface provides
//! management and data passage functionality for the host end of this interface.
//!
//! A HSS1394 network is superimposed upon a 1394 network. A HSS1394 network
//! consists of a set of nodes with a simple common interface allowing them
//! to exchange information with a host node (star network topology) via
//! full-duplex data channels.
//
//! This interface is not thread-safe, and therefore the user should ensure that
//! multiple concurrent invocations through this interface are protected using
//! some external synchronization mechanism.
//! 
//! Created by Don Goodeve (don@bearanascence.com), 28Apr2008, 11:30PST
//! this file Copyright (C) 2009, Stanton Group.
//! $Id: HSS1394.h 1563 2009-04-16 06:48:34Z don $
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

#ifndef _HSS1394_
#define _HSS1394_


#if defined(_WIN32)
#if defined(HSS1394_EXPORT_DLL)
#define HSS1394_CLASS_CONVENTION __declspec(dllexport)
#elif defined(HSS1394_IMPORT_DLL)
#define HSS1394_CLASS_CONVENTION __declspec(dllimport)
#else
#define HSS1394_CLASS_CONVENTION  // static import
#endif

#else // _WIN32

#define HSS1394_CLASS_CONVENTION  // static import

#endif // _WIN32


//*** Includes
//*****************************************************************************
#include <string>			// STL string class
#include "HSS1394Types.h"


//*** Types
//*****************************************************************************
//! HSS1394 namespace encapsulates all HSS1394 functionality.
namespace hss1394 {

	// TNodeInfo
	//! Contains information about a discovered HSS1394 node on the 1394 net.
	//-------------------------------------------------------------------------
	typedef struct {
		std::string sName;			//!< Textual name of node.
		uint64 uGUID;				//!< Node 1394 Global Unique ID.
		uint16 uProtocolVersion;	//!< HSS1394 protocol code for this node
	}TNodeInfo;
};


//*** Class declarations
//*****************************************************************************
namespace hss1394 {

	// Listener::
	//! Abstract base class for an object informed when the connection state of the
	//! HSS1394 interface changes - ie. connections appear or disappear on the
	//! network.
	//-----------------------------------------------------------------------------
	class HSS1394_CLASS_CONVENTION Listener {
		public:
			//! ::Do method is called when the nodes in the HSS1394 network have changed.
			virtual void Do(void) = 0;
	};


	// ChannelListener::
	//! Callback object for an incomming data channel. 
	//-----------------------------------------------------------------------------
	class HSS1394_CLASS_CONVENTION ChannelListener {
		public:
			// :: Process is called when data has arrived. The arguments present
			//! the data. On completion of this call, pBuffer is no longer valid.
			//! This call will occur inside a separate thread. The implementer must
			//! ensure concurrency control issues are dealt with.
			virtual void Process(const uint8 *pBuffer, uint uBufferSize) { }

			// ::Disconnected is called when the node attached to this channel becomes
			//! unavailable. 
			virtual void Disconnected(void) { }

			// ::Reconnected is called when the node attached to this channel becomes
			//! available again.
			virtual void Reconnected(void) { }
	};


	// Channel::
	//! Abstraction of a channel endpoint at the host communicating with a
	//! remote HSS1394 node.
	//-----------------------------------------------------------------------------
	class HSS1394_CLASS_CONVENTION Channel {
		public:
			//! Stop data transmission/reception. After this call all send and
			//! receive calls will fail with a zero return.
			virtual void Stop(void) = 0;

			//! Re-start data transmission/reception.
			virtual void Restart(void) = 0;

			//! Cause send and receive data buffers to be flushed.
			virtual void Flush(void) = 0;

			//! Return an information structure describing the node connected
			//! via this channel.
			virtual void GetNodeInfo(TNodeInfo &tNodeInfo) = 0;

			//! Send the specified buffer of the specified length. On success
			//! returns the number of bytes actually sent. This may be less
			//! than uBufferSize and specifies the size of the prefix within
			//! pBuffer which has been sent.
			virtual uint SendChannelBytes(const uint8 *pBuffer, uint uBufferSize, bool bForce = true) = 0;

			//! Send a packet to be echoed back (testing purposes). Packet will
			//! not be passed to application layer on receiving node.
			virtual uint SendChannelEcho(const uint8 *pBuffer, uint uBufferSize) = 0;

			//! Return the number of send retries that have occurred since the last
			//! call to ::GetRetries or since the channel was created.
			virtual uint GetRetries(void) = 0;

			//! Receive up to the specified number of bytes into pBuffer
			//! from the channel. Returns the number of bytes received.
			//! If a channel listener is installed, will always return
			//! zero.
			virtual uint ReceiveChannelBytes(uint8 *pBuffer, uint uBufferSize) = 0;

			//! Install (or uninstall) a channel listener object onto
			//! the receive channel. If the argument is NULL, then the
			//! existing listener (if any) will be removed. Storage
			//! management of the listener object remains the responsibility
			//! of the caller. On the initial call, any data in the buffer will
			//! be immediately passed to the just-installed listener, flushing
			//! the receive buffer.
			virtual bool InstallChannelListener(ChannelListener *pListener) = 0;

			//! Returns TRUE if there is a channel listener installed on this channel,
			//! false otherwise.
			virtual bool ChannelListenerExists(void) = 0;

			//! Send a user control message via the channel to the target
			//! node. Returns the number of bytes sent on success, zero on
			//! failure. uUserTag is valid in the range 0x00 - 0xDF. All other
			//! tag values will be rejected (0 return).
			virtual uint SendUserControl(uint8 uUserTag, const uint8 *pUserData, uint uDataBytes) = 0;
	};


	// Node::
	//! Singleton High-speed serial over 1394 host node interface.
	//-----------------------------------------------------------------------------
	class HSS1394_CLASS_CONVENTION Node {
		protected:
			static Node *mspInstance;	//!< Singleton instance of class

			// Dispose of class and reclaim all resources. Closes all channels.
			// All channel objects created by the interface will be destroyed. It
			// is up to the caller to clean up any pointers remaining to channel
			// objects.
			virtual ~Node() { };
		
		public:
			//! Return a pointer to the signeton 1394 host-node interface. On the
			//! first call this will result in class initialization.
			static Node *Instance(void);

			//! Shutdown the current instance - cleanup
			static void Shutdown(void);

			//! Add a user-defined connection listener callback object. This object's
			//! ::Do method will be invoked whenever the topology of the HSS1394 network
			//! changes. Note that this will *not* occur on every 1394 bus reset. Note that
			//! this call may occur from a separate thread. The user must deal with
			//! any concurrency control issues.
			virtual void InstallConnectionListener(Listener *pListener) = 0;

			//! Return the number of HSS1394 nodes available to connect to.
			virtual uint GetNodeCount(void) = 0;

			//! Return information about the specified node. if pbAvailable is not NULL,
			//! on return *pbAvailable==TRUE implies that a channel may be opened with this
			//! node. Note that exclusivity requires a maximum of one channel is open with
			//! any network node at any time. If pbInstalled is not NULL, on return, the
			//! *pbInstalled variable will be set if sufficient driver resources are 
			//! configured to allow normal operation with this node. This being set to
			//! false may indicate the need to complete installation of drivers for this
			//! node.
			virtual bool GetNodeInfo(TNodeInfo &tNodeInfo, uint uNode, bool *pbAvailable = NULL, bool *pbInstalled = NULL) = 0;

			//! Open the specified channel index, returning a valid channel object
			//! or NULL on failure.
			virtual Channel *OpenChannel(uint uNode) = 0;

			//! Release the specified Channel object. This closes the channel, permitting
			//! it to be re-opened in future (if the connection is still available). On
			//! failure, returns FALSE.
			virtual bool ReleaseChannel(Channel *pChannel) = 0;
	};


};	// namespace hss1394


#endif // _HSS1394_


