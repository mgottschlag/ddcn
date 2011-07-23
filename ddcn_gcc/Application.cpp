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

#include "Application.h"

#include <QDBusReply>
#include <QDBusInterface>
#include <cstdlib>
#include <iostream>
#include <QDir>
#include <cstdio>

QDBusArgument &operator<<(QDBusArgument &argument, const ToolChainInfo &info)
{
	argument.beginStructure();
	argument << info.version << info.path;
	argument.endStructure();
	return argument;
}
const QDBusArgument &operator>>(const QDBusArgument &argument, ToolChainInfo &info)
{
	argument.beginStructure();
	QString version;
	QString path;
	argument >> version >> path;
	argument.endStructure();
	info.version = version;
	info.path = path;
	return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const JobResult &jobResult) {
	argument.beginStructure();
	argument << jobResult.stdout << jobResult.stderr << jobResult.returnValue;
	argument.endStructure();
	return argument;
}
const QDBusArgument &operator>>(const QDBusArgument &argument, JobResult &jobResult) {
	argument.beginStructure();
	argument >> jobResult.stdout >> jobResult.stderr >> jobResult.returnValue;
	argument.endStructure();
	return argument;
}

int Application::run(int argc, char **argv) {
	// Fetch list of available tool chains
	QStringList availableToolChains = fetchToolChainList();
	if (availableToolChains.count() == 0) {
		qCritical("Error: No tool chains available or service not available.");
		return -1;
	}
	// Select a tool chain
	QString toolChain = availableToolChains[0];
	const char *toolChainEnv = getenv("DDCN_TOOLCHAIN");
	if (toolChainEnv) {
		bool toolchainFound = false;
		for (int i = 0; i < availableToolChains.size(); i++) {
			if (availableToolChains[i] == QString(toolChainEnv)) {
				toolchainFound = true;
				break;
			}
		}
		if (!toolchainFound) {
			qCritical("Error: Selected toolchain not supported by the service.");
			return -1;
		}
	}
	// Select a language
	QString language = "c";
	const char *languageEnv = getenv("DDCN_LANGUAGE");
	if (languageEnv) {
		language = languageEnv;
	}
	if (language != "c" && language != "c++") {
		language = "c";
	}
	// Check whether we need to read stdin
	QByteArray stdinData;
	for (int i = 1; i < argc; i++) {
		// "-" says that we should read from stdin
		if (!strcmp(argv[i], "-")) {
			stdinData = readStdin();
			break;
		}
	}
	// Send the job
	QStringList parameters;
	for (int i = 1; i < argc; i++) {
		parameters.append(argv[i]);
	}
	return executeJob(toolChain, parameters, stdinData, language);
}

QStringList Application::fetchToolChainList() {
	QDBusInterface interface("org.ddcn.service",
	                         "/CompilerService",
	                         "org.ddcn.CompilerService");
	if (!interface.isValid()) {
		qCritical("Error: Could not connect to compiler service.");
		return QStringList();
	}
	QDBusReply<QList<ToolChainInfo> > reply = interface.call("getToolChains");
	if (!reply.isValid()) {
		qCritical("Error: Could not call compiler service (getToolChains()).");
		return QStringList();
	}
	QList<ToolChainInfo> toolChainInfo = reply.value();
	QStringList toolChains;
	foreach(ToolChainInfo info, toolChainInfo) {
		qDebug("Toolchain available: %s", info.version.toAscii().data());
		toolChains.append(info.version);
	}

	return toolChains;
}
int Application::executeJob(QString toolChain, QStringList parameters,
		const QByteArray &stdinData, QString language) {
	QDBusInterface interface("org.ddcn.service",
	                         "/CompilerService",
	                         "org.ddcn.CompilerService");
	if (!interface.isValid()) {
		qCritical("Error: Compiler service not available.");
		return -1;
	}
	qDebug("Sending job, toolchain: %s", toolChain.toAscii().data());
	QList<QVariant> args;
	args <<  parameters << toolChain << QDir::currentPath() << stdinData
		<< language;
	QDBusMessage msg = QDBusMessage::createMethodCall(interface.service(), interface.path(), interface.interface(), "executeJob");
	msg.setArguments(args);
	QDBusReply<JobResult> reply = QDBusConnection::sessionBus().call(msg,
			QDBus::Block, 60000);
	if (!reply.isValid()) {
		qCritical(reply.error().message().toAscii().data());
		qCritical("Error: Could not call compiler service (executeJob()).");
		return -1;
	}
	JobResult result = reply.value();
	std::cout.write(result.stdout.data(), result.stdout.size());
	std::cerr.write(result.stderr.data(), result.stderr.size());
	return result.returnValue;
}

QByteArray Application::readStdin() {
	QFile input;
	input.open(stdin, QIODevice::ReadOnly);
	QByteArray inputData = input.readAll();
	return inputData;
}
