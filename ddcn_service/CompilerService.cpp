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
}

void CompilerService::addJob(Job *job) {
	if (job->isRemoteJob())
		this->localJobQueue.append(job);
	else
		this->remoteJobQueue.append(job);
	manageJobs();
}

void CompilerService::manageJobs() {
	//TODO in what case do jobs have to be distributed in the network?
	/*if (getThreadCount() > this->localJobQueue.count()) {
		while (network->canAcceptOutgoingJobRequest()) {
			Job *job = this->localJobQueue.last();
			if (network->delegateOutgoingJob(job)) {
				removeJob(job);
				this->delegatedJobQueue.append(job);
			} else {
				// TODO: Move the job to the beginning of the list as other jobs
				// might be possible to be delegated
			}
		}
	}*/
}

void CompilerService::findToolChains() {
	//TODO
	//Als erstes ein Array mit den vorhanden Versionen erstellen ls + grep?
	//mit den vorhandenen ToolChain Objekten vergleichen und gegebenfalls einfügen
	//nicht mehr vorhandene GCC Versionen aus der Liste der ToolChainObjekte löschen...
	//oder einfach direkt bei jedem Start von ddcn eine neue Liste der vorhandenen GVV
	//Versionen anlegen?
	//print 'system('/usr/bin |grep gcc')';
}
bool CompilerService::isToolChainAvailable(ToolChain target) {
	foreach (ToolChain toolChainListObjekt, availableToolChains) {
		if (toolChainListObjekt == target) {
			return true;
		}
	}
	return false;
}

void CompilerService::onIncomingJob(Job *job) {
	addJob(job);
}

void CompilerService::onIncomingJobRequest(JobRequest *request) {
	//Accept Jobs if local queue is empty or if number of jobs in the local queue is less than the available threads.
	if (this->localJobQueue.count() < this->getThreadCount()) {

	}
}


bool CompilerService::removeJob(Job *job) {
	//Tries to locate a given Job in the local job queue or in the remote job queue and removes it,
	//otherwise return false.
	return (this->localJobQueue.removeOne(job)) ?  true : (this->remoteJobQueue.removeOne(job));
}

void CompilerService::spareResourcesInNetwork() {
	// TODO
}
void CompilerService::requestAccepted(JobRequest *request) {
	// TODO
}
