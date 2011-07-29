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

#include "MainWindow.h"

#include "SettingsDialog.h"
#include "../ddcn_crypto/PublicKey.h"

#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>
#include <QProcess>
#include <QDBusInterface>
#include <QFileDialog>
#include <QInputDialog>
#include <QDBusReply>
#include <cassert>

MainWindow::MainWindow() : currentThreads(0), maxThreads(0),
	dbusNetwork("org.ddcn.service", "/CompilerNetwork", "org.ddcn.CompilerNetwork"),
	dbusService("org.ddcn.service", "/CompilerService", "org.ddcn.CompilerService")
{
	ui.setupUi(this);
	serviceActive = false;
	emit serviceStatusChanged(false);
	connect(&serviceStatusTimer, SIGNAL(timeout()), SLOT(pollServiceStatus()));
	connect(&serviceTimeoutTimer, SIGNAL(timeout()), SLOT(serviceStartTimeout()));
	connect(this, SIGNAL(serviceStatusChanged(bool)), SLOT(updateStatusText()));
	// Setup status bar
	statusBar()->addWidget(&statusLabel);
	updateStatusText();
	// Periodically poll whether the service is active
	// TODO: This is ugly and not that good for your battery
	serviceStatusTimer.setInterval(1000);
	serviceStatusTimer.setSingleShot(false);
	serviceStatusTimer.start();
	// Set list models
	ui.onlinePeerList->setModel(&onlinePeerModel);
	ui.onlinePeerList->setColumnWidth(0, 22);
	ui.onlinePeerList->setColumnWidth(1, 150);
	ui.onlinePeerList->setColumnWidth(2, 200);
	ui.onlinePeerList->setColumnWidth(3, 40);
	ui.onlinePeerList->setUniformRowHeights(true);
	ui.onlinePeerList->setItemDelegate(&onlinePeerItemDelegate);
	ui.onlineGroupList->setModel(&onlineGroupModel);
	ui.onlineGroupList->setColumnWidth(0, 22);
	ui.onlineGroupList->setColumnWidth(1, 150);
	ui.onlineGroupList->setColumnWidth(2, 200);
	ui.onlineGroupList->setColumnWidth(3, 80);
	ui.onlineGroupList->setColumnWidth(4, 40);
	ui.onlineGroupList->setUniformRowHeights(true);
	// We can already connect the signals here, even if the service is not yet
	// running
	QDBusConnection::sessionBus().connect("org.ddcn.service", "/CompilerNetwork",
			"org.ddcn.CompilerNetwork", "nodeStatusChanged", this,
			SLOT(onNodeStatusChanged(QString, QString, NodeStatus, QStringList)));
	QDBusConnection::sessionBus().connect("org.ddcn.service", "/CompilerService",
			"org.ddcn.CompilerService", "currentThreadCountChanged", this,
			SLOT(onCurrentThreadCountChanged(int)));
	QDBusConnection::sessionBus().connect("org.ddcn.service", "/CompilerService",
			"org.ddcn.CompilerService", "maxThreadCountChanged", this,
			SLOT(onMaxThreadCountChanged(int)));
	QDBusConnection::sessionBus().connect("org.ddcn.service", "/CompilerService",
			"org.ddcn.CompilerService", "numberOfJobsInLocalQueueChanged", this,
			SLOT(onNumberOfLocalJobsChanged(int)));
	QDBusConnection::sessionBus().connect("org.ddcn.service", "/CompilerService",
			"org.ddcn.CompilerService", "numberOfJobsInRemoteQueueChanged", this,
			SLOT(onNumberOfRemoteJobsChanged(int)));
	QDBusConnection::sessionBus().connect("org.ddcn.service", "/CompilerService",
			"org.ddcn.CompilerService", "toolChainsChanged", this,
			SLOT(onToolChainsChanged(QList<ToolChainInfo>)));
	QDBusConnection::sessionBus().connect("org.ddcn.service", "/CompilerNetwork",
			"org.ddcn.CompilerNetwork", "trustedPeersChanged", this,
			SLOT(onTrustedPeersChanged(QList<TrustedPeerInfo>)));
	QDBusConnection::sessionBus().connect("org.ddcn.service", "/CompilerNetwork",
			"org.ddcn.CompilerNetwork", "trustedGroupsChanged", this,
			SLOT(onTrustedGroupsChanged(QList<TrustedGroupInfo>)));
	QDBusConnection::sessionBus().connect("org.ddcn.service", "/CompilerNetwork",
			"org.ddcn.CompilerNetwork", "groupMembershipsChanged", this,
			SLOT(onGroupMembershipsChanged(QList<GroupMembershipInfo>)));

	if(dbusService.isValid()) {
		QDBusReply<QList<ToolChainInfo> > reply = dbusService.call("getToolChains");
		updateToolChainList(reply.value());
	}
}

void MainWindow::startService() {
	if (serviceActive) {
		return;
	}
	// Start process if not already started
	if (!QProcess::startDetached(QCoreApplication::applicationDirPath() + "/ddcn_service",
			QStringList(), QCoreApplication::applicationDirPath())) {
		QMessageBox::critical(this, "Error!", "Could not start service process.");
		return;
	}
	statusLabel.setText("Service starting...");
	// Wait for the process to start up
	serviceTimeoutTimer.setInterval(5000);
	serviceTimeoutTimer.setSingleShot(true);
	serviceTimeoutTimer.start();
}
void MainWindow::stopService() {
	if (!serviceActive) {
		return;
	}
	dbusService.call("shutdown");
	// TODO
}
void MainWindow::showSettings() {
	if (!serviceActive) {
		return;
	}
	SettingsDialog settings;
	int result = settings.exec();
	if (result == QDialog::Accepted) {
		// Set peer name
		if (settings.isPeerNameChanged()) {
			// TODO
		}
		// Set number of active threads
		if (settings.isThreadCountChanged()) {
			// TODO
		}
		// Set private key if necessary
		if (settings.isKeyChanged()) {
			// TODO
		}
	}
}
void MainWindow::openHelp() {
	if (!QDesktopServices::openUrl(QUrl("http://code.google.com/p/ddcn/wiki/Documentation"))) {
		QMessageBox::critical(this, "Error!", "Could not open web browser.");
	}
}
void MainWindow::addToolChain() {
	QString fileName = QFileDialog::getOpenFileName(this,
									tr("Add new toolchain"), "/", NULL).replace("gcc", "*").replace("g++", "*");
	if (dbusService.isValid() && fileName != "") {
		QDBusReply<bool> reply = dbusService.call("addToolChain", fileName);
		if (!reply.value()) {
			QMessageBox::critical(this, "Error!", "Could not add the toolchain you have specified."
				"\nPossible reasons are:"
				"\n- the file does not exist."
				"\n- could not execute the compiler (for version check)."
			);
		}
	} else if (!dbusService.isValid()) {
		QMessageBox::critical(this, "Error!", "Connection to Service interrupted.");
	}
}

void MainWindow::removeToolChain() {
	//QMessageBox::critical(this, "Error!", "Not yet implemented.");
	QList<QListWidgetItem*> items = ui.toolChainList->selectedItems();
	if (dbusService.isValid()) {
		if (items.count() > 0) {
			foreach (QListWidgetItem *i, items) {
				QString path = findToolChainPathOfListItem(i);
				dbusService.call("removeToolChain", path);
			}
		} else {
			QMessageBox::critical(this, "Error!", "Please select some items first.");
		}
	} else {
		QMessageBox::critical(this, "Error!", "Connection to Service interrupted.");
	}
}

QString MainWindow::findToolChainPathOfListItem(QListWidgetItem *item) {
	for (int i=0; i < ui.toolChainList->count(); i++) {
		if (ui.toolChainList->item(i) == item) {
			return toolChainPaths.at(i);
		}
	}
	// This should not be reached as this function must be called with a valid item
	assert(false);
	return "";
}


void MainWindow::refreshNetworkStatus() {
	onlinePeerModel.clear();
	if (!dbusNetwork.isValid()) {
		return;
	}
	dbusNetwork.call("queryNetworkStatus");
}

void MainWindow::addTrustedPeer() {
	// Show a dialog to fetch the public key of the group
	QString keyFile = QFileDialog::getOpenFileName(this, tr("Select the PEM public key of the peer"));
	if (keyFile == "") {
		return;
	}
	PublicKey publicKey = PublicKey::load(keyFile);
	if (!publicKey.isValid()) {
		QMessageBox::critical(this, "Error!",
				"The selected file does not contain a valid PEM encoded public key!");
		return;
	}
	// Let the user select a name to identify this group
	bool ok;
	QString peerName = QInputDialog::getText(this, "Name of this peer",
			"Enter the name of this group", QLineEdit::Normal, "unknown-peer", &ok);
	if (!ok) {
		return;
	}
	// Add the peer
	QString pemKey = publicKey.toPEM();
	dbusNetwork.call("addTrustedPeer", peerName, pemKey);
}
void MainWindow::removeTrustedPeer() {
	// TODO
}
void MainWindow::addTrustedGroup() {
	// Show a dialog to fetch the public key of the group
	QString keyFile = QFileDialog::getOpenFileName(this, tr("Select the PEM public key of the group"));
	if (keyFile == "") {
		return;
	}
	PublicKey publicKey = PublicKey::load(keyFile);
	if (!publicKey.isValid()) {
		// It might be a private key, which is common if this is a group which
		// we already are a member in
		publicKey = PrivateKey::load(keyFile);
	}
	if (!publicKey.isValid()) {
		QMessageBox::critical(this, "Error!",
				"The selected file does not contain a valid PEM encoded public key!");
		return;
	}
	// Let the user select a name to identify this group
	bool ok;
	QString groupName = QInputDialog::getText(this, "Name of this group",
			"Enter the name of this group", QLineEdit::Normal, "unknown-group", &ok);
	if (!ok) {
		return;
	}
	// Add the group
	QString pemKey = publicKey.toPEM();
	dbusNetwork.call("addTrustedGroup", groupName, pemKey);
}
void MainWindow::removeTrustedGroup() {
	// TODO
}
void MainWindow::joinGroup() {
	// Show a dialog to fetch the private key of the group
	QString keyFile = QFileDialog::getOpenFileName(this, tr("Select the PEM private key of the group"));
	if (keyFile == "") {
		return;
	}
	PrivateKey privateKey = PrivateKey::load(keyFile);
	if (!privateKey.isValid()) {
		QMessageBox::critical(this, "Error!",
				"The selected file does not contain a valid PEM encoded private key!");
		return;
	}
	// Let the user select a name to identify this group
	bool ok;
	QString groupName = QInputDialog::getText(this, "Name of this group",
			"Enter the name of this group", QLineEdit::Normal, "unknown-group", &ok);
	if (!ok) {
		return;
	}
	// Add the group
	QString pemKey = privateKey.toPEM();
	dbusNetwork.call("addGroupMembership", groupName, pemKey);
	// We do not have to refresh the list, this is done by a change signal from the service
}
void MainWindow::leaveGroup() {
	// TODO
}
void MainWindow::createGroup() {
	// Let the user select a name to identify this group
	bool ok;
	QString peerName = QInputDialog::getText(this, "Name of this group",
			"Enter the name of this group", QLineEdit::Normal, "unknown-group", &ok);
	if (!ok) {
		return;
	}
	// Automatically generate a random key pair for the group
	PrivateKey privateKey = PrivateKey::generate();
	QString pemKey = privateKey.toPEM();
	dbusNetwork.call("addGroupMembership", peerName, pemKey);
}
void MainWindow::exportPrivateGroupKey() {
	// TODO
}

void MainWindow::pollServiceStatus() {
	bool active = dbusService.isValid();
	if (active != serviceActive) {
		serviceActive = active;
		updateStatusText();
		if (serviceTimeoutTimer.isActive()) {
			serviceTimeoutTimer.stop();
		}
		emit serviceStatusChanged(active);
	}
}
void MainWindow::serviceStartTimeout() {
	QMessageBox::critical(this, "Error!", "Starting the service timed out.");
	updateStatusText();
}

void MainWindow::updateStatusText() {
	if (serviceActive) {
		statusLabel.setText("Service active.");
	} else {
		statusLabel.setText("Service inactive.");
	}
}

void MainWindow::onNodeStatusChanged(QString publicKey, QString fingerprint,
		NodeStatus nodeStatus, QStringList groups) {
	qDebug("onNodeStatusChanged");
	// TODO: Trusted, group membership, load
	//onlinePeerModel.updateNode("unknown", publicKey.left(32), false, 0.0f, false);
	onlinePeerModel.updateNode("unknown", fingerprint, false,
			(float)nodeStatus.currentThreads / nodeStatus.maxThreads, false);
	// TODO
}

void MainWindow::onTrustedPeersChanged(const QList<TrustedPeerInfo> &trustedPeers) {
	ui.trustedPeerList->clear();
	trustedPeerKeys.clear();
	foreach (TrustedPeerInfo peer, trustedPeers) {
		trustedPeerKeys.append(peer.publicKey);
		PublicKey key = PublicKey::fromPEM(peer.publicKey);
		ui.trustedPeerList->addItem(peer.name + ": " + key.fingerprint());
	}
}
void MainWindow::onTrustedGroupsChanged(const QList<TrustedGroupInfo> &trustedGroups) {
	ui.trustedGroupList->clear();
	trustedGroupKeys.clear();
	foreach (TrustedGroupInfo group, trustedGroups) {
		trustedGroupKeys.append(group.publicKey);
		PublicKey key = PublicKey::fromPEM(group.publicKey);
		ui.trustedGroupList->addItem(group.name + ": " + key.fingerprint());
	}
}
void MainWindow::onGroupMembershipsChanged(const QList<GroupMembershipInfo> &groupMemberships) {
	ui.groupMembershipList->clear();
	groupMembershipKeys.clear();
	foreach (GroupMembershipInfo group, groupMemberships) {
		groupMembershipKeys.append(group.privateKey);
		PrivateKey key = PrivateKey::fromPEM(group.privateKey);
		ui.groupMembershipList->addItem(group.name + ": " + PublicKey(key).fingerprint());
	}
}

void MainWindow::updateToolChainList(QList< ToolChainInfo > toolChains) {
	ui.toolChainList->clear();
	toolChainPaths.clear();
	if (!toolChains.empty()) {
		foreach (ToolChainInfo t , toolChains) {
			toolChainPaths.append(t.path);
			ui.toolChainList->addItem(QString("%1: %2").arg(t.version).arg(t.path));
		}
	}
}
