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
		freeLocalSlots(0), settings(QSettings::IniFormat, QSettings::UserScope, "ddcn", "ddcn") {
	// Load peer name and public key from configuration
	if (!settings.value("name").isValid()) {
		settings.setValue("name", "ddcn_node");
	}
	QString name = settings.value("general/name").toString();
	// Load key from file in the settings directory
	QString keyFile = QFileInfo(settings.fileName()).absolutePath() + "/privkey.pem";
	PrivateKey key = PrivateKey::load(keyFile);
	if (!key.isValid()) {
		key = PrivateKey::generate(2048);
		if (!key.save(keyFile)) {
			qWarning("Warning: Could not save local private key!");
		}
	}
	// Load trusted peers/groups etc from file
	// TODO
	// Initialize ariba
	network = new NetworkInterface(name, key);
	connect(network,
	        SIGNAL(peerConnected(NetworkNode*, QString, PublicKey)),
	        this,
	        SLOT(onPeerConnected(NetworkNode*, QString, PublicKey)));
	connect(network,
	        SIGNAL(peerDisconnected(NetworkNode*)),
	        this,
	        SLOT(onPeerDisconnected(NetworkNode*)));
	connect(network,
	        SIGNAL(peerChanged(NetworkNode*, QString)),
	        this,
	        SLOT(onPeerChanged(NetworkNode*, QString)));
	connect(network,
	        SIGNAL(messageReceived(NetworkNode*, Packet)),
	        this,
	        SLOT(onMessageReceived(NetworkNode*, Packet)));
	connect(network,
	        SIGNAL(groupMessageReceived(McpoGroup*, NetworkNode*, Packet)),
	        this,
	        SLOT(onGroupMessageReceived(McpoGroup*, NetworkNode*, Packet)));
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

void CompilerNetwork::setLocalKey(const PrivateKey &privateKey) {
	localKey = privateKey;
	// TODO: Change key in NetworkInterface
	//network->changeIdentity(peerName, publicKey);
	// Save key
	QString keyFile = QFileInfo(settings.fileName()).absolutePath() + "/privkey.pem";
	if (!privateKey.save(keyFile)) {
		qWarning("Warning: Could not save local private key!");
	}
	emit localKeyChanged(privateKey);
}
void CompilerNetwork::generateLocalKey() {
	// TODO: Let the user choose the key length
	PrivateKey privateKey = PrivateKey::generate(2048);
	setLocalKey(privateKey);
}
PrivateKey CompilerNetwork::getLocalKey() {
	return localKey;
}

void CompilerNetwork::addTrustedPeer(QString name, const PublicKey &publicKey) {
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
	emit trustedPeersChanged(trustedPeers);
}
void CompilerNetwork::removeTrustedPeer(QString name, const PublicKey &publicKey) {
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
	emit trustedPeersChanged(trustedPeers);
}

void CompilerNetwork::addTrustedGroup(QString name, const PublicKey &publicKey) {
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
	emit trustedGroupsChanged(trustedGroups);
}
void CompilerNetwork::removeTrustedGroup(QString name, const PublicKey &publicKey) {
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
	emit trustedGroupsChanged(trustedGroups);
}

void CompilerNetwork::addGroupMembership(QString name, const PrivateKey &privateKey) {
	GroupMembership *existing = getGroupMembership(privateKey);
	if (existing) {
		existing->setName(name);
		return;
	}
	GroupMembership *groupMembership = new GroupMembership(name, privateKey);
	groupMemberships.append(groupMembership);
	// Join the group
	ariba::ServiceID serviceId = McpoGroup::getServiceIdFromPublicKey(privateKey);
	groupMembership->setMcpoGroup(network->joinGroup(serviceId));
	saveSettings();
	emit groupMembershipsChanged(groupMemberships);
}
void CompilerNetwork::removeGroupMembership(QString name, const PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (GroupMembership *groupMembership, groupMemberships) {
		if (groupMembership->getPrivateKey() == publicKey) {
			network->leaveGroup(groupMembership->getMcpoGroup());
			groupMemberships.removeOne(groupMembership);
			delete groupMembership;
			return;
		}
	}
	saveSettings();
	emit groupMembershipsChanged(groupMemberships);
}

void CompilerNetwork::delegateOutgoingJob(Job *job) {
	waitingJobs.append(job);
	if (freeRemoteSlots.size() > 0) {
		// TODO: Delegate the job
		// If the free remote slots are too little, ask all nodes for free slots
		// TODO: Better threshold here, like 0.5 * maxFreeRemoteSlots
		if (freeRemoteSlots.count() < 4) {
			askForFreeSlots();
		}
	} else {
		askForFreeSlots();
	}
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

void CompilerNetwork::queryNetworkStatus() {
	qDebug("queryNetworkStatus");
	// Send network status requests to all connected peers
	Packet packet(PacketType::QueryNodeStatus);
	network->sendToAll(packet);
}

void CompilerNetwork::onPeerConnected(NetworkNode *node, QString name,
		const PublicKey &publicKey) {
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
void CompilerNetwork::onMessageReceived(NetworkNode *node, const Packet &packet) {
	switch (packet.getType()) {
		case PacketType::QueryNodeStatus:
			qDebug("Node asking for node status.");
			reportNodeStatus(node);
			break;
		case PacketType::NodeStatus:
			onNodeStatusChanged(node, packet);
			break;
		default:
			qWarning("Warning: Unknown package type received.");
			break;
	}
}
void CompilerNetwork::onGroupMessageReceived(McpoGroup *group, NetworkNode *node,
		const Packet &packet) {
	switch (packet.getType()) {
		case PacketType::QueryNetworkResources:
			qDebug("Node asking for network resources.");
			reportNetworkResources(node);
			break;
		default:
			qWarning("Warning: Unknown group package type received.");
			break;
	}
}

TrustedPeer *CompilerNetwork::getTrustedPeer(const PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (TrustedPeer *trustedPeer, trustedPeers) {
		if (trustedPeer->getPublicKey() == publicKey) {
			return trustedPeer;
		}
	}
	return NULL;
}
TrustedGroup *CompilerNetwork::getTrustedGroup(const PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (TrustedGroup *trustedGroup, trustedGroups) {
		if (trustedGroup->getPublicKey() == publicKey) {
			return trustedGroup;
		}
	}
	return NULL;
}
GroupMembership *CompilerNetwork::getGroupMembership(const PublicKey &publicKey) {
	// TODO: Way too slow
	foreach (GroupMembership *groupMembership, groupMemberships) {
		if (groupMembership->getPrivateKey() == publicKey) {
			return groupMembership;
		}
	}
	return NULL;
}

void CompilerNetwork::saveSettings() {
	// TODO: Save trusted groups, trusted peers, group memberships
}

void CompilerNetwork::askForFreeSlots() {
	//  Send a message to all trusted peers
	Packet packet(PacketType::QueryNetworkResources);
	foreach (TrustedPeer *trustedPeer, trustedPeers) {
		network->send(trustedPeer->getNetworkNode(), packet);
	}
	// The packet sent to the groups has to look different as we have to
	// include the group key
	foreach (TrustedGroup *trustedGroup, trustedGroups) {
		// Create a new packet for each group containing the group key
		QByteArray payload = trustedGroup->getPublicKey().toDER();
		packet = Packet(PacketType::QueryNetworkResources, payload);
		network->send(trustedGroup->getMcpoGroup(), packet);
	}
}

void CompilerNetwork::reportNodeStatus(NetworkNode *node) {
	NodeStatusPacket nodeStatus;
	// TODO: Get real data
	nodeStatus.maxThreads = 2;
	nodeStatus.currentThreads = 1;
	nodeStatus.delegatedJobs = 0;
	nodeStatus.remoteJobs = 0;
	nodeStatus.groupCount = groupMemberships.count();
	QByteArray packetData;
	packetData.resize(sizeof(nodeStatus));
	memcpy(packetData.data(), &nodeStatus, sizeof(nodeStatus));
	QDataStream stream(&packetData, QIODevice::Append);
	foreach (GroupMembership *groupMembership, groupMemberships) {
		QByteArray key = groupMembership->getPrivateKey().toDER();
		stream << (unsigned int)key.size();
		stream << key;
	}
	Packet packet(PacketType::NodeStatus, packetData);
	network->send(node, packet);
}
void CompilerNetwork::reportNetworkResources(NetworkNode *node) {
	// TODO
}

void CompilerNetwork::onNodeStatusChanged(NetworkNode *node, const Packet &packet) {
	qDebug("onNodeStatusChanged");
	// Parse node info
	const NodeStatusPacket *nodeStatus = packet.getPayload<NodeStatusPacket>();
	if (!nodeStatus) {
		qWarning("Warning: Received too small packet (%d/%d).", packet.getPayloadSize(), sizeof(*nodeStatus));
		return;
	}
	NodeStatus status;
	status.currentThreads = nodeStatus->currentThreads;
	status.maxThreads =nodeStatus->maxThreads;
	status.delegatedJobs = nodeStatus->delegatedJobs;
	status.localJobs = nodeStatus->localJobs;
	status.remoteJobs = nodeStatus->remoteJobs;
	// Parse group info
	unsigned int groupCount = nodeStatus->groupCount;
	char *groupData = (char*)packet.getPayloadData() + sizeof(NodeStatusPacket);
	QByteArray remaining(groupData, packet.getPayloadSize() - sizeof(NodeStatusPacket));
	QDataStream stream(remaining);
	QStringList groupKeys;
	for (unsigned int i = 0; i < groupCount; i++) {
		unsigned int keyLength = 0;
		stream >> keyLength;
		QByteArray keyData;
		stream >> keyData;
		if (stream.atEnd()) {
			break;
		}
		PublicKey key = PublicKey::fromDER(keyData);
		groupKeys.append(key.toPEM());
	}
	QString fingerprint = node->getPublicKey().fingerprint();
	emit nodeStatusChanged(node->getPublicKey().toPEM(), fingerprint, status, groupKeys);
}
