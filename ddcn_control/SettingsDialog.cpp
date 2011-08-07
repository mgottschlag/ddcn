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

#include "SettingsDialog.h"
#include "../ddcn_crypto/PublicKey.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QInputDialog>
#include <QMessageBox>
#include <QThread>
#include <QFileDialog>

SettingsDialog::SettingsDialog()
		: dbusService("org.ddcn.service", "/CompilerService", "org.ddcn.CompilerService"),
		dbusNetwork("org.ddcn.service", "/CompilerNetwork", "org.ddcn.CompilerNetwork"),
		peerNameChanged(false), threadCountChanged(false), keyChanged(false),
		bootstrappingChanged(false), endpointsChanged(false), compressionChanged(false),
		readyForInput(false) {
	ui.setupUi(this);
}

bool SettingsDialog::fetchSettings() {
	// Check the connection to the service
	if (!dbusService.isValid()) {
		return false;
	}
	if (!dbusNetwork.isValid()) {
		return false;
	}
	// Read current values
	QDBusReply<int> intReply = dbusService.call("getMaxThreadCount");
	if (intReply.isValid()) {
		ui.threadCount->setValue(intReply.value());
	} else {
		return false;
	}
	QDBusReply<QString> stringReply = dbusNetwork.call("getPeerName");
	if (stringReply.isValid()) {
		ui.peerName->setText(stringReply.value());
	} else {
		return false;
	}
	stringReply = dbusNetwork.call("getLocalKey");
	if (stringReply.isValid()) {
		PublicKey key = PublicKey::fromPEM(stringReply.value());
		QString fingerprint = key.fingerprint();
		ui.keyLabel->setText(fingerprint.left(8) + "..." + fingerprint.right(8));
		ui.keyLabel->setToolTip(fingerprint);
	} else {
		return false;
	}
	QDBusReply<bool> boolReply = dbusNetwork.call("getCompression");
	if (boolReply.isValid()) {
		ui.useCompression->setChecked(boolReply.value());
	} else {
		return false;
	}
	// TODO: Bootstrapping, endpoints
	// Everything has been set up, so we can accept signals now
	readyForInput = true;
	return true;
}
bool SettingsDialog::writeSettings() {
	// Check the connection to the service
	if (!dbusService.isValid()) {
		return false;
	}
	if (!dbusNetwork.isValid()) {
		return false;
	}
	if (threadCountChanged) {
		dbusService.call("setMaxThreadCount", threadCount);
	}
	if (peerNameChanged) {
		dbusNetwork.call("setPeerName", peerName);
	}
	if (keyChanged) {
		dbusNetwork.call("setLocalKey", privateKey);
	}
	if (compressionChanged) {
		dbusNetwork.call("setCompression", useCompression);
	}
	// TODO: Bootstrapping, endpoints
	return true;
}

void SettingsDialog::generateKey() {
	bool ok;
	int bits = QInputDialog::getInt(this, "Enter key length",
			"Enter the length of the key in bits.", 2048, 0, 1 << 16, 1, &ok);
	if (!ok) {
		return;
	}
	PrivateKey privateKey = PrivateKey::generate(bits);
	if (!privateKey.isValid()) {
		QMessageBox::critical(this, "Error",
				"Could not generate a key with this length.");
		return;
	}
	keyChanged = true;
	this->privateKey = privateKey.toPEM();
	QString fingerprint = PublicKey(privateKey).fingerprint();
	ui.keyLabel->setText(fingerprint.left(8) + "..." + fingerprint.right(8));
	ui.keyLabel->setToolTip(fingerprint);
}
void SettingsDialog::importKey() {
	QString fileName = QFileDialog::getOpenFileName(this,
			tr("Select a PEM encoded private key file"));
	if (fileName == "") {
		return;
	}
	PrivateKey privateKey = PrivateKey::load(fileName);
	if (!privateKey.isValid()) {
		QMessageBox::critical(this, "Error",
				"Could not load the key. Are you sure it is both PEM encoded and a private key?");
		return;
	}
	keyChanged = true;
	this->privateKey = privateKey.toPEM();
	QString fingerprint = PublicKey(privateKey).fingerprint();
	ui.keyLabel->setText(fingerprint.left(8) + "..." + fingerprint.right(8));
	ui.keyLabel->setToolTip(fingerprint);
}
void SettingsDialog::computeThreadCount() {
	ui.threadCount->setValue(QThread::idealThreadCount());
}

void SettingsDialog::onBootstrappingChanged(const QString &bootstrapping) {
	if (!readyForInput) {
		return;
	}
	bootstrappingChanged = true;
	this->bootstrapping = bootstrapping;
}
void SettingsDialog::onEndpointsChanged(const QString &endpoints) {
	if (!readyForInput) {
		return;
	}
	endpointsChanged = true;
	this->endpoints = endpoints;
}
void SettingsDialog::onPeerNameChanged(const QString &peerName) {
	if (!readyForInput) {
		return;
	}
	peerNameChanged = true;
	this->peerName = peerName;
}
void SettingsDialog::onCompressionChanged(bool useCompression) {
	if (!readyForInput) {
		return;
	}
	compressionChanged = true;
	this->useCompression = useCompression;
}
void SettingsDialog::onThreadCountChanged(int threadCount) {
	if (!readyForInput) {
		return;
	}
	threadCountChanged = true;
	this->threadCount = threadCount;
}
