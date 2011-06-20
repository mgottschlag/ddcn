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

#ifndef JOB_H_INCLUDED
#define JOB_H_INCLUDED

#include <QObject>
#include <QStringList>
#include <QProcess>
#include "InputOutputFilePair.h"

struct JobResult {
	QString consoleOutput;
	int returnValue;
};




/**
 * Class which contains a single compiler job which has either been received
 * from the network or produced locally.
 */
class Job : public QObject {
	Q_OBJECT
public:
	Job(QStringList inputFiles, QStringList parameters, QString toolChain,
		bool isRemoteJob);
	bool isRemoteJob() {
		return this->remoteJob;
	}
	void setRemoteJob(bool isRemoteJob) {
		this->remoteJob = isRemoteJob;
	}
	JobResult getJobResult() {
		return this->jobResult;
	}
	void execute();
	void preProcess();
signals:
	void finished(Job *job);
	void preProcessFinished(Job *job);
private:
	QString getQProcessErrorDescription(QProcess::ProcessError error);
	QString consoleOutput;
	QStringList inputFiles;
	QStringList outputFiles;
	QStringList preProcessedFiles;
	QStringList parameters;
	QString toolChain;
	JobResult jobResult;
	JobResult preProcessResult;
	QProcess *gccPreProcess;
	int preProcessListPosition;
	QProcess *gccProcess;
	bool remoteJob;
private slots:
	void onPreProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void onPreProcessExecuteError(QProcess::ProcessError error);
	void onExecuteFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void onExecuteError(QProcess::ProcessError error);
};

#endif

