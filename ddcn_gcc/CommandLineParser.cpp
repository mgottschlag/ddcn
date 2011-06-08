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
#include <cstring>

CommandLineParser::CommandLineParser(int argc, char **argv) {
	// Parse arguments
	bool compilingOnly = false;
	bool nativeOptimization = false;
	for (int i = 1; i < argc; i++) {
		// Only if "-c" is given we can actually offload this command to the
		// network as linking has to be done locally
		if (!std::strcmp(argv[i], "-c")) {
			compilingOnly = true;
		}
		// If we are supposed to optimize for the host architecture, this cannot
		// easily be done on other computers, so leave this out for now
		if (!std::strcmp(argv[i], "-mcpu=native")) {
			compilingOnly = true;
		}
		if (!std::strcmp(argv[i], "-mtune=native")) {
			compilingOnly = true;
		}
		originalArguments.append(argv[i]);
	}
	QStringList &arguments = originalArguments;
	if (!compilingOnly || nativeOptimization) {
		remoteExecutionPossible = false;
		// We can only execute the job locally, so do not do more work here
		return;
	} else {
		remoteExecutionPossible = true;
	}
	// Split options into preprocessor and remote compiler options
	for (int i = 0; i < arguments.count(); i++) {
		QString argument = arguments[i];
		if (argument == "-I" || argument == "-iquote") {
			if (i == arguments.count() - 1) {
				remoteExecutionPossible = false;
				return;
			}
			preprocessingParameters.append(argument);
			i++;
			preprocessingParameters.append(arguments[i]);
		} else if (argument.startsWith("-I") || argument.startsWith("-iquote")) {
			preprocessingParameters.append(argument);
		} else if (argument == "-L") {
			// We do not need to care about any linker paths
			if (i == arguments.count() - 1) {
				remoteExecutionPossible = false;
				return;
			}
			i++;
		} else if (argument.startsWith("-L")) {
			// Drop linker path again
		} else if (argument == "-o") {
			if (i == arguments.count() - 1) {
				remoteExecutionPossible = false;
				return;
			}
			i++;
			outputFiles.append(arguments[i]);
		} else if (argument == "-c") {
			remoteParameters.append(argument);
		} else if (argument.at(0) == '-'){
			// TODO: Dependency generation arguments
			// TODO: Other parameters which do not have any "-" in front of them
			preprocessingParameters.append(argument);
			remoteParameters.append(argument);
		} else {
			// This is an input file
			inputFiles.append(argument);
		}
	}
	if (inputFiles.empty()) {
		remoteExecutionPossible = false;
		return;
	}
	if (outputFiles.empty()) {
		// No output file was specified, so we have to derive output names from
		// the input files
		for (int i = 0; i < inputFiles.count(); i++) {
			QString fileName = inputFiles[i];
			if (fileName.contains(".")) {
				fileName = fileName.left(fileName.lastIndexOf('.'));
			}
			fileName.append(".o");
			outputFiles.append(fileName);
		}
	}
}

bool CommandLineParser::isRemoteExecutionPossible() {
	return remoteExecutionPossible;
}
QStringList CommandLineParser::getPreprocessingParameters() {
	return preprocessingParameters;
}
QStringList CommandLineParser::getInputFiles() {
	return inputFiles;
}
QStringList CommandLineParser::getRemoteParameters() {
	return remoteParameters;
}
QStringList CommandLineParser::getOutputFiles() {
	return outputFiles;
}

QStringList CommandLineParser::getOriginalParameters() {
	return originalArguments;
}
