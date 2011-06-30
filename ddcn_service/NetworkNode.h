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

THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL <COPYRIGHT HOLDER> OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
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

class NetworkNode : public QObject {
	Q_OBJECT
public:
	NetworkNode(ariba::utility::NodeID nodeId, ariba::utility::LinkID linkId);
	PublicKey getPublicKey() {
		return publicKey;
	}
	void setTrustedPeer(TrustedPeer *trustedPeer) {
		this->trustedPeer = trustedPeer;
	}
	TrustedPeer *getTrustedPeer() {
		return trustedPeer;
	}
	void sendPacket(const Packet &packet);
signals:
	void outgoingDataAvailable(NetworkNode *node);
	void packetReceived(NetworkNode *node, const Packet &packet);
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

	// TODO: Remove this
	friend class NetworkInterface;
};

#endif
