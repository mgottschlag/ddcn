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
 * Class which manages, executes and delegates compiler jobs.
 */
class CompilerService : public QObject {
    Q_OBJECT
public:
    CompilerService(CompilerNetwork *network);
    void addJob(Job *job);

    QList<ToolChain> *getToolChains() {
		qCritical("GetToolchains, %d", this->toolChains.count());
        return &this->toolChains;
    }

    int getCurrentThreadCount() {
        return this->currentThreadCount;
    }

    int getMaxThreadCount() {
       // return this->maxThreadCount;
       return QThread::idealThreadCount();
    }

    void addToolChain(QString path) {
        ToolChain toolChain(path);
        if (this->toolChains.indexOf(toolChain) >= 0 && QFile(path).exists()) {
            this->toolChains.append(toolChain);
            saveToolChains();
        }
        network->setToolChains(toolChains);
        emit toolChainsChanged();
    }

	void setMaxThreadCount(int count) {
		this->saveMaxThreadCount(count);
		network->setFreeLocalSlots(computeFreeLocalSlotCount());
		emit maxThreadCountChanged(this->maxThreadCount);
	}

    void removeToolChain(QString path) {
        ToolChain toolChain(path);
        if (this->toolChains.removeOne(toolChain)) {
            saveToolChains();
        }
        emit toolChainsChanged();
    }
public slots:
    void onIncomingJob(Job *job);
    /**Removes the job from the local queue if it has been executed successfully.
     * Otherwise, moves the job to the end of the list in order to execute it again.
     */
signals:
    void currentThreadCountChanged(int currentThreadCount);
    void maxThreadCountChanged(int maxThreadCount);
	void localJobCompilationFinished(Job *job);
	void numberOfJobsInLocalQueueChanged(int noj);
	void numberOfJobsInRemoteQueueChanged(int noj);
	void toolChainsChanged();
private slots:
	void onReceivedJob(Job *job);
	void onRemoteJobAborted(Job *job);
    	void onLocalCompileFinished(Job *job);
    	void onRemoteCompileFinished(Job *job);
private:
    bool removeJob(Job *job);
    void manageJobs();
    void manageLocalJobs();
    void manageOutgoingJobs();
    void loadToolChains();
    void saveToolChains();
	void executeFirstJobFromList(QList<Job*> *jobList);
    void executeJobLocally(Job *job);
    bool isToolChainAvailable(ToolChain target);
	void saveMaxThreadCount(int count);
    void setCurrentThreadCount(int count) {
        this->currentThreadCount = count;
        emit currentThreadCountChanged(this->currentThreadCount);
    }
    void determineAndSetMaxThreadCount();

	unsigned int computeFreeLocalSlotCount();

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

