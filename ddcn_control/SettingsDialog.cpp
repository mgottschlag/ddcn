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

#include "SettingsDialog.h"

#include <QDBusInterface>
#include <QDBusReply>

SettingsDialog::SettingsDialog() {
	ui.setupUi(this);
	// Read current values
	QDBusInterface serviceInterface("org.ddcn.service", "/CompilerService", "org.ddcn.CompilerService");
	if (!serviceInterface.isValid()) {
		return;
	}
	QDBusReply<int> intReply = serviceInterface.call("getMaxThreadCount");
	if (intReply.isValid()) {
		oldThreadCount = intReply.value();
		ui.threadCount->setValue(oldThreadCount);
	} else {
		oldThreadCount = 0;
	}
	QDBusInterface networkInterface("org.ddcn.service", "/CompilerNetwork", "org.ddcn.CompilerNetwork");
	if (!networkInterface.isValid()) {
		return;
	}
	QDBusReply<QString> stringReply = networkInterface.call("getPeerName");
	if (stringReply.isValid()) {
		oldPeerName = stringReply.value();
		peerName = oldPeerName;
		ui.peerName->setText(oldPeerName);
	} else {
		ui.peerName->setText("unknown_peer");
	}
	stringReply = networkInterface.call("getKeyFingerprint");
	if (stringReply.isValid()) {
		oldKeyFingerprint = stringReply.value();
		keyFingerprint = oldKeyFingerprint;
		ui.keyLabel->setText(oldKeyFingerprint);
	} else {
		ui.keyLabel->setText("unknown_key");
	}
}

bool SettingsDialog::isPeerNameChanged() {
	return peerName != oldPeerName;
}
QString SettingsDialog::getPeerName() {
	return peerName;
}

bool SettingsDialog::isThreadCountChanged() {
	return threadCount != oldThreadCount;
}
int SettingsDialog::getThreadCount() {
	return threadCount;
}

bool SettingsDialog::isKeyChanged() {
	return keyFingerprint != oldKeyFingerprint;
}
QString SettingsDialog::getKey() {
	return privateKey;
}

void SettingsDialog::generateKey() {
	// TODO
}
void SettingsDialog::importKey() {
	// TODO
}
void SettingsDialog::computeThreadCount() {
	// TODO
}

void SettingsDialog::onAccepted() {
	// TODO
}
