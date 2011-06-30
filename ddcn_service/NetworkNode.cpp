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

#include "NetworkNode.h"
#include "NetworkInterface.h"

#include <QtEndian>

NetworkNode::NetworkNode(ariba::utility::NodeID nodeId, ariba::utility::LinkID linkId) : aribaNode(nodeId),
		aribaLink(linkId), trustedPeer(NULL) {
	connect(&tls, SIGNAL(readyReadOutgoing()), this,
		SLOT(onOutgoingDataAvailable()));
	connect(&tls, SIGNAL(readyRead()), this,
		SLOT(onIncomingDataAvailable()));
	connect(&tls, SIGNAL(handshakeComplete()), this,
		SLOT(onHandshakeComplete()));
	// TODO: Connect to TLS error signal?
}

void NetworkNode::sendPacket(QByteArray packet) {
	if ((size_t)packet.size() < sizeof(PacketHeader)) {
		qCritical("Sending packet without packet header.");
		return;
	}
	PacketHeader::insertPacketHeaderLength(packet);;
	tls.write(packet);
}

void NetworkNode::onOutgoingDataAvailable() {
	emit outgoingDataAvailable(this);
}
void NetworkNode::onIncomingDataAvailable() {
	incomingData += tls.read();
	// Read packets until not enough data is left
	PacketHeader header;
	while ((size_t)incomingData.size() > sizeof(PacketHeader)) {
		uint32_t dataSize = incomingData.size();
		memcpy(&header, incomingData.data(), sizeof(header));
		uint32_t packetSize = qFromBigEndian(header.size);
		if (dataSize >= packetSize) {
			if (dataSize == packetSize) {
				emit packetReceived(this, incomingData);
				incomingData = QByteArray();
			} else {
				// Leave bytes which do not form a complete packet in the queue
				emit packetReceived(this, incomingData.left(packetSize));
				incomingData = incomingData.right(dataSize - packetSize);
			}
		} else {
			break;
		}
	}
}

void NetworkNode::onHandshakeComplete() {
	qCritical("Handshaken!");
	Certificate cert = tls.getPeerCertificate();
	if (!cert.isValid()) {
		qCritical("Remote cert is null!");
	} else {
		publicKey = cert.getPublicKey();
	}
	// TODO: Peer name
	emit connectionReady(this);
}
