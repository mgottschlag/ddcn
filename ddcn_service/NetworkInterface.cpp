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

const ariba::ServiceID NetworkInterface::SERVICE_ID = ariba::ServiceID(42);

NetworkInterface::NetworkInterface(QString name,
		const QCA::PrivateKey &privateKey) {
	// Load networking settings
	// TODO
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
	// TODO
	return NULL;
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

bool NetworkInterface::onLinkRequest(const NodeID &remote) {
	std::cout << "Link request." << std::endl;
	// TODO
	return true;
}
void NetworkInterface::onMessage(const ariba::DataMessage &msg, const NodeID &remote,
		const LinkID &link) {
	std::cout << "Message received." << std::endl;
	// TODO
}
void NetworkInterface::onLinkUp(const LinkID &link, const NodeID &remote) {
	std::cout << "Link up." << std::endl;
	// Send the other node our name and public key
	// TODO
}
void NetworkInterface::onLinkDown(const LinkID &link, const NodeID &remote) {
	std::cout << "Link down." << std::endl;
	// TODO
}
void NetworkInterface::onLinkChanged(const LinkID &link, const NodeID &remote) {
	std::cout << "Link changed." << std::endl;
	// TODO
}
void NetworkInterface::onLinkFail(const LinkID &link, const NodeID &remote) {
	std::cout << "Link fail." << std::endl;
	// TODO
}

void NetworkInterface::onJoinCompleted(const ariba::SpoVNetID &vid) {
	// Initialize multicast
	mcpo = new MCPO(this, aribaModule, node);
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
	ariba::Name nodeName = ariba::Name::UNSPECIFIED;
	aribaModule->start();
	// Create a new node
	node = new ariba::Node(*aribaModule, nodeName);
	// TODO: Initialize bootstrapping and names
	// Bind listeners
	node->bind(this);
	node->bind(this, SERVICE_ID);
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
