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

CompilerNetwork::CompilerNetwork() : encryptionEnabled(true),
		freeLocalSlots(0) {
	// Load peer name and public key from configuration
	QString name = "ddcn_node";
	QCA::PrivateKey key;
	// TODO
	// Load trusted peers/groups etc from file
	// TODO
	// Initialize ariba
	network = new NetworkInterface(name, key);
	connect(network,
	        SIGNAL(peerConnected(NetworkNode*, QString, QCA::PublicKey)),
	        this,
	        SLOT(onPeerConnected(NetworkNode*, QString, QCA::PublicKey)));
	connect(network,
	        SIGNAL(peerDisconnected(NetworkNode*)),
	        this,
	        SLOT(onPeerDisconnected(NetworkNode*)));
	connect(network,
	        SIGNAL(peerChanged(NetworkNode*, QString)),
	        this,
	        SLOT(onPeerChanged(NetworkNode*, QString)));
	connect(network,
	        SIGNAL(messageReceived(NetworkNode*, QByteArray)),
	        this,
	        SLOT(onMessageReceived(NetworkNode*, QByteArray)));
	connect(network,
	        SIGNAL(groupMessageReceived(McpoGroup*, NetworkNode*, QByteArray)),
	        this,
	        SLOT(onGroupMessageReceived(McpoGroup*, NetworkNode*, QByteArray)));
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
	TrustedPeer *existing = getTrustedPeer(publicKey);
	if (existing) {
		existing->setName(name);
		return;
	}
	TrustedPeer *trustedPeer = new TrustedPeer(name, publicKey);
	trustedPeers.append(trustedPeer);
	NetworkNode *node = network->getNetworkNode(publicKey);
	if (node) {
		node->setTrustedPeer(trustedPeer);
		trustedPeer->setNetworkNode(node);
	}
	saveSettings();
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
	saveSettings();
}

void CompilerNetwork::addTrustedGroup(QString name, const QCA::PublicKey &publicKey) {
	TrustedGroup *existing = getTrustedGroup(publicKey);
	if (existing) {
		existing->setName(name);
		return;
	}
	TrustedGroup *trustedGroup = new TrustedGroup(name, publicKey);
	trustedGroups.append(trustedGroup);
	// Join the group
	ariba::ServiceID serviceId = McpoGroup::getServiceIdFromPublicKey(publicKey);
	trustedGroup->setMcpoGroup(network->joinGroup(serviceId));
	saveSettings();
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
	saveSettings();
}

void CompilerNetwork::addGroupMembership(QString name, const QCA::PublicKey &publicKey, const QCA::PrivateKey &privateKey) {
	GroupMembership *existing = getGroupMembership(publicKey);
	if (existing) {
		existing->setName(name);
		return;
	}
	GroupMembership *groupMembership = new GroupMembership(name, publicKey, privateKey);
	groupMemberships.append(groupMembership);
	// Join the group
	ariba::ServiceID serviceId = McpoGroup::getServiceIdFromPublicKey(publicKey);
	groupMembership->setMcpoGroup(network->joinGroup(serviceId));
	saveSettings();
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
	saveSettings();
}

void CompilerNetwork::delegateOutgoingJob(Job *job) {
	waitingJobs.append(job);
	// TODO: Check whether we have free remote slots left and delegate the job
}
Job *CompilerNetwork::cancelOutgoingJob() {
	if (!waitingJobs.empty()) {
		Job *job = waitingJobs.last();
		waitingJobs.removeLast();
		return job;
	}
	// TODO: Check whether we can abort an already delegated job
	return NULL;
}
void CompilerNetwork::rejectIncomingJob(Job *job) {
	// TODO
}

void CompilerNetwork::setFreeLocalSlots(unsigned int localSlots) {
	freeLocalSlots = localSlots;
}
unsigned int CompilerNetwork::getFreeLocalSlots() {
	return freeLocalSlots;
}

void CompilerNetwork::onPeerConnected(NetworkNode *node, QString name,
		const QCA::PublicKey &publicKey) {
	TrustedPeer *trustedPeer = getTrustedPeer(publicKey);
	if (trustedPeer) {
		trustedPeer->setNetworkNode(node);
	}
	// TODO: Emit spareResources() ?
}
void CompilerNetwork::onPeerDisconnected(NetworkNode *node) {
	if (node->getTrustedPeer()) {
		node->getTrustedPeer()->setNetworkNode(NULL);
	}
	// Reject local jobs delegated to this node
	// TODO
	// Abort remote jobs from this node
	// TODO
}
void CompilerNetwork::onPeerChanged(NetworkNode *node, QString name) {
	// TODO: Nothing to do here? Maybe update TrustedPeer name
}
void CompilerNetwork::onMessageReceived(NetworkNode *node, const QByteArray &message) {
	// TODO
}
void CompilerNetwork::onGroupMessageReceived(McpoGroup *group, NetworkNode *node,
		const QByteArray &message) {
	// TODO
}

TrustedPeer *CompilerNetwork::getTrustedPeer(const QCA::PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (TrustedPeer *trustedPeer, trustedPeers) {
		if (trustedPeer->getPublicKey() == publicKey) {
			return trustedPeer;
		}
	}
	return NULL;
}
TrustedGroup *CompilerNetwork::getTrustedGroup(const QCA::PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (TrustedGroup *trustedGroup, trustedGroups) {
		if (trustedGroup->getPublicKey() == publicKey) {
			return trustedGroup;
		}
	}
	return NULL;
}
GroupMembership *CompilerNetwork::getGroupMembership(const QCA::PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (GroupMembership *groupMembership, groupMemberships) {
		if (groupMembership->getPublicKey() == publicKey) {
			return groupMembership;
		}
	}
	return NULL;
}

void CompilerNetwork::saveSettings() {
	// TODO: Save trusted groups, trusted peers, group memberships
}
