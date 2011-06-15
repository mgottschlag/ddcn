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

uint qHash(ariba::utility::NodeID nodeId) {
	return qHash(nodeId.toString().c_str());
}

const ariba::ServiceID NetworkInterface::SERVICE_ID = ariba::ServiceID(42);

NetworkInterface::NetworkInterface(QString name,
		const QCA::PrivateKey &privateKey) : name(name), discoveryTimer(this) {
	// We use signals/slots to pass data from the Ariba thread to the Qt thread
	connect(this, SIGNAL(aribaMessage(ariba::DataMessage, ariba::utility::NodeID, ariba::utility::LinkID)),
		SLOT(onAribaMessage(ariba::DataMessage, ariba::utility::NodeID, ariba::utility::LinkID)), Qt::QueuedConnection);
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

void NetworkInterface::send(NetworkNode *node, const QByteArray &message) {
	// TODO
}
void NetworkInterface::send(McpoGroup *group, const QByteArray &message) {
	// TODO
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
	// TODO
}
void NetworkInterface::setName(QString name) {
	// TODO
}

NetworkNode *NetworkInterface::getNetworkNode(const QCA::PublicKey &publicKey) {
	// TODO
	return NULL;
}

bool NetworkInterface::onLinkRequest(const ariba::utility::NodeID &remote) {
	return true;
}
void NetworkInterface::onMessage(const ariba::DataMessage &msg,
		const ariba::utility::NodeID &remote, const ariba::utility::LinkID &link) {
	std::cout << "Message received." << std::endl;
	// TODO
}
void NetworkInterface::onLinkUp(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote) {
	emit aribaLinkUp(link, remote);
}
void NetworkInterface::onLinkDown(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote) {
	emit aribaLinkDown(link, remote);
	knownNodes.remove(remote);
}
void NetworkInterface::onLinkChanged(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote) {
	emit aribaLinkChanged(link, remote);
	std::cout << "Link changed." << std::endl;
}
void NetworkInterface::onLinkFail(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote) {
	emit aribaLinkFail(link, remote);
	knownNodes.remove(remote);
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
	// TODO: Initialize bootstrapping and names
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
	// TODO
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
	} else {
		qCritical("NetworkInterface: Unknown event type.");
	}
}

void NetworkInterface::onAribaMessage(const ariba::DataMessage &msg,
		const ariba::utility::NodeID &remote, const ariba::utility::LinkID &link) {
	// TODO
}
void NetworkInterface::onAribaLinkUp(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote) {
	// TODO
}
void NetworkInterface::onAribaLinkDown(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote) {
	// TODO
}
void NetworkInterface::onAribaLinkChanged(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote) {
	// TODO
}
void NetworkInterface::onAribaLinkFail(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote) {
	// TODO
}
void NetworkInterface::onMcpoReceiveData(const ariba::DataMessage &msg) {
	// TODO
}

void NetworkInterface::peerDiscovery() {
	std::cout << "Checking for new nodes..." << std::endl;
	std::vector<ariba::utility::NodeID> nodes = node->getNeighborNodes();
	knownNodesMutex.lock();
	BOOST_FOREACH(ariba::utility::NodeID nodeId, nodes) {
		if (knownNodes.contains(nodeId)) {
			continue;
		}
		std::cout << "New neighbor " << nodeId.toString() << std::endl;
		if (nodeId == node->getNodeId()) {
			qCritical("Duplicate node ids.");
			continue;
		}
		// Only create the link once
		knownNodes.insert(nodeId);
		// Only one side of the connection shall create a link
		if (nodeId > node->getNodeId()) {
			continue;
		}
		// Establish link to the new node
		node->establishLink(nodeId, SERVICE_ID);
	}
	knownNodesMutex.unlock();
}

void NetworkInterface::PeerDiscoveryTimer::eventFunction() {
	network->peerDiscovery();
}

const ariba::utility::SystemEventType NetworkInterface::JOIN_GROUP_EVENT("JoinGroup");
const ariba::utility::SystemEventType NetworkInterface::LEAVE_GROUP_EVENT("LeaveGroup");
