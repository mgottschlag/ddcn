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

#ifndef MAINWINDOW_H_INCLUDED
#define MAINWINDOW_H_INCLUDED

#include "OnlinePeerModel.h"
#include "OnlineGroupModel.h"
#include "OnlinePeerItemDelegate.h"

#include <QMainWindow>
#include <QTimer>
#include <QDBusArgument>

#include "ui_MainWindow.h"
#include <QDBusInterface>
#include <QMessageBox>

struct NodeStatus {
	int maxThreads;
	int currentThreads;
	int localJobs;
	int delegatedJobs;
	int remoteJobs;
};

Q_DECLARE_METATYPE(NodeStatus)


struct ToolChainInfo {
	QString path;
	QString version;
};

Q_DECLARE_METATYPE(ToolChainInfo)
Q_DECLARE_METATYPE(QList<ToolChainInfo>)

QDBusArgument &operator<<(QDBusArgument &argument, const ToolChainInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &argument, ToolChainInfo &info);

QDBusArgument &operator<<(QDBusArgument &argument, const NodeStatus &nodeStatusInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, NodeStatus &nodeStatusInfo);

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow();
public slots:
	void startService();
	void stopService();
	void showSettings();
	void openHelp();
	void addToolChain();
	void removeToolChain();
	void refreshNetworkStatus();
private slots:
	void pollServiceStatus();
	void serviceStartTimeout();
	void updateStatusText();
	void onNodeStatusChanged(QString publicKey, QString fingerprint,
			NodeStatus nodeStatus, QStringList groups);
	void onCurrentThreadCountChanged(int threadCount) {
		this->currentThreads = serviceActive ? threadCount : 0;
		updateThreadCount();
	}
	void onMaxThreadCountChanged(int threadCount) {
		this->maxThreads = serviceActive ? threadCount : 0;
		updateThreadCount();
	}
	void onNumberOfLocalJobsChanged(int noj) {
		ui.labelLocalJobs->setText(QString::number(noj));
	}
	void onNumberOfRemoteJobsChanged(int noj) {
		ui.labelRemoteJobs->setText(QString::number(noj));
	}
	void updateThreadCount() {
		ui.labelWorkload->setText(QString("%1 / %2").arg(this->currentThreads).arg(this->maxThreads));
	}
	void onToolChainsChanged(QList<ToolChainInfo> toolChains) {
		updateToolChainList(toolChains);
	}
signals:
	void serviceStatusChanged(bool active);
private:
	Ui::MainWindow ui;

	QTimer serviceStatusTimer;
	QTimer serviceTimeoutTimer;

	bool serviceActive;
	int currentThreads;
	int maxThreads;

	QDBusInterface dbusNetwork;
	QDBusInterface dbusService;
	
	QLabel statusLabel;

	QStringList toolChainPaths;

	OnlinePeerModel onlinePeerModel;
	OnlineGroupModel onlineGroupModel;

	OnlinePeerItemDelegate onlinePeerItemDelegate;
    QString findToolChainPathOfListItem(QListWidgetItem* item);
	void updateToolChainList(QList<ToolChainInfo> toolChains);
};

#endif
