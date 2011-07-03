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

#include "CompilerServiceAdaptor.h"
#include "ParameterParser.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QVariant>

CompilerServiceAdaptor::CompilerServiceAdaptor(CompilerService *service)
		: QDBusAbstractAdaptor(service), service(service) {
	connect(service,
			SIGNAL(currentThreadCountChanged(int)),
			this,
			SLOT(onCurrentThreadCountChanged(int)));
	connect(service,
			SIGNAL(maxThreadCountChanged(int)),
			this,
			SLOT(onMaxThreadCountChanged(int)));
	connect(service,
			SIGNAL(localJobCompilationFinished(Job*)),
			this,
			SLOT(localCompilationJobFinished(Job*)));
}

void CompilerServiceAdaptor::setMaxThreadCount(int threadCount) {
	service->setMaxThreadCount(threadCount);
}

int CompilerServiceAdaptor::getCurrentThreadCount() {
	return service->getCurrentThreadCount();
}

int CompilerServiceAdaptor::getMaxThreadCount() {
	return service->getMaxThreadCount();
}

void CompilerServiceAdaptor::onCurrentThreadCountChanged(int threadCount) {
	emit currentThreadCountChanged(threadCount);
}

void CompilerServiceAdaptor::onMaxThreadCountChanged(int threadCount) {
	emit maxThreadCountChanged(threadCount);
}

JobResult CompilerServiceAdaptor::executeJob(QStringList parameters,
									   QString toolChain, QString workingPath,
									   const QDBusMessage &message) {
	//TODO DEBUG:qCritical("DBus anfrage");
	ParameterParser parser(parameters);
	//TODO DEBUG:qCritical("Parser durch");
	Job *job = new Job(parser.getInputFiles(), parser.getOutputFiles(),
	                   parser.getOriginalParameters(),
	                   parser.getPreprocessingParameters(),
	                   parser.getCompilerParameters(),
	                   toolChain, workingPath, false, parser.isDelegatable());
	//TODO DEBUG:qCritical("job erzeugt");
	message.setDelayedReply(true);
	//TODO DEBUG:qCritical("setDelayedReply");
	QDBusMessage *dBusMessage = new QDBusMessage(message.createReply());
	//TODO DEBUG:qCritical("Reply created");
	this->jobDBusMessageMap.insert(job, dBusMessage);

	//TODO DEBUG:qCritical("DBus anfrage bearbeitet");
	service->addJob(job);
	return JobResult();
}

void CompilerServiceAdaptor::requestShutdown() {
	shutdown();
}

void CompilerServiceAdaptor::shutdown() {
	//-> main
	emit onShutdownRequest();
}

void CompilerServiceAdaptor::addToolChain(QString path) {
	this->service->addToolChain(path);
}

void CompilerServiceAdaptor::removeToolChain(QString path) {
	this->service->removeToolChain(path);
}

void CompilerServiceAdaptor::localCompilationJobFinished(Job *job) {
	QDBusMessage *message(this->jobDBusMessageMap.value(job));
	QDBusArgument argument;
	QVariant variant = QVariant::fromValue(job->getJobResult());
	*message << variant;
	QDBusConnection::sessionBus().send(*message);
	this->jobDBusMessageMap.remove(job);
	delete message;
}

ToolChainInfo CompilerServiceAdaptor::toToolChainInfo(ToolChain toolChain) {
	ToolChainInfo info;
	info.path = toolChain.getPath();
	info.version = toolChain.getVersion();
	return info;
}

QList<ToolChainInfo> CompilerServiceAdaptor::getToolChains() {
	QList<ToolChainInfo> toolChainInfoList;
	QList<ToolChain> *toolChainsList = this->service->getToolChains();
	for (int i = 0; i < toolChainsList->count(); i++) {
		ToolChain toolChain = (*toolChainsList)[i];
		toolChainInfoList.append(toToolChainInfo(toolChain));
	}
	return toolChainInfoList;
}

