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
#include "NetworkNode.h"

#include <QObject>

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

	void setKeys(QString publicKey, QString privateKey);
	void generateKeys();
	QString getPublicKey();
	QString getPrivateKey();

	void addTrustedPeer(QString name, QString publicKey);
	void removeTrustedPeer(QString name, QString publicKey);
	QList<TrustedPeer*> getTrustedPeers() {
		return trustedPeers;
	}

	void addTrustedGroup(QString name, QString publicKey);
	void removeTrustedGroup(QString name, QString publicKey);
	QList<TrustedGroup*> getTrustedGroups() {
		return trustedGroups;
	}

	void addGroupMembership(QString name, QString publicKey, QString privateKey);
	void removeGroupMembership(QString name, QString publicKey);
	QList<GroupMembership*> getGroupMemberships() {
		return groupMemberships;
	}

	bool canAcceptOutgoingJobRequest();
	bool delegateOutgoingJob(Job *job);
signals:
	void peerNameChanged(QString peerName);
	void encryptionChanged(bool encryptionEnabled);
	void publicKeyChanged(QString publicKey);
	void privateKeyChanged(QString privateKey);
	void trustedPeersChanged(QList<TrustedPeer*> trustedPeers);
	void trustedGroupsChanged(QList<TrustedGroup*> trustedGroups);
	void groupMembershipsChanged(QList<GroupMembership*> groupMemberships);
private:
	QString peerName;
	bool encryptionEnabled;
	QString publicKey;
	QString privateKey;

	// TODO: Do we need much lookups here? A hash map then would be faster.
	// We could need public-key based lookups a lot.
	QList<TrustedPeer*> trustedPeers;
	QList<TrustedGroup*> trustedGroups;
	QList<GroupMembership*> groupMemberships;

	NetworkNode *localNode;
};

#endif

