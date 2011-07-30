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
#include <QCloseEvent>

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
	ui.onlineGroupList->setItemDelegate(&onlineGroupItemDelegate);
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
	refreshAllWidgets();
	pollServiceStatus();
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
	onlineGroupModel.clear();
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
	QList<QListWidgetItem*> selected = ui.trustedPeerList->selectedItems();
	if (selected.empty()) {
		return;
	}
	int selectedIndex = ui.trustedPeerList->row(selected[0]);
	dbusNetwork.call("removeTrustedPeer", "", trustedPeerKeys[selectedIndex]);
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
	QList<QListWidgetItem*> selected = ui.trustedGroupList->selectedItems();
	if (selected.empty()) {
		return;
	}
	int selectedIndex = ui.trustedGroupList->row(selected[0]);
	dbusNetwork.call("removeTrustedGroup", "", trustedGroupKeys[selectedIndex]);
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
	QList<QListWidgetItem*> selected = ui.groupMembershipList->selectedItems();
	if (selected.empty()) {
		return;
	}
	int selectedIndex = ui.groupMembershipList->row(selected[0]);
	PublicKey key = PrivateKey::fromPEM(groupMembershipKeys[selectedIndex]);
	dbusNetwork.call("removeGroupMembership", "", key.toPEM());
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
	QList<QListWidgetItem*> selected = ui.groupMembershipList->selectedItems();
	if (selected.empty()) {
		return;
	}
	int selectedIndex = ui.groupMembershipList->row(selected[0]);
	PrivateKey key = PrivateKey::fromPEM(groupMembershipKeys[selectedIndex]);
	// Let the user select a target file
	QString fileName = QFileDialog::getSaveFileName(this, "Select the place where to save the group key");
	if (fileName == "") {
		return;
	}
	if (!key.save(fileName)) {
		QMessageBox::critical(this, "Error!",
				"Could not save the private key of the group!");
	}
}

void MainWindow::pollServiceStatus() {
	bool active = dbusService.isValid();
	if (active != serviceActive) {
		serviceActive = active;
		if (serviceTimeoutTimer.isActive()) {
			serviceTimeoutTimer.stop();
		}
		refreshAllWidgets();
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
	float load = (float)nodeStatus.currentThreads / nodeStatus.maxThreads;
	// Add node to the group list
	QStringList groupMembershipList;
	foreach (QString privateKey, groupMembershipKeys) {
		PublicKey key = PrivateKey::fromPEM(privateKey);
		groupMembershipList.append(key.toPEM());
	}
	bool inTrustedGroup = false;
	foreach (QString groupKey, groups) {
		PublicKey key = PublicKey::fromPEM(groupKey);
		bool trusted = false;
		bool member = false;
		if (trustedGroupKeys.contains(groupKey)) {
			inTrustedGroup = true;
			trusted = true;
		}
		if (groupMembershipList.contains(groupKey)) {
			member = true;
		}
		// TODO: Name
		onlineGroupModel.addNodeToGroup("unknown", key.fingerprint(), trusted,
				load, member);
	}
	// Add node to the peer list
	bool trusted = trustedPeerKeys.contains(publicKey);
	// TODO: Name
	onlinePeerModel.updateNode("unknown", fingerprint, trusted, load, inTrustedGroup);
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

void MainWindow::closeEvent(QCloseEvent *event) {
	if (serviceActive) {
		QMessageBox::StandardButton button = QMessageBox::question(this,
				"Close service?",
				"The local DDCN service is still running, do you want to stop it?",
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
				QMessageBox::Cancel);
		if (button == QMessageBox::No) {
			// Don't do anything here, just close the window
			event->accept();
		} else if (button == QMessageBox::Yes) {
			stopService();
			event->accept();
		} else {
			// Do not close the window
			event->ignore();
		}
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

void MainWindow::refreshAllWidgets() {
	updateStatusText();
	if (serviceActive) {
		QDBusReply<QString> nameReply = dbusNetwork.call("getPeerName");
		ui.localNameLabel->setText(nameReply.value());
		QDBusReply<QString> keyReply = dbusNetwork.call("getLocalKey");
		PublicKey key = PublicKey::fromPEM(keyReply.value());
		QString fingerprint = key.fingerprint();
		ui.localKeyLabel->setText(fingerprint.left(8) + "..." + fingerprint.right(8));
		ui.localKeyLabel->setToolTip(fingerprint);
		QDBusReply<int> intReply = dbusService.call("getMaxThreadCount");
		int maxThreadCount = intReply.value();
		intReply = dbusService.call("getCurrentThreadCount");
		int currentThreadCount = intReply.value();
		ui.threadCountLabel->setText(QString::number(maxThreadCount));
		ui.serviceActiveLabel->setText("yes");
		intReply = dbusService.call("getNumberOfJobsInLocalQueue");
		ui.labelLocalJobs->setText(QString::number(intReply.value()));
		intReply = dbusService.call("getNumberOfJobsInRemoteQueue");
		ui.labelIncomingJobs->setText(QString::number(intReply.value()));
		// TODO
		ui.labelDelegatedJobs->setText("0");
		ui.labelCompletedJobs->setText("0");
		ui.workloadBar->setMaximum(maxThreadCount);
		ui.workloadBar->setValue(currentThreadCount);
		// TODO
		ui.trustedPeerList->clear();
		ui.trustedGroupList->clear();
		ui.groupMembershipList->clear();
		QDBusReply<QList<ToolChainInfo> > toolChains = dbusService.call("getToolChains");
		updateToolChainList(toolChains.value());
		ui.logList->clear();
		onlinePeerModel.clear();
		// TODO: Online group model
	} else {
		ui.localNameLabel->setText("<not connected>");
		ui.localKeyLabel->setText("<not connected>");
		ui.threadCountLabel->setText("1");
		ui.serviceActiveLabel->setText("no");
		ui.labelLocalJobs->setText("0");
		ui.labelIncomingJobs->setText("0");
		ui.labelDelegatedJobs->setText("0");
		ui.labelCompletedJobs->setText("0");
		ui.workloadBar->setMaximum(1);
		ui.workloadBar->setValue(0);
		ui.trustedPeerList->clear();
		ui.trustedGroupList->clear();
		ui.groupMembershipList->clear();
		ui.toolChainList->clear();
		ui.logList->clear();
		onlinePeerModel.clear();
		// TODO: Online group model
	}
}
