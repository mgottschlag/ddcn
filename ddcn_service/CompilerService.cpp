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

#include "CompilerService.h"


QString CompilerService::settingToolChains("toolChains");
QString CompilerService::settingToolChainPath("path");
QString CompilerService::settingToolChainVersion("version");
QString CompilerService::settingMaxThreadCount("maxThreadCount");


CompilerService::CompilerService(CompilerNetwork *network)
		: settings(QSettings::IniFormat, QSettings::UserScope, "ddcn", "ddcn") {
	this->network = network;
	setCurrentThreadCount(0);
	determineAndSetMaxThreadCount();
	loadToolChains();
	// Connect network signals
	connect(network, SIGNAL(receivedJob(Job*)), this, SLOT(onReceivedJob(Job*)));
	connect(network, SIGNAL(remoteJobAborted(Job*)), this, SLOT(onRemoteJobAborted(Job*)));
	network->setFreeLocalSlots(computeFreeLocalSlotCount());
}

void CompilerService::addJob(Job *job) {
	if (job->isRemoteJob()) {
		emit numberOfJobsInRemoteQueueChanged(this->remoteJobQueue.count());
		connect(job,
			SIGNAL(finished(Job*)),
			this,
			SLOT(onRemoteCompileFinished(Job*))
		);
		this->remoteJobQueue.append(job);
	} else {
		// Local jobs shall trigger a signal which gets forwarded to the adaptor
		connect(job,
			SIGNAL(finished(Job*)),
			this,
			SLOT(onLocalCompileFinished(Job*))
		);
		this->localJobQueue.append(job);
		emit numberOfJobsInLocalQueueChanged(this->localJobQueue.count());
	}
	network->setFreeLocalSlots(computeFreeLocalSlotCount());
	//TODO DEBUG:qCritical("Job hinzugefuegt");
	manageJobs();
}


//SLOTS
void CompilerService::onIncomingJob(Job *job) {
	job->setRemoteJob(true);
	this->addJob(job);
}

/*void CompilerService::onRemoteCompileFinished(Job *job, bool executed) {
	if (executed) {
		this->removeJob(job);
		emit numberOfJobsInLocalQueueChanged(this->localJobQueue.count());
		emit numberOfJobsInRemoteQueueChanged(this->remoteJobQueue.count());
	} else {
		this->localJobQueue.move(this->localJobQueue.indexOf(job, 0), this->localJobQueue.count());
	}
	manageJobs();
}*/

//PRIVATE

bool CompilerService::removeJob(Job *job) {
	//Tries to locate a given Job in the local job queue or in the remote job queue and removes it,
	//otherwise return false.
	if (this->localJobQueue.removeOne(job)) {
		network->setFreeLocalSlots(computeFreeLocalSlotCount());
		return true;
	} else if (this->remoteJobQueue.removeOne(job)) {
		network->setFreeLocalSlots(computeFreeLocalSlotCount());
		return true;
	} else {
		return false;
	}
}

void CompilerService::manageJobs() {
	//TODO DEBUG: qCritical("Managing gestartet");
	manageLocalJobs();
	manageOutgoingJobs();
}

void CompilerService::manageLocalJobs() {
	//compiles maxThreadCount jobs at the same time.
	//if there are no more jobs to compile in the localJobQueue, get back enough jobs from the network in
	//			order to compile maxThreadCount jobs locally
	//if there are no more (own) jobs to compile at all, start to compile remoteJobs
	while (this->maxThreadCount > this->currentThreadCount
		&& this->localJobQueue.count() > 0) {
		executeFirstJobFromList(&this->localJobQueue);
	}
	while (this->maxThreadCount > this->currentThreadCount
			&& this->localJobQueue.count() == 0) {
		Job *job = this->network->cancelOutgoingJob();
		if (job != NULL) {
			executeJobLocally(job);
		} else {
			break;
		}
	}
	while (this->maxThreadCount > this->currentThreadCount
		&& this->remoteJobQueue.count() > 0) {
		executeFirstJobFromList(&this->remoteJobQueue);
	}
}

void CompilerService::executeFirstJobFromList(QList<Job*> *jobList) {
		//TODO DEBUG:qCritical("executeFirstJobFromList");
		Job *job = jobList->first();
		jobList->removeFirst();
		executeJobLocally(job);
}


void CompilerService::executeJobLocally(Job* job) {
	//TODO DEBUG:qCritical("executeJobLocally");
	job->execute();
	setCurrentThreadCount(this->currentThreadCount + 1);
}

void CompilerService::manageOutgoingJobs() {
	//delegates all jobs until only this->maxThreadCount * 2 Jobs remain in localJobQueue
	//removes delegated Jobs from the delegatedJobQueue
	while (this->maxThreadCount < this->localJobQueue.count()) {
		Job *job = extractLocalDelegatableJob();
		if (job == NULL) {
			break;
		}
		//this->delegatedJobQueue.append(job);
		//network->delegateOutgoingJob(this->delegatedJobQueue.first());
		network->delegateOutgoingJob(job);
		//this->delegatedJobQueue.removeFirst();
	}
	network->setFreeLocalSlots(computeFreeLocalSlotCount());
}



void CompilerService::loadToolChains() {
	int size = this->settings.beginReadArray(this->settingToolChains);
	for (int i = 0; i < size; ++i) {
		this->settings.setArrayIndex(i);
		QString version = this->settings.value(this->settingToolChainVersion).toString();
		QString path = this->settings.value(this->settingToolChainPath).toString();
		if (QFile(path).exists()) {
			ToolChain toolChain(version, path);
			this->toolChains.append(toolChain);
		}
	}
	this->settings.endArray();
	if (this->toolChains.count() < 1) {
		qWarning("No toolchains loaded, creating default toolchain entry.");
		// Try to fetch the default compiler
		ToolChain defaultToolChain("/usr/bin/gcc");
		if (defaultToolChain.getVersion() != "") {
			this->toolChains.append(defaultToolChain);
		}
		// Save the updated toolchain list
		saveToolChains();
	}
	if (this->toolChains.count() < 1) {
		qFatal("No ToolChains available!");
	}
	network->setToolChains(toolChains);
}

void CompilerService::saveToolChains()  {
	this->settings.beginWriteArray(this->settingToolChains);
	for (int i = 0; i < this->toolChains.size(); ++i) {
		this->settings.setArrayIndex(i);
		this->settings.setValue(this->settingToolChainVersion, this->toolChains.at(i).getVersion());
		this->settings.setValue(this->settingToolChainPath, this->toolChains.at(i).getPath());
	}
	this->settings.endArray();
}

void CompilerService::determineAndSetMaxThreadCount() {
	this->maxThreadCount = this->settings.value(this->settingMaxThreadCount, -1).toInt();
	if (this->maxThreadCount <= 0) {
		setMaxThreadCount(-1);
	}
}

void CompilerService::saveMaxThreadCount(int count) {
	//int QThread::idealThreadCount () determines the number of cores (physically and virtually)
	int systemCount = QThread::idealThreadCount();
	this->maxThreadCount = ((count >= systemCount || count < 1) ? systemCount : count);
	this->settings.setValue(this->settingMaxThreadCount, this->maxThreadCount);
}

bool CompilerService::isToolChainAvailable(ToolChain target) {
	foreach (ToolChain toolChainListObjekt, toolChains) {
		if (toolChainListObjekt == target) {
			return true;
		}
	}
	return false;
}

// PRIVATE SLOTS
void CompilerService::onLocalCompileFinished(Job* job) {
	//TODO DEBUG:qCritical("Compiler finished");
	emit localJobCompilationFinished(job);
	if (!job->wasDelegated()) {
		emit numberOfJobsInLocalQueueChanged(this->localJobQueue.count());
		setCurrentThreadCount(this->currentThreadCount - 1);
	}
	manageJobs();
}
void CompilerService::onRemoteCompileFinished(Job* job) {
	qDebug("onRemoteCompileFinished()");
	setCurrentThreadCount(this->currentThreadCount - 1);
	network->onDelegatedJobFinished(job);
	manageJobs();
}

void CompilerService::onReceivedJob(Job *job) {
	qDebug("onReceivedJob()");
	addJob(job);
}
void CompilerService::onRemoteJobAborted(Job *job) {
	qDebug("onRemoteJobAborted()");
	if (!remoteJobQueue.removeOne(job)) {
		// If the job is not in the queue, this means that it is active right
		// now. In this case we do not have to do anything as the job is killed
		// by CompilerNetwork
		// However, we have to free one thread as the process will be killed
		setCurrentThreadCount(this->currentThreadCount - 1);
	}
}

unsigned int CompilerService::computeFreeLocalSlotCount() {
	// free slots = 2*max threads - active jobs, so that every thread has one
	// more job in the queue
	return std::max(maxThreadCount - localJobQueue.size() - remoteJobQueue.size(), 0);
}

Job *CompilerService::extractLocalDelegatableJob() {
	for (int i = this->localJobQueue.size() - 1; i >= 0; i--) {
		if (this->localJobQueue[i]->isDelegatable()) {
			Job *job = this->localJobQueue[i];
			this->localJobQueue.removeAt(i);
			return job;
		}
	}
	return NULL;
}
