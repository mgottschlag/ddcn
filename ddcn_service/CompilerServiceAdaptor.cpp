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
	connect(service,
			SIGNAL(numberOfJobsInLocalQueueChanged(int)),
			this,
			SLOT(onNumberOfJobsInLocalQueueChanged(int)));
	connect(service,
			SIGNAL(numberOfJobsInRemoteQueueChanged(int)),
			this,
			SLOT(onNumberOfJobsInRemoteQueueChanged(int)));
	connect(service,
			SIGNAL(toolChainsChanged()),
			this,
			SLOT(onToolChainsChanged()));
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
void CompilerServiceAdaptor::onNumberOfJobsInLocalQueueChanged(int noj) {
	emit numberOfJobsInLocalQueueChanged(noj);
}
void CompilerServiceAdaptor::onNumberOfJobsInRemoteQueueChanged(int noj) {
	emit numberOfJobsInRemoteQueueChanged(noj);
}
void CompilerServiceAdaptor::onToolChainsChanged() {
	emit toolChainsChanged(this->getToolChains());
}

JobResult CompilerServiceAdaptor::executeJob(QStringList parameters,
		QString toolChain, QString workingPath,
		const QByteArray &stdinData, QString language,
		const QDBusMessage &message) {
	// Fetch the toolchain path
	QList<ToolChain> toolChains = *service->getToolChains();
	ToolChain toolChainInfo;
	for (int i = 0; i < toolChains.size(); i++) {
		if (toolChains[i].getVersion() == toolChain) {
			// TODO: Rather pass the whole ToolChain class here so that the job
			// can get the right filename for the language (gcc/g++)?
			toolChainInfo = toolChains[i];
			break;
		}
	}
	// TODO: Error checking - toolchain unsupported?
	//TODO DEBUG:qCritical("DBus anfrage");
	ParameterParser parser(parameters);
	//TODO DEBUG:qCritical("Parser durch");
	Job *job = new Job(parser.getInputFiles(), parser.getOutputFiles(),
	                   parser.getOriginalParameters(),
	                   parser.getPreprocessingParameters(),
	                   parser.getCompilerParameters(),
	                   toolChainInfo, workingPath, false, parser.isDelegatable(),
	                   stdinData, language);
	// TODO: Language
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

bool CompilerServiceAdaptor::addToolChain(QString path) {
	return this->service->addToolChain(path);
}

bool CompilerServiceAdaptor::removeToolChain(QString path) {
	return this->service->removeToolChain(path);
}
int CompilerServiceAdaptor::getNumberOfJobsInLocalQueue() {
	return service->getNumberOfJobsInLocalQueue();
}
int CompilerServiceAdaptor::getNumberOfJobsInRemoteQueue() {
	return service->getNumberOfJobsInRemoteQueue();
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

