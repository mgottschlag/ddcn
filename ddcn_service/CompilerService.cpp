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

CompilerService::CompilerService(CompilerNetwork *network) : threadCount(1) {
	this->network = network;
	//TODO Toolchains laden
}

void CompilerService::addJob(Job *job) {
	if (job->isRemoteJob())
		this->localJobQueue.append(job);
	else
		this->remoteJobQueue.append(job);
	manageJobs();
}

//SLOTS
void CompilerService::onIncomingJob(Job *job) {
	addJob(job);
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
	//TODO in what case do jobs have to be distributed in the network?
	if (this->getThreadCount() < this->localJobQueue.count()) {
		Job *job = this->localJobQueue.last();
		this->localJobQueue.removeLast();
		this->delegatedJobQueue.append(job);
		network->delegateOutgoingJob(job);
	}
}

void CompilerService::loadToolChains() {
	QSettings settings("ddcn", "ddcn");
	int size = settings.beginReadArray("toolChains");
	for (int i = 0; i < size; ++i) {
		settings.setArrayIndex(i);
		QString version = settings.value("version").toString();
		QString path = settings.value("path").toString();
		if (QFile(path).exists()) {
			ToolChain toolChain(version, path);
			this->toolChains.append(toolChain);
		}
	}
	settings.endArray();
}

void CompilerService::saveToolChains()  {
	QSettings settings("ddcn", "ddcn");
	settings.beginWriteArray("toolChans");
	for (int i = 0; i < this->toolChains.size(); ++i) {
		settings.setArrayIndex(i);
		settings.setValue("version", this->toolChains.at(i).getVersion());
		settings.setValue("path", this->toolChains.at(i).getPath());
	}
	settings.endArray();
}

bool CompilerService::isToolChainAvailable(ToolChain target) {
	foreach (ToolChain toolChainListObjekt, toolChains) {
		if (toolChainListObjekt == target) {
			return true;
		}
	}
	return false;
}
