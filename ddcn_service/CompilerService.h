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

#ifndef COMPILERSERVICE_H_INCLUDED
#define COMPILERSERVICE_H_INCLUDED

#include "Job.h"
#include "ToolChain.h"
#include "JobRequest.h"
#include "CompilerNetwork.h"
#include <QList>
#include <QObject>
#include <QSettings>
#include <QThread>

using namespace std;

/**
 * The CompilerService manages, executes and delegates compiler jobs.
 */
class CompilerService : public QObject {
    Q_OBJECT
public:
	/**
	 * Creates a new instance of the CompilerService.
	 * The CompilerService manages, executes and delegates compiler jobs.
	 * @param network A pointer on an instance of the CompilerNetwork.
	 */
    CompilerService(CompilerNetwork *network);

	/**
	 * Adds a Job to the list of jobs. The CompilerService will automatically decide whether to append the job to the List
	 * of jobs created localy or to the list of jobs created remote.
	 * @param job The job to append to the list.
	 */
    void addJob(Job *job);

	/**
	 * Returns a list of ToolChains.
	 * @return the list of ToolChains.
	 */
    QList<ToolChain> *getToolChains() {
        return &this->toolChains;
    }

	/**
	 * Returns the number of threads currently running on this machine.
	 * @return the number of threads currently running on this machine.
	 */
    int getCurrentThreadCount() {
        return this->currentThreadCount;
    }

	/**
	 * Returns the maximum number of threads that will be executed on this machine.
	 * @return  the maximum number of threads that will be executed on this machine.
	 */
    int getMaxThreadCount() {
       return this->maxThreadCount;
    }

    /**
	 * Returns true if a given ToolChain could be added to the list of ToolChains.
	 * If the ToolChain already exists in the set of ToolChains administrated, the ToolChain will not be added.
	 * In case the given path can not be executed, the ToolChain will also not be added.
	 * @return true if a given ToolChain could be added to the list of ToolChains.
	 * @param path The path to the ToolChain (compiler) to add.
	 * @emit toolChainsChanged
	 */
    bool addToolChain(QString path) {
        ToolChain toolChain(path);
		if (toolChain.getPath() == "") {
			return false;
		}
		path.replace("*", "gcc");
        if (this->toolChains.indexOf(toolChain) < 0 && QFile(path).exists()) {
            this->toolChains.append(toolChain);
            saveToolChains();
        }
        network->setToolChains(toolChains);
        emit toolChainsChanged();
		return true;
    }

	/**
	 * Sets the maximum number of threads executed on this machine to the given value and saves it.
	 * @param count the number of threads to be run on this machine.
	 * @emit maxThreadCountChanged
	 */
	void setMaxThreadCount(int count) {
		this->saveMaxThreadCount(count);
		network->setFreeLocalSlots(computeFreeLocalSlotCount());
		network->updateStatistics(maxThreadCount, currentThreadCount);
		emit maxThreadCountChanged(this->maxThreadCount);
	}

	/**
	 * Returns true if the given ToolChain (identified by path) could be removed from the list of
	 * administrated ToolChains.
	 * @return true if the ToolChain could be removed.
	 * @emit toolChainsChanged
	 */
    bool removeToolChain(QString path) {
        ToolChain toolChain(path);
		bool returnValue = false;
        if (this->toolChains.removeOne(toolChain)) {
            saveToolChains();
			returnValue = true;
        }
        emit toolChainsChanged();
		return returnValue;
    }

	int getNumberOfJobsInLocalQueue() {
		return localJobQueue.count();
	}
	int getNumberOfJobsInRemoteQueue() {
		return localJobQueue.count();
	}

public slots:
	/**
	 * Adds a job to the list of remote jobs.
	 * Uses the addJob(Job *job) method.
	 * @param job the job to be added.
	 */
    void onIncomingJob(Job *job);

    /**Removes the job from the local queue if it has been executed successfully.
     * Otherwise, moves the job to the end of the list in order to execute it again.
     */
signals:
	/**
	 * Triggered when the number of threads currently run on this machine changed.
	 * @param currentThreadCount the number of threads currently executed.
	 */
    void currentThreadCountChanged(int currentThreadCount);

	/**
	 * Triggered in case the number of threads maximum run on this machine chanded.
	 * @param maxThreadCount number of threads maximum executed on this machine
	 */
    void maxThreadCountChanged(int maxThreadCount);

	/**
	 * Triggered if a local compiling process finished.
	 * @param job the finished job.
	 */
	void localJobCompilationFinished(Job *job);

	/**
	 * Triggered whenever the number of jobs waiting in the local queue changes.
	 * @param noj number of jobs in the local queue.
	 */
	void numberOfJobsInLocalQueueChanged(int noj);

	/**
	 * Triggered whenever the number of jobs waiting in the remote queue changes.
	 * @param noj number of jobs in the local queue.
	 */
	void numberOfJobsInRemoteQueueChanged(int noj);

	/**
	 * Triggered when a toolchain is added or removed to or from the list of toolchains.
	 */
	void toolChainsChanged();
private slots:
	/**
	 */
	void onReceivedJob(Job *job);
	void onIncomingJobAborted(Job *job);
    	void onLocalCompileFinished(Job *job);
    	void onRemoteCompileFinished(Job *job);
	void onOutgoingJobCancelled(Job *job);
private:
	/**
	 * Returns true if the given job could be removed from the list successfully.
	 * @return true if the given job could be removed from the list successfully.
	 */
    bool removeJob(Job *job);

	/**
	 * Manages the jobs in the local and remote list.
	 */
    void manageJobs();

	/**
	 * Manages the jobs in the local job queue.
	 * Starts to compile maxThreadCount jobs at the same time.
	 * If there are no more jobs to compile in the localJobQueue, the method gets back enough jobs from the network in
	 * order to compile maxThreadCount jobs locally.
	 * if there are no more (own) jobs to compile at all, remote jobs that came over the network will be compiled.
	 */
    void manageLocalJobs();

	/**
	 * Delegates jobs to the network until only this->maxThreadCount * 2 Jobs remain in local queue.
	 * Delegated jobs will be removed from the job queue.
	 */
    void manageOutgoingJobs();

	/**
	 * Loads the toolchains saved in the configuration file.
	 */
    void loadToolChains();

	/**
	 * Saves the toolchains in the configuration files.
	 */
    void saveToolChains();

	/**
	 * Executes (Compiles) thefirst job from the given job list.
	 * @param jobList the list containing the jobs to compile.
	 */
	void executeFirstJobFromList(QList<Job*> *jobList);

	/**
	 * Executes a given job on the local machine.
	 * @param job the job to execute.
	 */
    void executeJobLocally(Job *job);

	/**
	 * Checks if the given toolchain is available in the list of toolchains.
	 * @param target the toolchain to find localy.
	 */
    bool isToolChainAvailable(ToolChain target);

	/**
	 * Saves the given maximum number of jobs executed on the local machine.
	 * @param count maximum number of jobs to be executed on the local machine.
	 */
	void saveMaxThreadCount(int count);

	/**
	 * Sets the current number of threads to the given count.
	 * @param count the current number of running threads.
	 */
    void setCurrentThreadCount(int count) {
        this->currentThreadCount = count;
		network->updateStatistics(maxThreadCount, currentThreadCount);
        emit currentThreadCountChanged(this->currentThreadCount);
    }

    /**
	 * Loads the number of processes to use from the settings file, or if this
	 * fails, computes it from the number of cores.
	 */
    void loadMaxThreadCount();

	/**
	 * Returns the number of free slots.
	 * @return the number of free slots.
	 */
	unsigned int computeFreeLocalSlotCount();

	/**
	 * Returns a job that can be delegated to the network.
	 * @return a job that can be delegated to the network.
	 */
	Job *extractLocalDelegatableJob();

    int currentThreadCount;
    int maxThreadCount;
    QList<ToolChain> toolChains;
    CompilerNetwork *network;
    QList<Job*> localJobQueue;
    QList<Job*> remoteJobQueue;
	QSettings settings;
	static QString settingToolChains;
	static QString settingToolChainPath;
	static QString settingToolChainVersion;
	static QString settingMaxThreadCount;
};

#endif

