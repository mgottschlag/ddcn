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

#ifndef COMPILERNETWORK_H_INCLUDED
#define COMPILERNETWORK_H_INCLUDED

#include "TrustedPeer.h"
#include "TrustedGroup.h"
#include "GroupMembership.h"
#include "Job.h"
#include "NetworkInterface.h"

#include <QObject>
#include "JobRequest.h"

/**
 * Class which communicates with peers in the compiler network and receives
 * and sends compiler jobs.
 */
class CompilerNetwork : public QObject {
	Q_OBJECT
	Q_PROPERTY(QString peerName
	           READ getPeerName
	           WRITE setPeerName
	           NOTIFY peerNameChanged)
	Q_PROPERTY(bool encryption
	           READ getEncryption
	           WRITE setEncryption
	           NOTIFY encryptionChanged)
public:
	CompilerNetwork();
	~CompilerNetwork();

	void setPeerName(QString peerName);
	QString getPeerName();

	void setEncryption(bool encryptionEnabled);
	bool getEncryption();

	void setKeys(QCA::PublicKey publicKey, QCA::PrivateKey privateKey);
	void generateKeys();
	QCA::PublicKey getPublicKey();
	QCA::PrivateKey getPrivateKey();

	void addTrustedPeer(QString name, const QCA::PublicKey &publicKey);
	void removeTrustedPeer(QString name, const QCA::PublicKey &publicKey);
	QList<TrustedPeer*> getTrustedPeers() {
		return trustedPeers;
	}

	void addTrustedGroup(QString name, const QCA::PublicKey &publicKey);
	void removeTrustedGroup(QString name, const QCA::PublicKey &publicKey);
	QList<TrustedGroup*> getTrustedGroups() {
		return trustedGroups;
	}

	void addGroupMembership(QString name, const QCA::PublicKey &publicKey, const QCA::PrivateKey &privateKey);
	void removeGroupMembership(QString name, const QCA::PublicKey &publicKey);
	QList<GroupMembership*> getGroupMemberships() {
		return groupMemberships;
	}

	bool canAcceptOutgoingJobRequest();
	void rejectIncomingJobReqiest(JobRequest *request);
	void acceptIncomingJobRequest(JobRequest *request);
	bool delegateOutgoingJob(Job *job);
private slots:
	void onPeerConnected(NetworkNode *node, QString name,
		const QCA::PublicKey &publicKey);
	void onPeerDisconnected(NetworkNode *node);
	void onPeerChanged(NetworkNode *node, QString name);
	void onMessageReceived(NetworkNode *node, const QByteArray &message);
	void onGroupMessageReceived(McpoGroup *group, NetworkNode *node,
		const QByteArray &message);
signals:
	void peerNameChanged(QString peerName);
	void encryptionChanged(bool encryptionEnabled);
	void publicKeyChanged(const QCA::PublicKey &publicKey);
	void privateKeyChanged(const QCA::PrivateKey &privateKey);
	void trustedPeersChanged(QList<TrustedPeer*> trustedPeers);
	void trustedGroupsChanged(QList<TrustedGroup*> trustedGroups);
	void groupMembershipsChanged(QList<GroupMembership*> groupMemberships);
	void receivedJob(Job *job);
	void receivedJobRequest(JobRequest *request);
	void finishedJob(Job *job, bool executed, bool success);
	void spareResources();
	void outgoingRequestAccepted(JobRequest *request);
private:
	QString peerName;
	bool encryptionEnabled;
	QCA::PublicKey publicKey;
	QCA::PrivateKey privateKey;

	// TODO: Do we need much lookups here? A hash map then would be faster.
	// We could need public-key based lookups a lot.
	QList<TrustedPeer*> trustedPeers;
	QList<TrustedGroup*> trustedGroups;
	QList<GroupMembership*> groupMemberships;

	NetworkInterface *network;
};

#endif
