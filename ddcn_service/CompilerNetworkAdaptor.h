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

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COMPILERNETWORKADAPTOR_H_INCLUDED
#define COMPILERNETWORKADAPTOR_H_INCLUDED

#include "CompilerNetwork.h"
#include "DBusStructs.h"

#include <QDBusAbstractAdaptor>

/**
 * Class which exports a CompilerNetwork object.
 */
class CompilerNetworkAdaptor : public QDBusAbstractAdaptor {
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.ddcn.CompilerNetwork")
	Q_PROPERTY(QString peerName
	           READ getPeerName
	           WRITE setPeerName
	           NOTIFY peerNameChanged)
	Q_PROPERTY(bool encryption
	           READ getEncryption
	           WRITE setEncryption
	           NOTIFY encryptionChanged)
public:
	/**
	 * Creates a dbus adaptor for the compiler network.
	 * @param network Compiler network object to export.
	 */
	explicit CompilerNetworkAdaptor(CompilerNetwork *network);
public slots:
	void setPeerName(QString peerName);
	QString getPeerName();

	void setEncryption(bool encryptionEnabled);
	bool getEncryption();

	void setCompression(bool compressionEnabled);
	bool getCompression();

	void setLocalKey(QString privateKey);
	void generateLocalKey(int keyLength = 2048);
	QString getLocalKey();

	void addTrustedPeer(QString name, QString publicKey);
	void removeTrustedPeer(QString name, QString publicKey);
	QList<TrustedPeerInfo> getTrustedPeers();

	void addTrustedGroup(QString name, QString publicKey);
	void removeTrustedGroup(QString name, QString publicKey);
	QList<TrustedGroupInfo> getTrustedGroups();

	void addGroupMembership(QString name, QString privateKey);
	void removeGroupMembership(QString name, QString publicKey);
	QList<GroupMembershipInfo> getGroupMemberships();

	void queryNetworkStatus();
private slots:
	// These slots just emit the corresponding signals of this class, they are
	// used to forward the signals of the encapsulated class to dbus
	void onPeerNameChanged(QString peerName);
	void onEncryptionChanged(bool encryptionEnabled);
	void onCompressionChanged(bool compressionEnabled);
	void onLocalKeyChanged(const PrivateKey &privateKey);
	void onTrustedPeersChanged(const QList<TrustedPeer*> &trustedPeers);
	void onTrustedGroupsChanged(const QList<TrustedGroup*> &trustedGroups);
	void onGroupMembershipsChanged(const QList<GroupMembership*> &groupMemberships);
	void onNodeStatusChanged(QString publicKey, QString fingerPrint, NodeStatus nodeStatus, QStringList groups);
signals:
	void peerNameChanged(QString peerName);
	void encryptionChanged(bool encryptionEnabled);
	void compressionChanged(bool compressionEnabled);
	void publicKeyChanged(QString publicKey);
	void trustedPeersChanged(const QList<TrustedPeerInfo> &trustedPeers);
	void trustedGroupsChanged(const QList<TrustedGroupInfo> &trustedGroups);
	void groupMembershipsChanged(const QList<GroupMembershipInfo> &groupMemberships);
	void nodeStatusChanged(QString publicKey, QString fingerPrint, NodeStatus nodeStatus, QStringList groups);
private:
	static TrustedPeerInfo toTrustedPeerInfo(TrustedPeer *trustedPeer);
	static TrustedGroupInfo toTrustedGroupInfo(TrustedGroup *trustedGroup);
	static GroupMembershipInfo toGroupMembershipInfo(GroupMembership *groupMembership);
	static QList<TrustedPeerInfo> toTrustedPeerInfo(const QList<TrustedPeer*> &trustedPeers);
	static QList<TrustedGroupInfo> toTrustedGroupInfo(const QList<TrustedGroup*> &trustedGroups);
	static QList<GroupMembershipInfo> toGroupMembershipInfo(const QList<GroupMembership*> &groupMemberships);

	CompilerNetwork *network;
};

#endif

