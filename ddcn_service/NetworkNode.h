/*
Copyright 2011 Benjamin Fus, Florian Muenchbach, Mathias Gottschlag. All
rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NETWORKNODE_H_INCLUDED
#define NETWORKNODE_H_INCLUDED

#include "TLS.h"
#include "Protocol.h"

#include <QString>
#include <ariba/ariba.h>

class TrustedPeer;

/**
 * Holds the information about the connection to another peer in the network.
 */
class NetworkNode : public QObject {
	Q_OBJECT
public:
	/**
	 * Constructor.
	 */
	NetworkNode(ariba::utility::NodeID nodeId, ariba::utility::LinkID linkId);
	/**
	 * Returns the public key of this peer. This key is always correct and has
	 * been verified in the TLS handshake.
	 */
	PublicKey getPublicKey() {
		return publicKey;
	}
	/**
	 * Marks the peer as trusted.
	 */
	void setTrustedPeer(TrustedPeer *trustedPeer) {
		this->trustedPeer = trustedPeer;
	}
	/**
	 * Returns the TrustedPeer instance connected to this node or NULL when we
	 * do not trust the peer.
	 */
	TrustedPeer *getTrustedPeer() {
		return trustedPeer;
	}
	/**
	 * Sends a packet to the peer. Use NetworkInterface::send() instead.
	 */
	void sendPacket(const Packet &packet);

	/**
	 * Returns the next expected serial number for the next packet from this
	 * peer. This is used in NetworkInterface for some basic error detection.
	 */
	unsigned short getNextExpectedSerial() {
		return ++lastExpectedSerial;
	}
	/**
	 * Returns the next serial number for the next packet sent to this peer.
	 * This is used in NetworkInterface for some basic error detection.
	 */
	unsigned short getNextOutgoingSerial() {
		return ++lastOutgoingSerial;
	}
signals:
	/**
	 * Triggered when there is data which should be sent by NetworkInterface.
	 */
	void outgoingDataAvailable(NetworkNode *node);
	/**
	 * Triggered if a complete packet was received from the TLS connection.
	 */
	void packetReceived(NetworkNode *node, const Packet &packet);
	/**
	 * Triggered when the TLS stream is ready.
	 */
	void connectionReady(NetworkNode *node);
private slots:
	void onOutgoingDataAvailable();
	void onIncomingDataAvailable();
	void onHandshakeComplete();
private:
	TLS &getTLS() {
		return tls;
	}

	ariba::NodeID aribaNode;
	ariba::LinkID aribaLink;
	PublicKey publicKey;
	TrustedPeer *trustedPeer;

	TLS tls;

	QByteArray incomingData;

	unsigned short lastExpectedSerial;
	unsigned short lastOutgoingSerial;

	friend class NetworkInterface;
};

#endif
