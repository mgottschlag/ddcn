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


CompilerService::CompilerService(CompilerNetwork *network) : settings("ddcn", "ddcn") {
	this->network = network;
	setCurrentThreadCount(0);
	determineAndSetMaxThreadCount(); 
	this->toolChains.append(ToolChain("/usr/bin/gcc"));//TODO Debug Only
	loadToolChains();
}

void CompilerService::addJob(Job *job) {
	if (job->isRemoteJob()) {
		this->localJobQueue.append(job);
	} else {
		this->remoteJobQueue.append(job);
	}
	manageJobs();
}


//SLOTS
void CompilerService::onIncomingJob(Job *job) {
	job->setRemoteJob(true);
	this->addJob(job);
}

void CompilerService::onRemoteCompileFinished(Job *job, bool executed) {
	if (executed) {
		this->removeJob(job);
	} else {
		this->localJobQueue.move(this->localJobQueue.indexOf(job, 0), this->localJobQueue.count());
	}
	manageJobs();
}

//PRIVATE

bool CompilerService::removeJob(Job *job) {
	//Tries to locate a given Job in the local job queue or in the remote job queue and removes it,
	//otherwise return false.
	return (this->localJobQueue.removeOne(job)) ?  true : (this->remoteJobQueue.removeOne(job));
}

void CompilerService::manageJobs() {
	manageLocalJobs();
	manageOutgoingJobs();
}

void CompilerService::manageLocalJobs() {
	//compiles maxThreadCount jobs at the same time.
	//if there are no more jobs to compile in the localJobQueue, get back enough jobs from the network in
	//			order to compile maxThreadCount jobs locally
	//if there are no more (own) jobs to compile at all, start to compile remoteJobs
	while (this->maxThreadCount > this->currentThreadCount && this->localJobQueue.count() > 0) {
		executeFirstJobFromList(&this->localJobQueue);
	}
	while (this->maxThreadCount > this->currentThreadCount && this->localJobQueue.count() == 0) {
		Job *job = this->network->cancelOutgoingJob();
		if (job != NULL) {
			executeJobLocally(job);
		} else {
			break;
		}
	}
	while (this->maxThreadCount > this->currentThreadCount && this->remoteJobQueue.count() > 0) {
		executeFirstJobFromList(&this->remoteJobQueue);
	}
}

void CompilerService::executeFirstJobFromList(QList<Job*> *jobList) {
		Job *job = jobList->first();
		jobList->removeFirst();
		executeJobLocally(job);
}


void CompilerService::executeJobLocally(Job* job) {
	connect(job,
		SIGNAL(finished(Job*)),
		this,
		SLOT(onLocalCompileFinished(Job*))
	);
	job->execute();
	setCurrentThreadCount(++this->currentThreadCount);
}

void CompilerService::manageOutgoingJobs() {
	//delegates all jobs until only this->maxThreadCount * 2 Jobs remain in localJobQueue
	//removes delegated Jobs from the delegatedJobQueue
	while (this->maxThreadCount < this->localJobQueue.count()) {
		Job *job = this->localJobQueue.last();
		this->localJobQueue.removeLast();
		this->delegatedJobQueue.append(job);
		network->delegateOutgoingJob(this->delegatedJobQueue.first());
		this->delegatedJobQueue.removeFirst();
	}
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
	if (this->toolChains.count() < 1) {
		qFatal("No ToolChains available!");
	}
	this->settings.endArray();
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
	if (this->maxThreadCount == -1) {
		setMaxThreadCount(-1);
	}
}

void CompilerService::saveMaxThreadCount(int count) {
	//int QThread::idealThreadCount () determines the number of cores (physically and virtually)
	int systemCount = QThread::idealThreadCount();
	this->currentThreadCount = (count >= systemCount || count < 1) ? systemCount : count;
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
	emit localJobCompilationFinished(job);
	setCurrentThreadCount(--this->currentThreadCount);
	manageJobs();
}