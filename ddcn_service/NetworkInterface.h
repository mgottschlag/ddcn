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

#ifndef NETWORKINTERFACE_H_INCLUDED
#define NETWORKINTERFACE_H_INCLUDED

#include "McpoGroup.h"
#include "NetworkNode.h"
#include "BootstrapConfig.h"
#include "Protocol.h"

#include <ariba/ariba.h>
#include <ariba/utility/system/StartupInterface.h>
#include <mcpo/MCPO.h>
#include <QMutex>
#include <QSet>

uint qHash(ariba::utility::NodeID nodeId);

using ariba::services::mcpo::MCPO;

Q_DECLARE_METATYPE(ariba::utility::NodeID);
Q_DECLARE_METATYPE(ariba::utility::LinkID);
Q_DECLARE_METATYPE(ariba::DataMessage);

/**
 * This class initializes Ariba and MCPO and does the communication between the
 * main thread and the Ariba system queue thread.
 */
class NetworkInterface : public QObject,
		public ariba::utility::StartupInterface,
		public ariba::NodeListener,
		public ariba::CommunicationListener,
		public MCPO::ReceiverInterface,
		public ariba::utility::SystemEventListener {
	Q_OBJECT
public:
	NetworkInterface(QString name, const PrivateKey &privateKey);
	~NetworkInterface();

	void send(NetworkNode *node, const Packet &packet);
	void send(McpoGroup *group, const Packet &packet);
	void sendToAll(const Packet &packet);
	McpoGroup *joinGroup(ariba::ServiceID group);
	void leaveGroup(McpoGroup *group);
	void setName(QString name);

	NetworkNode *getNetworkNode(const PublicKey &publicKey);
signals:
	void peerConnected(NetworkNode *node, QString name,
		const PublicKey &publicKey);
	void peerDisconnected(NetworkNode *node);
	void peerChanged(NetworkNode *node, QString name);
	void messageReceived(NetworkNode *node, const Packet &packet);
	void groupMessageReceived(McpoGroup *group, NetworkNode *node,
		const Packet &packet);
protected:
	// Communication listener interface
	virtual bool onLinkRequest(const ariba::utility::NodeID &remote);
	virtual void onMessage(const ariba::DataMessage &msg, const ariba::utility::NodeID &remote,
		const ariba::utility::LinkID &link = ariba::utility::LinkID::UNSPECIFIED);
	virtual void onLinkUp(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote);
	virtual void onLinkDown(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote);
	virtual void onLinkChanged(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote);
	virtual void onLinkFail(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote);

	// Node listener interface
	virtual void onJoinCompleted(const ariba::SpoVNetID &vid);
	virtual void onJoinFailed(const ariba::SpoVNetID &vid);
	virtual void onLeaveCompleted(const ariba::SpoVNetID &vid);
	virtual void onLeaveFailed(const ariba::SpoVNetID &vid);

	// Startup wrapper interface
	virtual void startup();
	virtual void shutdown();

	// MCPO receiver interface
	virtual void receiveData(const ariba::DataMessage &msg);
	virtual void serviceIsReady();

	// System event listener interface
	virtual void handleSystemEvent(const ariba::utility::SystemEvent &event);
signals:
	void aribaMessage(const ariba::DataMessage &msg, const ariba::utility::NodeID &remote,
		const ariba::utility::LinkID &link);
	void aribaLinkUp(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote);
	void aribaLinkDown(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote);
	void aribaLinkChanged(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote);
	void aribaLinkFail(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote);
	void mcpoReceiveData(const ariba::DataMessage &msg);
private slots:
	void onAribaMessage(const ariba::DataMessage &msg, const ariba::utility::NodeID &remote,
		const ariba::utility::LinkID &link);
	void onAribaLinkUp(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote);
	void onAribaLinkDown(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote);
	void onAribaLinkChanged(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote);
	void onAribaLinkFail(const ariba::utility::LinkID &link, const ariba::utility::NodeID &remote);
	void onMcpoReceiveData(const ariba::DataMessage &msg);

	void onNodeOutgoingDataAvailable(NetworkNode *node);
	void onNodePacketReceived(NetworkNode *node, const Packet &packet);
	void onNodeConnectionReady(NetworkNode *node);
private:
	void peerDiscovery();

	ariba::AribaModule *aribaModule;
	ariba::Node *node;

	MCPO *mcpo;

	QString name;
	PrivateKey privateKey;
	Certificate certificate;

	QMap<ariba::ServiceID, McpoGroup*> mcpoGroups;
	// TODO: Fix this!
	//QMap<ariba::utility::NodeID, NetworkNode*> onlineNodes;
	//QMap<ariba::utility::NodeID, NetworkNode*> pendingNodes;
	QMap<QString, NetworkNode*> onlineNodes;
	QMap<QString, NetworkNode*> pendingNodes;

	QMutex knownNodesMutex;
	// TODO: Why doesn't this work?'
	//QSet<ariba::utility::NodeID> knownNodes;
	QSet<QString> knownNodes;

	static const ariba::ServiceID SERVICE_ID;

	BootstrapConfig bootstrapConfig;

	class PeerDiscoveryTimer : public ariba::utility::Timer {
	public:
		PeerDiscoveryTimer(NetworkInterface *network) : network(network) {
		}

		virtual void eventFunction();
	private:
		NetworkInterface *network;
	};

	PeerDiscoveryTimer discoveryTimer;

	static const ariba::utility::SystemEventType JOIN_GROUP_EVENT;
	static const ariba::utility::SystemEventType LEAVE_GROUP_EVENT;
	static const ariba::utility::SystemEventType SEND_GROUP_MESSAGE_EVENT;
	static const ariba::utility::SystemEventType SEND_PEER_MESSAGE_EVENT;
};

#endif
