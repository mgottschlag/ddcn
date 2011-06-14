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

#include "CompilerNetwork.h"

CompilerNetwork::CompilerNetwork() : encryptionEnabled(true) {
	// Load peer name and public key from configuration
	QString name = "ddcn_node";
	QCA::PrivateKey key;
	// TODO
	// Initialize ariba
	network = new NetworkInterface(name, key);
	connect(network,
	        SIGNAL(peerConnected(NodeID, QString, QCA::PublicKey)),
	        this,
	        SLOT(onPeerConnected(NodeID, QString, QCA::PublicKey)));
	connect(network,
	        SIGNAL(peerDisconnected(NodeID)),
	        this,
	        SLOT(onPeerDisconnected(NodeID)));
	connect(network,
	        SIGNAL(peerChanged(NodeID, QString)),
	        this,
	        SLOT(onPeerChanged(NodeID, QString)));
	connect(network,
	        SIGNAL(messageReceived(NodeID, QByteArray)),
	        this,
	        SLOT(onMessageReceived(NodeID, QByteArray)));
	connect(network,
	        SIGNAL(groupMessageReceived(McpoGroup*, NodeID, QByteArray)),
	        this,
	        SLOT(onGroupMessageReceived(McpoGroup*, NodeID, QByteArray)));
}
CompilerNetwork::~CompilerNetwork() {
	delete network
;
}

void CompilerNetwork::setPeerName(QString peerName) {
	this->peerName = peerName;
	network->setName(peerName);
	emit peerNameChanged(peerName);
}
QString CompilerNetwork::getPeerName() {
	return peerName;
}

void CompilerNetwork::setEncryption(bool encryptionEnabled) {
	this->encryptionEnabled = encryptionEnabled;
	// TODO: Disable encryption in NetworkNode
	emit encryptionChanged(encryptionEnabled);
}
bool CompilerNetwork::getEncryption() {
	return encryptionEnabled;
}

void CompilerNetwork::setKeys(QCA::PublicKey publicKey, QCA::PrivateKey privateKey) {
	this->publicKey = publicKey;
	this->privateKey = privateKey;
	// TODO
	//network->changeIdentity(peerName, publicKey);
	emit publicKeyChanged(publicKey);
	emit privateKeyChanged(privateKey);
}
void CompilerNetwork::generateKeys() {
	// TODO
}
QCA::PublicKey CompilerNetwork::getPublicKey() {
	return publicKey;
}
QCA::PrivateKey CompilerNetwork::getPrivateKey() {
	return privateKey;
}

void CompilerNetwork::addTrustedPeer(QString name, const QCA::PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (TrustedPeer *other, trustedPeers) {
		if (other->getPublicKey() == publicKey) {
			// TODO: We might want to update the name here?
			return;
		}
	}
	TrustedPeer *trustedPeer = new TrustedPeer(name, publicKey);
	trustedPeers.append(trustedPeer);
	NetworkNode *node = network->getNetworkNode(publicKey);
	if (node) {
		node->setTrustedPeer(trustedPeer);
		trustedPeer->setNetworkNode(node);
	}
}
void CompilerNetwork::removeTrustedPeer(QString name, const QCA::PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (TrustedPeer *trustedPeer, trustedPeers) {
		if (trustedPeer->getPublicKey() == publicKey) {
			if (trustedPeer->getNetworkNode() != NULL) {
				trustedPeer->getNetworkNode()->setTrustedPeer(NULL);
			}
			trustedPeers.removeOne(trustedPeer);
			delete trustedPeer;
			return;
		}
	}
}

void CompilerNetwork::addTrustedGroup(QString name, const QCA::PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (TrustedGroup *other, trustedGroups) {
		if (other->getPublicKey() == publicKey) {
			// TODO: We might want to update the name here?
			return;
		}
	}
	TrustedGroup *trustedGroup = new TrustedGroup(name, publicKey);
	trustedGroups.append(trustedGroup);
	// Join the group
	ariba::ServiceID serviceId = McpoGroup::getServiceIdFromPublicKey(publicKey);
	trustedGroup->setMcpoGroup(network->joinGroup(serviceId));
}
void CompilerNetwork::removeTrustedGroup(QString name, const QCA::PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (TrustedGroup *trustedGroup, trustedGroups) {
		if (trustedGroup->getPublicKey() == publicKey) {
			network->leaveGroup(trustedGroup->getMcpoGroup());
			trustedGroups.removeOne(trustedGroup);
			delete trustedGroup;
			return;
		}
	}
}

void CompilerNetwork::addGroupMembership(QString name, const QCA::PublicKey &publicKey, const QCA::PrivateKey &privateKey) {
	// TODO: Way too slow
	foreach (GroupMembership *other, groupMemberships) {
		if (other->getPublicKey() == publicKey) {
			// TODO: We might want to update the name here?
			return;
		}
	}
	GroupMembership *groupMembership = new GroupMembership(name, publicKey, privateKey);
	groupMemberships.append(groupMembership);
	// Join the group
	ariba::ServiceID serviceId = McpoGroup::getServiceIdFromPublicKey(publicKey);
	groupMembership->setMcpoGroup(network->joinGroup(serviceId));
}
void CompilerNetwork::removeGroupMembership(QString name, const QCA::PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (GroupMembership *groupMembership, groupMemberships) {
		if (groupMembership->getPublicKey() == publicKey) {
			network->leaveGroup(groupMembership->getMcpoGroup());
			groupMemberships.removeOne(groupMembership);
			delete groupMembership;
			return;
		}
	}
}

bool CompilerNetwork::canAcceptOutgoingJobRequest() {
	// TODO
	return false;
}
void rejectIncomingJobReqiest(JobRequest *request) {
	// TODO
}
void acceptIncomingJobRequest(JobRequest *request) {
	// TODO
}
bool CompilerNetwork::delegateOutgoingJob(Job *job) {
	// TODO
	return false;
}

void CompilerNetwork::onPeerConnected(NodeID node, QString name,
		const QCA::PublicKey &publicKey) {
	// TODO
}
void CompilerNetwork::onPeerDisconnected(NodeID node) {
	// TODO
}
void CompilerNetwork::onPeerChanged(NodeID node, QString name) {
	// TODO
}
void CompilerNetwork::onMessageReceived(NodeID node, const QByteArray &message) {
	// TODO
}
void CompilerNetwork::onGroupMessageReceived(McpoGroup *group, NodeID node,
		const QByteArray &message) {
	// TODO
}
