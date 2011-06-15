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

#include <ariba/ariba.h>
#include <ariba/utility/system/StartupInterface.h>
#include <mcpo/MCPO.h>
#include <QtCrypto>

using ariba::services::mcpo::MCPO;

/**
 * This class initializes Ariba and MCPO and does the communication between the
 * main thread and the Ariba system queue thread.
 */
class NetworkInterface : public QObject,
		public ariba::utility::StartupInterface,
		public ariba::NodeListener,
		public ariba::CommunicationListener,
		public MCPO::ReceiverInterface,
		public ariba::utility::Timer {
	Q_OBJECT
public:
	NetworkInterface(QString name, const QCA::PrivateKey &privateKey);
	~NetworkInterface();

	void send(NetworkNode *node, const QByteArray &message);
	void send(McpoGroup *group, const QByteArray &message);
	McpoGroup *joinGroup(ariba::ServiceID group);
	void leaveGroup(McpoGroup *group);
	void setName(QString name);

	NetworkNode *getNetworkNode(const QCA::PublicKey &publicKey);
signals:
	void peerConnected(NetworkNode *node, QString name,
		const QCA::PublicKey &publicKey);
	void peerDisconnected(NetworkNode *node);
	void peerChanged(NetworkNode *node, QString name);
	void messageReceived(NetworkNode *node, const QByteArray &message);
	void groupMessageReceived(McpoGroup *group, NetworkNode *node,
		const QByteArray &message);
protected:
	// Communication listener interface
	virtual bool onLinkRequest(const NodeID &remote);
	virtual void onMessage(const ariba::DataMessage &msg, const NodeID &remote,
		const LinkID &link = LinkID::UNSPECIFIED);
	virtual void onLinkUp(const LinkID &link, const NodeID &remote);
	virtual void onLinkDown(const LinkID &link, const NodeID &remote);
	virtual void onLinkChanged(const LinkID &link, const NodeID &remote);
	virtual void onLinkFail(const LinkID &link, const NodeID &remote);

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

	// Timer interface
	virtual void eventFunction();
private:
	ariba::AribaModule *aribaModule;
	ariba::Node *node;

	MCPO *mcpo;

	QString name;

	QMap<ariba::ServiceID, McpoGroup*> mcpoGroups;
	QMap<NodeID, NetworkNode*> onlineNodes;

	QMap<NodeID, NetworkNode*> pendingNodes;

	static const ariba::ServiceID SERVICE_ID;

	BootstrapConfig bootstrapConfig;
};

#endif
