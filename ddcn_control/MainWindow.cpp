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

#include <QDesktopServices>
#include <QMessageBox>
#include <QUrl>
#include <QProcess>
#include <QDBusInterface>
#include "SettingsDialog.h"

MainWindow::MainWindow() {
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
	QMessageBox::critical(this, "Error!", "Not yet implemented.");
	// TODO
}
void MainWindow::removeToolChain() {
	QMessageBox::critical(this, "Error!", "Not yet implemented.");
	// TODO
}

void MainWindow::pollServiceStatus() {
	QDBusInterface dbusInterface("org.ddcn.service", "/CompilerService", "org.ddcn.CompilerService");
	bool active = dbusInterface.isValid();
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
