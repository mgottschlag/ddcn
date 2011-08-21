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

#ifndef COMPILERSERVICEADAPTOR_H_INCLUDED
#define COMPILERSERVICEADAPTOR_H_INCLUDED

#include "CompilerService.h"
#include "Job.h"
#include <QList>
#include <QDBusAbstractAdaptor>
#include <QStringList>
#include <QDBusMessage>
#include "DBusStructs.h"

/**
 * Class which exports a CompilerService object.
 */
class CompilerServiceAdaptor : public QDBusAbstractAdaptor {
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.ddcn.CompilerService")
public:
	/**
	 * Creates a dbus adaptor for the compiler service.
	 * @param service Compiler service object to export.
	 */
	explicit CompilerServiceAdaptor(CompilerService *service);
public slots:
	/**
	 * Sets the maximum number of Threads strated on this machine.
	 * @param threadCount the number of threads run on the local machine.
	 */
	void setMaxThreadCount(int threadCount);
	/**
	 * Returns the number of currently running Threads.
	 * @return the number of currently running Threads.
	 */
	int getCurrentThreadCount();
	/**
	 * Returns the maximum number of Threads that can be run on this machine.
	 * @return the maximum number of Threads that can be run on this machine.
	 */
	int getMaxThreadCount();
	/**
	 * Ends the running service.
	 * Running or waiting (enqueued) jobs will be aborted.
	 */
	void shutdown();
	/**
	 * Returns the list of ToolChains supported by this peer.
	 * @param the list of ToolChains supported by this peer.
	 */
	QList<ToolChainInfo> getToolChains();
	/**
	 * Delegates the execution of a job to the CompilerService and returns its compiled result.
	 * The result will be returned delayed via dbus.
	 * @param parameters the parameters that will be passed on to the gcc/g++ compiler.
	 * @param toolChain the toolChain that will used for compiling.
	 * @param workingPath the direcotry in which the compiler was executed and the output files will be stored.
	 * @param stdinData input parameters from the terminal.
	 * @param language the programming language of the code to be compiled.
	 * @param message dbus message
	 * @return the result of an executed job.
	 */
	JobResult executeJob(QStringList parameters, QString toolChain,
		QString workingPath, const QByteArray &stdinData,
		QString language, const QDBusMessage &message);

	/**
	 * Adds a ToolChain to the list of supported ToolChains if the given path is valid.
	 * The given file will be executed if possible to determine the ToolChain version.
	 * If the path could be executed the mnethod will return true.
	 * @param path the path to execute.
	 * @return true if the path could have been executed and the ToolChain be added.
	 */
	bool addToolChain(QString path);
	/**
	 * Removes a given ToolChain from the list of supported ToolChains and returns true if successful.
	 * @param path the ToolChain that should be removed.
	 * @return true if the given ToolChain could be removed successfully.
	 */
	bool removeToolChain(QString path);
	/**
	 * Returns the number of jobs in the queue that will be compiled locally.
	 * @return the number of jobs in the queue that will be compiled locally.
	 */
	int getNumberOfJobsInLocalQueue();
	/**
	 * Return the number of jobs that have been accepted from other peers.
	 * @return the number of jobs that have been accepted from other peers.
	 */
	int getNumberOfJobsInRemoteQueue();
	/**
	 * Clears the log file.
	 */
	void clearLog();
private slots:
	/**
	 * Called when the number of currently running threads changes.
	 * @param threadCount the number of threads that are currently running.
	 */
	void onCurrentThreadCountChanged(int threadCount);
	/**
	 * Called when the number of maximum allowed threads on this peer changes.
	 * @param threadCount the number of threads to be run maximum on this peer.
	 */
	void onMaxThreadCountChanged(int threadCount);
	/**
	 * Called when the number of jobs in the local queue changes.
	 * @param noj the number of jobs in the local job queue.
	 */
	void onNumberOfJobsInLocalQueueChanged(int noj);
	/**
	 * Called when the number of jobs in the remote job queue changes.
	 * @param noj the number of jobs in the remote job queue.
	 */
	void onNumberOfJobsInRemoteQueueChanged(int noj);
	/**
	 * Called when a ToolChain in the list of supported ToolChains changes.
	 */
	void onToolChainsChanged();
	/**
	 * Called when shoutdown is requested.
	 * @see shoutdown()
	 */
	void requestShutdown();
	/**
	 * Called when compiling of a job has been finished.
	 * @param job the job that has been compiled.
	 */
	void localCompilationJobFinished(Job *job);
signals:
	/**
	 * Triggered when the number of currently running threads changes.
	 * @param threadCount the number of currently running threads.
	 */
	void currentThreadCountChanged(int threadCount);
	/**
	 * Triggered when the number maximum allowed threads on this peer changes.
	 * @param the number maximum allowed threads on this peer.
	 */
	void maxThreadCountChanged(int threadCount);
	/**
	 * Triggered when shoutdown has been rewuested.
	 */
	void onShutdownRequest();
	/**
	 * Triggered  when the number of jobs in the local queue changes.
	 * @param noj the number of jobs in the local queue changes.
	 */
	void numberOfJobsInLocalQueueChanged(int noj);
	/**
	 * Triggered when the number of jobs in the remote job queue changes.
	 * @param noj the number of jobs in the remote job queue.
	 */
	void numberOfJobsInRemoteQueueChanged(int noj);
	/**
	 * Triggered when the lit of supported ToolChains has changed.
	 * @return the updated list of ToolChains.
	 */
	void toolChainsChanged(QList<ToolChainInfo> toolChains);
private:
	/**
	 * Converts a given ToolChain to a dbus compatible ToolChainInfo struct.
	 * @param toolChain the ToolChain to convert.
	 * @return the converted ToolChainInfo struct.
	 */
	ToolChainInfo toToolChainInfo(ToolChain toolChain);
	CompilerService *service;
	QMap<Job*, QDBusMessage*> jobDBusMessageMap;
};

#endif

