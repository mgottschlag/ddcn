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

#ifndef SETTINGSDIALOG_H_INCLUDED
#define SETTINGSDIALOG_H_INCLUDED

#include <QDialog>
#include <QDBusInterface>

#include "ui_SettingsDialog.h"

class SettingsDialog : public QDialog {
	Q_OBJECT
public:
	SettingsDialog();

	bool fetchSettings();
	bool writeSettings();
private slots:
	void generateKey();
	void importKey();
	void computeThreadCount();
	void onBootstrappingChanged(const QString &bootstrapping);
	void onEndpointsChanged(const QString &endpoints);
	void onPeerNameChanged(const QString &peerName);
	void onCompressionChanged(bool useCompression);
	void onThreadCountChanged(int threadCount);
private:
	Ui::SettingsDialog ui;

	QDBusInterface dbusService;
	QDBusInterface dbusNetwork;

	bool peerNameChanged;
	QString peerName;
	bool threadCountChanged;
	int threadCount;
	bool keyChanged;
	QString keyFingerprint;
	QString privateKey;
	bool bootstrappingChanged;
	QString bootstrapping;
	bool endpointsChanged;
	QString endpoints;
	bool compressionChanged;
	bool useCompression;

	bool readyForInput;
};

#endif
