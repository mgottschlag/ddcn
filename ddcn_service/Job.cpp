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

#include "Job.h"
#include <QProcess>
#include <QTemporaryFile>
#include "InputOutputFilePair.h"
#include "TemporaryFile.h"

#include <cassert>

Job::Job(QStringList inputFiles, QStringList outputFiles,
		QStringList fullParameters, QStringList preprocessorParameters,
		QStringList compilerParameters,QString toolChain,
		QString workingDir, bool isRemoteJob, bool delegatable,
		const QByteArray &stdinData) :
		preProcessListPosition(0), preprocessing(false), preprocessed(false),
		compiling(false), incomingJob(NULL), outgoingJob(NULL) {
	qCritical("in: %s, param: %s, tool: %s, delegatable: %d",
			  (inputFiles.count() <= 0) ? "[no inputFiles]" : inputFiles[0].toAscii().data(),
			  (fullParameters.count() <= 0) ? "[no parameters]" : fullParameters[0].toAscii().data(),
			  toolChain.toAscii().data(), (int)this->delegatable);
	this->inputFiles = inputFiles;
	this->outputFiles = outputFiles;
	this->fullParameters = fullParameters;
	this->toolChain = toolChain;
	this->remoteJob = isRemoteJob;
	this->preprocessorParameters = preprocessorParameters;
	this->delegatable = delegatable;
	this->workingDir = workingDir;
	this->stdinData = stdinData;
}
Job::~Job() {
	// Remove temporary files created for preprocessing
	foreach (QString fileName, preprocessedFiles) {
		QFile file(fileName);
		file.remove();
	}
}

//will be called by the CompilerNetwork
void Job::preProcess() {
	QStringList preProcessParameter;
	if (this->preProcessListPosition < this->inputFiles.count()) {
		QString inputFile = this->inputFiles[this->preProcessListPosition];
		TemporaryFile tmpFile(
			inputFile.right(inputFile.length() - inputFile.lastIndexOf(".")),
			"ddcn_tmp_",
			inputFile.left(inputFile.indexOf(".")));
		preProcessParameter << "-E " << inputFile << "-o"
									<< tmpFile.getFilename()
									<< this->preprocessorParameters;
		this->preprocessedFiles.append(tmpFile.getFilename());
		gccPreProcess = new QProcess(this);
		connect(gccPreProcess,
			SIGNAL(finished(int, QProcess::ExitStatus)),
			this,
			SLOT(onPreProcessFinished(int,QProcess::ExitStatus))
		);
		connect(gccPreProcess,
			SIGNAL(error(int, QProcess::ExitStatus)),
			this,
			SLOT(onPreProcessExecuteError(QProcess::ExitStatus))
		);
		gccPreProcess->start("gcc", preProcessParameter);
		preprocessing = true;
	} else {
		preprocessing = false;
		preprocessed = true;
		emit preprocessingFinished(this);
	}
}

void Job::execute() {
	QStringList parameters;
	if (isRemoteJob()) {
		parameters = compilerParameters;
		parameters << inputFiles;
		// We do not need to set the output files here as they need to be in the
		// same directory and have the same name as the input files
		assert(outputFiles.count() == inputFiles.count());
		// TODO: Check file names in debug builds via assert
	} else {
		parameters = fullParameters;
	}
	//create a gcc process and submit the parameters
	gccProcess = new QProcess(this);
	connect(gccProcess,
		SIGNAL(finished(int, QProcess::ExitStatus)),
		this,
		SLOT(onExecuteFinished(int, QProcess::ExitStatus))
	);
	connect(gccProcess,
		SIGNAL(error(QProcess::ProcessError)),
		this,
		SLOT(onExecuteError(QProcess::ProcessError))
	);
	//TODO DEBUG:qCritical("Start Compiling");
	gccProcess->setWorkingDirectory(this->workingDir);
	gccProcess->setProcessChannelMode(QProcess::SeparateChannels);
	// TODO: Toolchain path
	gccProcess->start("gcc", parameters);
	gccProcess->write(stdinData);
	gccProcess->closeWriteChannel();
	compiling = true;
}

void Job::onExecuteFinished(int exitCode, QProcess::ExitStatus exitStatus) {
	compiling = false;
	qCritical("Execute finished: %d", gccProcess->exitCode());
	jobResult.stdout = gccProcess->readAllStandardOutput();
	jobResult.stderr = gccProcess->readAllStandardError();
	jobResult.returnValue = exitCode;
	emit finished(this);
}

void Job::onExecuteError(QProcess::ProcessError error) {
	compiling = false;

	jobResult.stdout = gccProcess->readAllStandardOutput();
	jobResult.stderr = gccProcess->readAllStandardError();
	jobResult.stderr.append((QString("\n") + getQProcessErrorDescription(error)).toAscii());
	jobResult.returnValue = -1; //= Error!

	emit finished(this);
}

void Job::onPreProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	preprocessingResult.stdout.append(gccPreProcess->readAllStandardOutput());
	preprocessingResult.stderr.append(gccPreProcess->readAllStandardError());
	preprocessingResult.returnValue = exitCode;
	if (exitCode != 0) {
		// Do not continue if preprocessing failed
		emit preprocessingFinished(this);
	} else {
		this->preProcessListPosition++;
		this->preProcess();
	}
}

void Job::onPreProcessExecuteError(QProcess::ProcessError error) {
	preprocessingResult.stdout.append(gccPreProcess->readAllStandardOutput());
	preprocessingResult.stderr.append(gccPreProcess->readAllStandardError());
	preprocessingResult.returnValue = -1;
	// Do not continue if preprocessing failed
	emit preprocessingFinished(this);
}

void Job::setFinished(int returnValue, const QByteArray &stdout, const QByteArray &stderr) {
	jobResult.stdout = stdout;
	jobResult.stderr = stderr;
	jobResult.returnValue = returnValue;
	emit finished(this);
}

QString Job::getQProcessErrorDescription(QProcess::ProcessError error) {
	//finished signal: finished(bool executed, int resultValue, QString consoleOutput, QList<QByteArray> outputFiles);
	QString errorString = "Error: An unresolveable error occured.\n"; //TODO
	switch(error) {
		case 0: errorString = "Error: Could not start gcc process.\n"; break;
		case 1: errorString = "Error: gcc process started successfully but crashed.\n"; break;
		case 2: errorString = "Error: gcc process timed out.\n"; break;
		case 3: errorString = "Error: An error occurred when attempting to write to the process.\n"; break;
		case 4: errorString = "Error: An error occurred when attempting to read from the process.\n"; break;
		default: errorString = "Error: An unresolveable unknown error occured.\n";
	}
	return errorString;
}
