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

#include "ServiceConnection.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <iostream>

ServiceConnection::ServiceConnection(QObject *parent) : QObject(parent) {
}

void ServiceConnection::sendJob(Job *job) {
	currentJob = job;
	// Fetch data needed to send the job
	QStringList parameters = job->getRemoteParameters();
	QList<QByteArray> inputFiles = job->readPreprocessedFiles();
	QString target = "x86_64-redhat-linux";
	// Send the job
	QDBusInterface interface("org.ddcn.Service",
	                         "/CompilerService",
	                         "org.ddcn.CompilerService");
	if (!interface.isValid()) {
		emit sendingFailed();
		return;
	}
	QList<QVariant> inputFilesParameter = QList<QVariant>();
	foreach (QByteArray file, inputFiles) {
		inputFilesParameter.append(file);
	}
	QDBusReply<QString> reply = interface.call("addJob",
	                                           inputFilesParameter,
	                                           QVariant(parameters),
	                                           QVariant(target));
	if (!reply.isValid()) {
		emit sendingFailed();
		return;
	}
	// Wait for the job to finish
	QString pathName = reply.value();
	bool connected = QDBusConnection::sessionBus().connect("org.ddcn.Service",
	                                                       pathName,
	                                                       "org.ddcn.Job",
	                                                       "finished",
	                                                       this,
	                                                       SLOT(onJobFinished(bool, int, QString, QList<QByteArray>)));
	if (!connected) {
		// FIXME: If this happens, the job was already completed before we
		// could react, this race condition has to be fixed
		emit sendingFailed();
		return;
	}
}

void ServiceConnection::onJobFinished(bool executed,
                                      int returnValue,
                                      QString consoleOutput,
                                      QList<QByteArray> outputFiles) {
	if (executed) {
		std::cout << consoleOutput.toStdString() << std::endl;
		if (!currentJob->writeOutputFiles(outputFiles)) {
			currentJob->wasFinished(-1);
		} else {
			currentJob->wasFinished(returnValue);
		}
	} else {
		// Fallback, compile locally
		emit sendingFailed();
	}
}
