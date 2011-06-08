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

#include <QTemporaryFile>
#include <QProcess>

Job::Job(QObject *parent) : QObject(parent), arguments(NULL) {
	// TODO
}
Job::~Job() {
	if (arguments) {
		delete arguments;
	}
}

void Job::initialize(int argc, char **argv) {
	arguments = new CommandLineParser(argc, argv);
}

bool Job::isRemoteExecutionPossible() {
	return arguments->isRemoteExecutionPossible();
}
QStringList Job::getRemoteParameters() {
	return arguments->getRemoteParameters();
}

void Job::executeLocally() {
	// Simply call GCC with the original parameters
	QStringList parameters = arguments->getOriginalParameters();
	int returnValue = QProcess::execute("gcc", parameters);
	emit finished(returnValue);
}
int Job::preprocess() {
	QStringList baseArguments = arguments->getPreprocessingParameters();
	QStringList inputFiles = this->arguments->getInputFiles();
	// We have to store the output of preprocessing into temporary files
	QList<QTemporaryFile*> tempOutput;
	for (int i = 0; i < inputFiles.count(); i++) {
		QString inputFile = inputFiles[i];
		QTemporaryFile *tempFile = new QTemporaryFile("ddcn_gcc");
		tempFile->setAutoRemove(true);
		if (!tempFile->open()) {
			qCritical("Could not open temporary file for preprocessing.");
			return -1;
		}
		tempFile->close();
		tempOutput.append(tempFile);
	}
	// Run the preprocessor over all files
	// We cannot do this with just one process because every file has to be
	// compiled into a different output file
	for (int i = 0; i < inputFiles.count(); i++) {
		QStringList fullArguments = baseArguments;
		fullArguments.append("-E");
		fullArguments.append(inputFiles[i]);
		fullArguments.append("-o");
		fullArguments.append(tempOutput[i]->fileName());
		/*qCritical("Preprocessing : gcc ");
		foreach (QString str, fullArguments)
			qCritical(str.toLocal8Bit());*/
		int returnValue = QProcess::execute("gcc", fullArguments);
		if (returnValue != 0) {
			return returnValue;
		}
	}
	// Read the output from the temporary files
	for (int i = 0; i < inputFiles.count(); i++) {
		if (!tempOutput[i]->open()) {
			qCritical("Could not open preprocessing output.");
			return -1;
		}
		preprocessedInput.append(tempOutput[i]->readAll());
		delete tempOutput[i];
	}
	return 0;
}

QList<QByteArray> Job::readPreprocessedFiles() {
	return preprocessedInput;
}
bool Job::writeOutputFiles(QList<QByteArray> outputFiles) {
	QStringList fileNames = arguments->getOutputFiles();
	if (outputFiles.count() > fileNames.count())
	for (int i = 0; i < outputFiles.count(); i++) {
		QFile file(fileNames[i]);
		if (!file.open(QIODevice::WriteOnly)) {
			qCritical("Error: Could not open %s.", fileNames[i].toStdString().c_str());
			emit finished(-1);
			return false;
		}
		file.write(outputFiles[i]);
		file.close();
	}
	return true;
}

void Job::wasFinished(int returnValue) {
	emit finished(returnValue);
}

