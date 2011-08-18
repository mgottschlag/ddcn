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
	/**
	 * Constructs a new job object.
	 * @param inputFiles a list of files to compile.
	 * @param outputFiles a list of files used as a target of the compiling process.
	 * @param fullParameters the complete list of parameters used to call ddcn.
	 * @param preprocessorParameters the list of parameters concerning the preprocessing of the job.
	 * @param compilerParameters the list of parameters concerning the compiling process.
	 * @param toolChain the ToolChain used for the job.
	 * @param workingDir the directory in which the job has been started.
	 * @param isRemoteJob true if the job has been accepted from the network.
	 * @param delegatable true if the job can be delegated (not only preprocessor parameters
	 * and not comipling and linking).
	 * @param stdinData data that has been transfered to ddcn via standard input.
	 * @param language the programming language used for the code of the job.
	 */
	Job(QStringList inputFiles, QStringList outputFiles,
		QStringList fullParameters, QStringList preprocessorParameters,
		QStringList compilerParameters, ToolChain toolChain, QString workingDir,
		bool isRemoteJob, bool delegatable, const QByteArray &stdinData,
		QString language);
	/**
	 * Destructor of the job.
	 */
	 ~Job();

	/**
	 * Returns true if the job is delegatable.
	 * @return true if the job is delegatable.
	 */
	bool isRemoteJob() {
		return this->remoteJob;
	}

	
	void setRemoteJob(bool isRemoteJob) {
		this->remoteJob = isRemoteJob;
	}

	/**
	 * Returns the result of the job as JobResult object.
	 * This method has to be called after recieving the finished signal.
	 * @return the result of the job as JobResult object.
	JobResult getJobResult() {
		return this->jobResult;
	}

	/**
	 * Returns the result of the preprocessing.
	 * This method has to be called after checking the state of the operation using the wasPreprocessed() method.
	 * Additionally, the signal preprocessingFinished will be triggered after finishing the process.
	 * @return the result of the preprocessing.
	 */
	JobResult getPreprocessingResult() {
		return this->preprocessingResult;
	}

	/**
	 * Returns true if the job is delegatable.
	 * @return true if the job is delegatable.
	 */
	bool isDelegatable() {
		return this->delegatable;
	}

	/**
	 * Executes this job.
	 * The signal finished will be triggered after finishing the compiling process.
	 */
	void execute();

	/**
	 * Preprocesses this job.
	 * The signal preprocessingFinished will be triggered after finishing the preprocessing.
	 */
	void preProcess();

	/**
	 * Returns true if the job is currently being preprocessed.
	 * @return true if the job is currently being preprocessed.
	 */
	bool isPreprocessing() {
		return preprocessing;
	}

	/**
	 * Returns true if the job is currently being compiled.
	 * @return true if the job is currently being compiled.
	 */
	bool isCompiling() {
		return compiling;
	}

	/**
	 * Returns true if the job has been preprocessed successfully.
	 * @return true if the job has been preprocessed successfully.
	 */
	bool wasPreprocessed() {
		return preprocessed;
	}

	/**
	 * Returns true if the job has been delegated to the network.
	 * @return true if the job has been delegated to the network.
	 */
	bool wasDelegated() {
		return delegated;
	}

	/**
	 * Associates this job with the information (such as id, source peer, etc.) used
	 * to identify the job in the network.
	 * @param outgoingJob the OutgoingJob object containing the information used for the
	 * identification of this job in the network.
	 */
	void setOutgoingJob(OutgoingJob *outgoingJob) {
		this->outgoingJob = outgoingJob;
	}

	/**
	 * Returns the outgoingJob object if any.
	 * @return the outgoingJob object if any.
	 */
	OutgoingJob *getOutgoingJob() {
		return outgoingJob;
	}

	/**
	 * Associates an incomingJob object with this job for identification in the network.
	 * @param incomingJob the incomingJob to associate with this Job.
	 */
	void setIncomingJob(IncomingJob *incomingJob) {
		this->incomingJob = incomingJob;
	}

	/**
	 * Returns the incomingJob object associated with this job.
	 * @return the incomingJob object associated with this job.
	 */
	IncomingJob *getIncomingJob() {
		return incomingJob;
	}

	/**
	 * Returns the list of preprocessed files.
	 * @return the list of preprocessed files.
	 */
	QStringList getPreprocessedFiles() {
		return preprocessedFiles;
	}

	/**
	 * Returns the list of input files.
	 * @return the list of input files.
	 */
	QStringList getInputFiles() {
		return outputFiles;
	}

	/**
	 * Returns the list of written files.
	 * @return the list of written files.
	 */
	QStringList getOutputFiles() {
		return outputFiles;
	}

	/**
	 * Returns the list of compiler parameters.
	 * @return the list of compiler parameters.
	 */
	QStringList getCompilerParameters() {
		return compilerParameters;
	}

	/**
	 * Returns the ToolChain used to compile this job.
	 * @return the ToolChain used to compile this job.
	 */
	ToolChain getToolchain() {
		return toolChain;
	}

	/**
	 * Returns the working directory in which the job was started in.
	 * @return the working directory in which the job was started in.
	 */
	QString getWorkingDirectory() {
		return workingDir;
	}

	/**
	 * Returns the programming language in which the files of this job will be compiled in.
	 * @return the programming language in which the files of this job will be compiled in.
	 */
	QString getLanguage() {
		return language;
	}

	/**
	 * Used in case the job has been compiled remote.
	 * This method will write the results of the compiling process in this job and trigger the finished signal.
	 * @param returnValue the return value of the process.
	 * @param stdout the output of the process.
	 * @param stderr the errors occured while executing the job.
	 */
	void setFinished(int returnValue, const QByteArray &stdout, const QByteArray &stderr);
signals:
	/**
	 * Triggered when the job has been compiled.
	 * @param job this Job.
	 */
	void finished(Job *job);

	/**
	 * Triggered when the preprocessing has finished.
	 * @param job this Job.
	 */
	void preprocessingFinished(Job *job);
private slots:
	/**
	 * Called when the preprocessing has finished.
	 * @param exitCode the status code returned by the spawned process.
	 * @param exitStatus the exit status of the spawned process.
	 */
	void onPreProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

	/**
	 * Called when an error has occured while preprocessing.
	 * @param error the error that has occured.
	 */
	void onPreProcessExecuteError(QProcess::ProcessError error);

	/**
	 * Called when the compiling has finished.
	 * @param exitCode the status code returned by the spawned process.
	 * @param exitStatus the exit status of the spawned process.
	 */
	void onExecuteFinished(int exitCode, QProcess::ExitStatus exitStatus);

	/**
	 * Called when an error has occured while compiling.
	 * @param error the error that has occured.
	 */
	void onExecuteError(QProcess::ProcessError error);
private:
	/**
	 * Returns an error message according to the given QProcess::ProcessError.
	 * @param error the error that might has occured while trying to execute this Job.
	 */
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
	IncomingJob *incomingJob;
	OutgoingJob *outgoingJob;
};

#endif

