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

#include <ariba/utility/system/StartupWrapper.h>

const ariba::ServiceID NetworkNode::SERVICE_ID = ariba::ServiceID(42);

NetworkNode::NetworkNode() {
	// Load networking settings
	// TODO
	// Start networking
	ariba::utility::StartupWrapper::startSystem();
	ariba::utility::StartupWrapper::startup(this, false);
}
NetworkNode::~NetworkNode() {
	ariba::utility::StartupWrapper::shutdown(this, false);
	ariba::utility::StartupWrapper::stopSystem();
}

bool NetworkNode::onLinkRequest(const NodeID &remote) {
	std::cout << "Link request." << std::endl;
	// TODO
	return true;
}
void NetworkNode::onMessage(const ariba::DataMessage &msg, const NodeID &remote,
		const LinkID &link) {
	std::cout << "Message received." << std::endl;
	// TODO
}
void NetworkNode::onLinkUp(const LinkID &link, const NodeID &remote) {
	std::cout << "Link up." << std::endl;
	// TODO
}
void NetworkNode::onLinkDown(const LinkID &link, const NodeID &remote) {
	std::cout << "Link down." << std::endl;
	// TODO
}
void NetworkNode::onLinkChanged(const LinkID &link, const NodeID &remote) {
	std::cout << "Link changed." << std::endl;
	// TODO
}
void NetworkNode::onLinkFail(const LinkID &link, const NodeID &remote) {
	std::cout << "Link fail." << std::endl;
	// TODO
}

void NetworkNode::onJoinCompleted(const ariba::SpoVNetID &vid) {
	// TODO
}
void NetworkNode::onJoinFailed(const ariba::SpoVNetID &vid) {
	// TODO
}
void NetworkNode::onLeaveCompleted(const ariba::SpoVNetID &vid) {
	// TODO
}
void NetworkNode::onLeaveFailed(const ariba::SpoVNetID &vid) {
	// TODO
}

void NetworkNode::startup() {
	// Initialize ariba module
	aribaModule = new ariba::AribaModule();
	ariba::Name spovnetName("ddcn");
	ariba::Name nodeName = ariba::Name::UNSPECIFIED;
	aribaModule->start();
	// Create a new node
	node = new ariba::Node(*aribaModule, nodeName);
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
	// Initialize multicast
	mcpo = new MCPO(this, aribaModule, node);
	// TODO
	// Publish presence
	// TODO
}
void NetworkNode::shutdown() {
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

void NetworkNode::receiveData(const ariba::DataMessage &msg) {
	// TODO
}
void NetworkNode::serviceIsReady() {
	// TODO
}
