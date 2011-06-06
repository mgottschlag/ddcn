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

#include "CommandLineParser.h"

#include <cstdlib>

CommandLineParser::CommandLineParser(int argc, char **argv) {
	// Create backup or all arguments
	backupArguments(argc, argv);
	// Parse arguments via getopt
	// TODO
}
CommandLineParser::~CommandLineParser() {
	// Free argument backup
	char **arg = argumentBackup;
	while (arg != NULL) {
		std::free(*arg);
		arg++;
	}
	delete[] argumentBackup;
}

bool CommandLineParser::isRemoteExecutionPossible() {
	// TODO
	return false;
}
QStringList CommandLineParser::getPreprocessingParameters() {
	// TODO
	return QStringList();
}
QStringList CommandLineParser::getPreprocessingFileList() {
	// TODO
	return QStringList();
}
QStringList CommandLineParser::getRemoteParameters() {
	// TODO
	return QStringList();
}
QStringList CommandLineParser::getOutputFileNames() {
	// TODO
	return QStringList();
}

char **CommandLineParser::getOriginalParameters() {
	return argumentBackup;
}

void CommandLineParser::backupArguments(int argc, char **argv) {
	argumentBackup = new char*[argc];
	argumentBackup[argc] = NULL;
	// We do not need to backup the first argument as it is our process name
	for (int i = 0; i < argc - 1; i++) {
		argumentBackup[i] = strdup(argv[i + 1]);
	}
}
