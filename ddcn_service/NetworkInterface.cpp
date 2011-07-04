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

#include "NetworkInterface.h"

#include <ariba/utility/system/StartupWrapper.h>
#include <QDebug>
#include <QtEndian>
#include <QThread>

uint qHash(ariba::utility::NodeID nodeId) {
	return qHash(nodeId.toString().c_str());
}

struct GroupMessage {
	ariba::utility::NodeID nodeId;
	ariba::ServiceID serviceId;
	Packet packet;
};
struct PeerMessage {
	ariba::utility::NodeID nodeId;
	ariba::utility::LinkID linkId;
	// This is an encrypted stream, so no Packet here
	QByteArray message;
	unsigned short serial;
};

class DdcnMessage : public ariba::Message {
	VSERIALIZEABLE;
public:
	DdcnMessage() : serial(0) {
	}
	DdcnMessage(unsigned short serial) : serial(serial) {
	}
	virtual ~DdcnMessage() {
	}

	QByteArray &getData() {
		return data;
	}

	unsigned short getSerial() {
		return serial;
	}

	unsigned short getChecksum() {
		return qChecksum(data.data(), data.size());
	}
private:
	unsigned short serial;
	QByteArray data;
};

sznBeginDefault(DdcnMessage, X) {
	if (X.isDeserializer() && X.getRemainingLength() < 6 * 8) {
		qCritical("Empty packet without header received.");
		data = QByteArray();
		// TODO
	} else {
	quint16 checksum;
	if (X.isSerializer()) {
		checksum = qChecksum(data.data(), data.size());
	}
	X && serial;
	X && checksum;
	unsigned int size = data.size();
	X && size;
	data.resize(size);

	// Limit packets to 1MB
	// TODO: Better limit
	size = std::min(size, 1u << 20);

	if (X.isSerializer()) {
		data.resize(size);
	}
	// TODO: This can be optimized
	for (unsigned int i = 0; i < size; i++) {
		if (X.isDeserializer() && X.getRemainingLength() < 8) {
			qCritical("Not enough data in packet!");
			// TODO
			break;
		}
		unsigned char c = data[i];
		X && c;
		data[i] = c;
	}
	if (X.isDeserializer()) {
		if (checksum != qChecksum(data.data(), data.size())) {
			qFatal("Checksums do not match!");
		}
	}
	}
} sznEnd();

vsznDefault(DdcnMessage);

class DdcnGroupMessage : public ariba::Message {
	VSERIALIZEABLE;
public:
	DdcnGroupMessage() {
	}
	DdcnGroupMessage(std::string nodeId, std::string serviceId, std::string text)
			: nodeId(nodeId), serviceId(serviceId), text(text) {
	}
	virtual ~DdcnGroupMessage() {
	}

	std::string getNodeId() {
		return nodeId;
	}
	std::string getServiceId() {
		return serviceId;
	}
	std::string getText() {
		return text;
	}
private:
	std::string nodeId;
	std::string serviceId;
	std::string text;
};

sznBeginDefault(DdcnGroupMessage, X) {
	X && T(serviceId) && T(text);
} sznEnd();

vsznDefault(DdcnGroupMessage);


void PacketHeader::insertPacketHeaderLength(QByteArray &packet) {
	PacketHeader header;
	memcpy(&header, packet.data(), sizeof(header));
	header.size = qToBigEndian(packet.size());
	memcpy(packet.data(), &header, sizeof(header));
}

const ariba::ServiceID NetworkInterface::SERVICE_ID = ariba::ServiceID(42);

NetworkInterface::NetworkInterface(QString name,
		const PrivateKey &privateKey) : name(name), privateKey(privateKey),
		discoveryTimer(this) {
	certificate = Certificate::createSelfSigned(privateKey);
	// We use signals/slots to pass data from the Ariba thread to the Qt thread
	connect(this, SIGNAL(aribaMessage(QByteArray, unsigned short, ariba::utility::NodeID, ariba::utility::LinkID)),
		SLOT(onAribaMessage(QByteArray, unsigned short, ariba::utility::NodeID, ariba::utility::LinkID)),
		Qt::QueuedConnection);
	connect(this, SIGNAL(aribaLinkUp(ariba::utility::LinkID, ariba::utility::NodeID)),
		SLOT(onAribaLinkUp(ariba::utility::LinkID, ariba::utility::NodeID)), Qt::QueuedConnection);
	connect(this, SIGNAL(aribaLinkDown(ariba::utility::LinkID, ariba::utility::NodeID)),
		SLOT(onAribaLinkDown(ariba::utility::LinkID, ariba::utility::NodeID)), Qt::QueuedConnection);
	connect(this, SIGNAL(aribaLinkChanged(ariba::utility::LinkID, ariba::utility::NodeID)),
		SLOT(onAribaLinkChanged(ariba::utility::LinkID, ariba::utility::NodeID)), Qt::QueuedConnection);
	connect(this, SIGNAL(aribaLinkFail(ariba::utility::LinkID, ariba::utility::NodeID)),
		SLOT(onAribaLinkFail(ariba::utility::LinkID, ariba::utility::NodeID)), Qt::QueuedConnection);
	// Start networking
	ariba::utility::StartupWrapper::startSystem();
	ariba::utility::StartupWrapper::startup(this, false);
}
NetworkInterface::~NetworkInterface() {
	ariba::utility::StartupWrapper::shutdown(this, false);
	ariba::utility::StartupWrapper::stopSystem();
}

void NetworkInterface::send(NetworkNode *node, const Packet &packet) {
	qDebug("Sending packet.");
	node->sendPacket(packet);
}
void NetworkInterface::send(McpoGroup *group, const Packet &packet) {
	// Send the packet to the ariba thread
	GroupMessage *groupMessage = new GroupMessage;
	groupMessage->nodeId = node->getNodeId();
	groupMessage->serviceId = group->getServiceId();
	groupMessage->packet = packet;
	SystemQueue::instance().scheduleEvent(SystemEvent(this,
		SEND_GROUP_MESSAGE_EVENT, groupMessage));
}
void NetworkInterface::sendToAll(const Packet &packet) {
	// TODO: Use an MCPO broadcast here?
	QMap<QString, NetworkNode*>::Iterator it = onlineNodes.begin();
	while (it != onlineNodes.end()) {
		send(it.value(), packet);
		it++;
	}
}
McpoGroup *NetworkInterface::joinGroup(ariba::ServiceID group) {
	QMap<ariba::ServiceID, McpoGroup*>::Iterator it = mcpoGroups.find(group);
	if (it == mcpoGroups.end()) {
		// Create a new group
		McpoGroup *mcpoGroup = new McpoGroup(group);
		mcpoGroup->grab();
		mcpoGroups.insert(group, mcpoGroup);
		// Actually joining the group has to be done in the ariba thread
		SystemQueue::instance().scheduleEvent(SystemEvent(this,
			JOIN_GROUP_EVENT, new ariba::ServiceID(group)));
		return mcpoGroup;
	} else {
		it.value()->grab();
		return it.value();
	}
}
void NetworkInterface::leaveGroup(McpoGroup *group) {
	ariba::ServiceID serviceId = group->getServiceId();
	if (!group->drop()) {
		SystemQueue::instance().scheduleEvent(SystemEvent(this,
			LEAVE_GROUP_EVENT, new ariba::ServiceID(serviceId)));
	}
}
void NetworkInterface::setName(QString name) {
	// TODO
}

NetworkNode *NetworkInterface::getNetworkNode(const PublicKey &publicKey) {
	// TODO: Too slow
	QMap<QString, NetworkNode*>::Iterator it = onlineNodes.begin();
	while (it != onlineNodes.end()) {
		if (it.value()->getPublicKey() == publicKey) {
			return it.value();
		}
	}
	return NULL;
}

bool NetworkInterface::onLinkRequest(const ariba::utility::NodeID &remote) {
	// TODO: Do we want to do this? probably not.
	if (knownNodes.contains(remote.toString().c_str())) {
		return false;
	}
	knownNodes.insert(remote.toString().c_str());
	return true;
}
void NetworkInterface::onMessage(const ariba::DataMessage &msg,
		const ariba::utility::NodeID &remote, const ariba::utility::LinkID &link) {
	// TODO: This must not be an assert
	assert(msg.isMessage());
	DdcnMessage* ddcnMessage = msg.getMessage()->convert<DdcnMessage>();
	if (ddcnMessage->getData().size() == 0) {
		qCritical("Received empty message.");
		return;
	}
	qDebug("Incoming: %d (serial: %d, checksum: %X)",
		(int)ddcnMessage->getData().size(), ddcnMessage->getSerial(),
		ddcnMessage->getChecksum());
	emit aribaMessage(ddcnMessage->getData(), ddcnMessage->getSerial(), remote, link);
	delete ddcnMessage;
}
void NetworkInterface::onLinkUp(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote) {
	emit aribaLinkUp(link, remote);
}
void NetworkInterface::onLinkDown(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote) {
	qCritical("onLinkDown");
	emit aribaLinkDown(link, remote);
	knownNodes.remove(remote.toString().c_str());
}
void NetworkInterface::onLinkChanged(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote) {
	emit aribaLinkChanged(link, remote);
}
void NetworkInterface::onLinkFail(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote) {
	qCritical("onLinkFail");
	emit aribaLinkFail(link, remote);
	knownNodes.remove(remote.toString().c_str());
}

void NetworkInterface::onJoinCompleted(const ariba::SpoVNetID &vid) {
	// Initialize multicast
	mcpo = new MCPO(this, aribaModule, node);
	// Start timer
	discoveryTimer.setInterval(1000);
	discoveryTimer.start();
}
void NetworkInterface::onJoinFailed(const ariba::SpoVNetID &vid) {
	qFatal("Could not join spovnet.");
}
void NetworkInterface::onLeaveCompleted(const ariba::SpoVNetID &vid) {
	// TODO
}
void NetworkInterface::onLeaveFailed(const ariba::SpoVNetID &vid) {
	// TODO
}

void NetworkInterface::startup() {
	// Initialize ariba module
	aribaModule = new ariba::AribaModule();
	ariba::Name spovnetName("ddcn");
	ariba::Name nodeName(name.toAscii());
	aribaModule->setProperty("endpoints", bootstrapConfig.getEndPoints().toStdString());
	aribaModule->setProperty("bootstrap.hints", bootstrapConfig.getBootstrapHints().toStdString());
	// TODO: Can we still change these settings later?
	aribaModule->start();
	// Create a new node
	node = new ariba::Node(*aribaModule, nodeName);
	// Bind listeners
	if (!node->bind(this)) {
		qFatal("Could not bind node listener.");
	}
	if (!node->bind(this, SERVICE_ID)) {
		qFatal("Could not bind communication listener.");
	}
	// Start node
	node->start();
	// Set properties
	ariba::SpoVNetProperties params;
	// TODO: Do any properties need to be changed?
	// Initiate the spovnet
	node->initiate(spovnetName, params);
	// Join the spovnet
	node->join(spovnetName);
}
void NetworkInterface::shutdown() {
	// Shutdown MCPO
	if (mcpo) {
		delete mcpo;
	}
	// Leave the spovnet
	node->leave();
	// Unbind listeners
	node->unbind(this);
	node->unbind(this, SERVICE_ID);
	// Stop ariba
	aribaModule->stop();
	// Clean up
	delete node;
	delete aribaModule;
}

void NetworkInterface::receiveData(const ariba::DataMessage &msg) {
	emit mcpoReceiveData(msg);
}
void NetworkInterface::serviceIsReady() {
	// TODO
}

void NetworkInterface::handleSystemEvent(const ariba::utility::SystemEvent &event) {
	if (event.getType() == JOIN_GROUP_EVENT) {
		ariba::ServiceID *serviceId = event.getData<ariba::ServiceID>();
		mcpo->joinGroup(*serviceId);
		delete serviceId;
	} else if (event.getType() == LEAVE_GROUP_EVENT) {
		ariba::ServiceID *serviceId = event.getData<ariba::ServiceID>();
		mcpo->leaveGroup(*serviceId);
		delete serviceId;
	} else if (event.getType() == SEND_GROUP_MESSAGE_EVENT) {
		GroupMessage *groupMessage = event.getData<GroupMessage>();
		// TODO: Optimize aray these unnecessary copies
		QByteArray packetData((const char*)groupMessage->packet.getRawData(), groupMessage->packet.getRawSize());
		QByteArray data = packetData.toHex();
		DdcnGroupMessage ddcnMessage(groupMessage->nodeId.toString(),
				groupMessage->serviceId.toString(), data.data());
		// TODO: This function is still unimplemented
		mcpo->sendToGroup(ddcnMessage, groupMessage->serviceId);
		delete groupMessage;
	} else if (event.getType() == SEND_PEER_MESSAGE_EVENT) {
		PeerMessage *peerMessage = event.getData<PeerMessage>();
		DdcnMessage message(peerMessage->serial);
		message.getData() = peerMessage->message;
		qDebug("Outgoing: %d (serial: %d, checksum: %X)",
			peerMessage->message.size(), peerMessage->serial, message.getChecksum());
		node->sendMessage(message, peerMessage->linkId);
		delete peerMessage;
	} else {
		qCritical("NetworkInterface: Unknown event type.");
	}
}

void NetworkInterface::onAribaMessage(const QByteArray &data, unsigned short serial,
		const ariba::utility::NodeID &remote, const ariba::utility::LinkID &link) {
	qCritical("onAribaMessage");
	NetworkNode *networkNode = NULL;
	QMap<QString, NetworkNode*>::Iterator it = onlineNodes.find(remote.toString().c_str());
	if (it != onlineNodes.end()) {
		networkNode = it.value();
	} else {
		it = pendingNodes.find(remote.toString().c_str());
		if (it != pendingNodes.end()) {
			networkNode = it.value();
		}
	}
	if (!networkNode) {
		qWarning("Message from unknown node.");
		return;
	}
	unsigned short expectedSerial = networkNode->getNextExpectedSerial();
	if (serial != expectedSerial) {
		qCritical("Serial mismatch: %d instead of %d", serial, expectedSerial);
	}
	networkNode->getTLS().writeIncoming(data);
}
void NetworkInterface::onAribaLinkUp(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote) {
	qCritical("onAribaLinkUp");
	NetworkNode *networkNode = new NetworkNode(remote, link);
	connect(networkNode, SIGNAL(outgoingDataAvailable(NetworkNode*)),
		this, SLOT(onNodeOutgoingDataAvailable(NetworkNode*)));
	connect(networkNode, SIGNAL(packetReceived(NetworkNode*, Packet)),
		this, SLOT(onNodePacketReceived(NetworkNode*, Packet)));
	connect(networkNode, SIGNAL(connectionReady(NetworkNode*)),
		this, SLOT(onNodeConnectionReady(NetworkNode*)));
	TLS *tls = &networkNode->getTLS();
	tls->setPrivateKey(privateKey);
	tls->setCertificate(certificate);
	if (remote > node->getNodeId()) {
		tls->startServer();
	} else {
		tls->startClient();
	}
	// Add the node to the pending nodes, we have to wait for encryption
	pendingNodes.insert(remote.toString().c_str(), networkNode);
}
void NetworkInterface::onAribaLinkDown(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote) {
	qCritical("onAribaLinkDown");
	// Find NetworkNode for this node id
	QMap<QString, NetworkNode*>::Iterator it = onlineNodes.find(remote.toString().c_str());
	if (it != onlineNodes.end()) {
		emit peerDisconnected(it.value());
		delete it.value();
		onlineNodes.erase(it);
		return;
	}
	it = pendingNodes.find(remote.toString().c_str());
	if (it != pendingNodes.end()) {
		delete it.value();
		pendingNodes.erase(it);
		return;
	}
}
void NetworkInterface::onAribaLinkChanged(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote) {
	// TODO
}
void NetworkInterface::onAribaLinkFail(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote) {
	// TODO
	onAribaLinkDown(link, remote);
}
void NetworkInterface::onMcpoReceiveData(const ariba::DataMessage &msg) {
	qCritical("Group message incoming.");
	DdcnGroupMessage* ddcnMessage = msg.getMessage()->convert<DdcnGroupMessage>();
	ariba::ServiceID groupId(std::atoi(ddcnMessage->getServiceId().c_str()));
	QByteArray packetData = QByteArray::fromHex(ddcnMessage->getText().c_str());
	// TODO: Once we get correct binary data instead of a string, we can pass a
	// char* in here instead of QByteArray
	Packet packet = Packet::fromRawData(packetData);
	if (!packet.isValid()) {
		qWarning("Received invalid group message.");
		return;
	}
	// Find the sender
	QMap<QString, NetworkNode*>::Iterator it = onlineNodes.find(ddcnMessage->getNodeId().c_str());
	if (it == onlineNodes.end()) {
		return;
	}
	// Find the group
	QMap<ariba::ServiceID, McpoGroup*>::Iterator it2 = mcpoGroups.find(groupId);
	if (it2 == mcpoGroups.end()) {
		return;
	}
	emit groupMessageReceived(it2.value(), it.value(), packet);
}

void NetworkInterface::onNodeOutgoingDataAvailable(NetworkNode *node) {
	QByteArray outgoingData = node->getTLS().readOutgoing();
	qCritical("onNodeOutgoingDataAvailable: %d", (int)outgoingData.size());
	// Inject data into the ariba thread
	PeerMessage *peerMessage = new PeerMessage;
	// TODO: This might be a security risk? We probably should encrypt the packet
	peerMessage->nodeId = node->aribaNode;
	peerMessage->linkId = node->aribaLink;
	peerMessage->message = outgoingData;
	peerMessage->serial = node->getNextOutgoingSerial();
	SystemQueue::instance().scheduleEvent(SystemEvent(this,
		SEND_PEER_MESSAGE_EVENT, peerMessage));
}
void NetworkInterface::onNodePacketReceived(NetworkNode *node, const Packet &packet) {
	emit messageReceived(node, packet);
}
void NetworkInterface::onNodeConnectionReady(NetworkNode *node) {
	QMap<QString, NetworkNode*>::Iterator it = pendingNodes.find(node->aribaNode.toString().c_str());
	pendingNodes.erase(it);
	onlineNodes.insert(node->aribaNode.toString().c_str(), node);
	// TODO: Get the name of the peer
	emit peerConnected(node, "", node->getPublicKey());
}

void NetworkInterface::peerDiscovery() {
	qCritical("Checking for new nodes...");
	std::vector<ariba::utility::NodeID> nodes = node->getNeighborNodes();
	knownNodesMutex.lock();
	//qCritical("Known nodes before: %d", knownNodes.size());
	BOOST_FOREACH(ariba::utility::NodeID nodeId, nodes) {
		if (knownNodes.contains(nodeId.toString().c_str())) {
			//qCritical("Node already known: %s.", nodeId.toString().c_str());
			continue;
		}
		if (nodeId == node->getNodeId()) {
			qCritical("Duplicate node ids.");
			continue;
		}
		if (nodeId >= node->getNodeId()) {
			continue;
		}
		qCritical("New neighbor %s", nodeId.toString().c_str());
		// Only create the link once
		knownNodes.insert(nodeId.toString().c_str());
		// Only one side of the connection shall create a link
		// Establish link to the new node
		node->establishLink(nodeId, SERVICE_ID);
		assert(knownNodes.contains(nodeId.toString().c_str()));
	}
	//qCritical("Known nodes after: %d", knownNodes.size());
	knownNodesMutex.unlock();
}

void NetworkInterface::PeerDiscoveryTimer::eventFunction() {
	network->peerDiscovery();
}

const ariba::utility::SystemEventType NetworkInterface::JOIN_GROUP_EVENT("JoinGroup");
const ariba::utility::SystemEventType NetworkInterface::LEAVE_GROUP_EVENT("LeaveGroup");
const ariba::utility::SystemEventType NetworkInterface::SEND_GROUP_MESSAGE_EVENT("SendGroupMessage");
const ariba::utility::SystemEventType NetworkInterface::SEND_PEER_MESSAGE_EVENT("SendPeerMessage");
