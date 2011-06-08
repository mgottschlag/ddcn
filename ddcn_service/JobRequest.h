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

#ifndef JOBREQUEST_H_
#define JOBREQUEST_H_

#include "OnlinePeer.h"
#include "Job.h"
#include "ToolChain.h"

#include <QList>


class JobRequest{

public:
	/**
	 * Creates a new JobRequest.
	 * @param sourcePeer the OnlinePeer that is responsible (that created the request).
	 * @param maxNumberOfAcceptedJobs the maximum number of jobs the peer accepts.
	 */
	JobRequest(OnlinePeer *sourcePeer, int maxNumberOfAcceptedJobs);
	/**
	 * Returns the target peer of the request.
	 * @return the target peer of the request.
	 */
	OnlinePeer *getTargetPeer();
	/**
	 * Returns the source peer of the request.
	 * @return the source peer of the request.
	 */
	OnlinePeer *getSourcePeer();
	/**
	 * Returns the number of jobs accepted by this peer.
	 * @return the number of jobs accepted by this peer.
	 */
	int getNumberOfAcceptedJobs();
	/**
	 * Returns the List of jobs.
	 * @return the List of jobs.
	 */
	QList<Job*> getJobList();
	/**
	 * Returns the Tool chain for this jobs.
	 * @return the Tool chain for this jobs.
	 */
	ToolChain *getToolChain();
	/**
	 * Sets the target peer where to send this request to.
	 * @param targetPeer  the target peer where to send this request to.
	 */
	void setTargetPeer(OnlinePeer *targetPeer);
	/**
	 * Sets the job list to pass with this job request.
	 * If a job list with a number of entries greater than the number of maximum accepted requests is entered,
	 * only the first maxNumberOfAcceptedJobs Jobs are accepted.
	 * @param jobList the job list to pass with this job request.
	 * @return true, if the job list has been added.
	 */
	bool addJobList(QList<Job*> jobList);
	/**
	 * Adds a given job to the job list and returns the success of the operation.
	 * Adding a job will fail, if the number of jobs in the list already reached maxNumberOfAcceptedJobs.
	 * @param the job to add to the list.
	 * @return true if the job has been added.
	 */
	bool addJob(Job *job);

private:
	OnlinePeer *targetPeer;
	OnlinePeer *sourcePeer;
	ToolChain *toolChain;
	int maxNumberOfAcceptedJobs;
	QList<Job*> jobList;
};

#endif /* JOBREQUEST_H_ */
