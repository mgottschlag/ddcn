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

#ifndef JOB_H_INCLUDED
#define JOB_H_INCLUDED

#include <QObject>
#include <QStringList>
#include <QProcess>
#include "InputOutputFilePair.h"
#include "ToolChain.h"

struct JobResult {
	QByteArray stdout;
	QByteArray stderr;
	int returnValue;
};


class OutgoingJob;
class IncomingJob;

/**
 * Class which contains a single compiler job which has either been received
 * from the network or produced locally.
 */
class Job : public QObject {
	Q_OBJECT
public:
	Job(QStringList inputFiles, QStringList outputFiles,
		QStringList fullParameters, QStringList preprocessorParameters,
		QStringList compilerParameters, ToolChain toolChain, QString workingDir,
		bool isRemoteJob, bool delegatable, const QByteArray &stdinData,
		QString language);
	~Job();

	bool isRemoteJob() {
		return this->remoteJob;
	}
	void setRemoteJob(bool isRemoteJob) {
		this->remoteJob = isRemoteJob;
	}
	JobResult getJobResult() {
		return this->jobResult;
	}
	JobResult getPreprocessingResult() {
		return this->preprocessingResult
;
	}
	bool isDelegatable() {
		return this->delegatable;
	}
	void execute();
	void preProcess();
	bool isPreprocessing() {
		return preprocessing;
	}
	bool isCompiling() {
		return compiling;
	}
	bool wasPreprocessed() {
		return preprocessed;
	}

	bool wasDelegated() {
		return delegated;
	}

	void setOutgoingJob(OutgoingJob *outgoingJob) {
		this->outgoingJob = outgoingJob;
	}
	OutgoingJob *getOutgoingJob() {
		return outgoingJob;
	}
	void setIncomingJob(IncomingJob *incomingJob) {
		this->incomingJob = incomingJob;
	}
	IncomingJob *getIncomingJob() {
		return incomingJob;
	}

	QStringList getPreprocessedFiles() {
		return preprocessedFiles;
	}
	QStringList getInputFiles() {
		return outputFiles;
	}
	QStringList getOutputFiles() {
		return outputFiles;
	}

	QStringList getCompilerParameters() {
		return compilerParameters;
	}
	ToolChain getToolchain() {
		return toolChain;
	}
	QString getWorkingDirectory() {
		return workingDir;
	}

	QString getLanguage() {
		return language;
	}

	void setFinished(int returnValue, const QByteArray &stdout, const QByteArray &stderr);
signals:
	void finished(Job *job);
	void preprocessingFinished(Job *job);
private:
	QString getQProcessErrorDescription(QProcess::ProcessError error);
	QByteArray compilerStdout;
	QByteArray compilerStderr;
	QStringList inputFiles;
	QStringList outputFiles;
	bool delegatable;
	QString language;

	QStringList fullParameters;
	QStringList preprocessorParameters;
	QStringList compilerParameters;

	QByteArray stdinData;

	QStringList preprocessedFiles;
	ToolChain toolChain;
	QString workingDir;
	JobResult jobResult;
	JobResult preprocessingResult;
	QProcess *gccPreProcess;
	int preProcessListPosition;
	QProcess *gccProcess;
	bool remoteJob;

	bool preprocessing;
	bool preprocessed;
	bool compiling;

	bool delegated;
private slots:
	void onPreProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void onPreProcessExecuteError(QProcess::ProcessError error);
	void onExecuteFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void onExecuteError(QProcess::ProcessError error);
private:
	IncomingJob *incomingJob;
	OutgoingJob *outgoingJob;
};

#endif

