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

#include "ParameterParser.h"

ParameterParser::ParameterParser(const QStringList &rawParameters) {
	parse(rawParameters);
	/*qCritical("Parser, input: %d", this->inputFiles.count());
	qCritical("Parser, compile parameter: %d", this->compilerParameters.count());
	qCritical("Parser, perProParameter: %d", this->preprocessingParameters.count());*/
}
ParameterParser::ParameterParser() : delegatable(false) {
}

void ParameterParser::parse(const QStringList &rawParameters) {
	originalParameters = rawParameters;
	preprocessingParameters.clear();
	compilerParameters.clear();
	inputFiles.clear();
	outputFiles.clear();
	// We cannot delegate the job if we are either linking, only running the
	// preprocessor or
	bool compilingOnly = false;
	bool nativeOptimization = false;
	foreach (QString parameter, rawParameters) {
		// Only if "-c" is given we can actually offload this command to the
		// network as linking has to be done locally
		if (parameter == "-c") {
			compilingOnly = true;
		}
		// If we are supposed to optimize for the host architecture, this cannot
		// easily be done on other computers, so leave this out for now
		if (parameter == "-mcpu=native") {
			nativeOptimization = true;
		}
		if (parameter == "-mtune=native") {
			nativeOptimization = true;
		}
	}
	delegatable = compilingOnly && !nativeOptimization;

	for (int i = 0; i < rawParameters.size(); i++) {
		QString parameter = rawParameters[i];
		if (parameter == "-I" || parameter == "-iquote") {
			if (i == parameter.count() - 1) {
				delegatable = false;
				continue;
			}
			preprocessingParameters.append(parameter);
			i++;
			preprocessingParameters.append(rawParameters[i]);
		} else if (parameter.startsWith("-I") || parameter.startsWith("-iquote")) {
			preprocessingParameters.append(parameter);
		} else if (parameter == "-L") {
			// We do not need to care about any linker paths
			if (i == parameter.count() - 1) {
				delegatable = false;
				continue;
			}
			i++;
		} else if (parameter.startsWith("-L")) {
			// Drop linker path again
		} else if (parameter == "-o") {
			if (i == parameter.count() - 1) {
				delegatable = false;
				continue;
			}
			i++;
			outputFiles.append(rawParameters[i]);
		} else if (parameter == "-c") {
			compilerParameters.append(parameter);
		} else if (parameter == "-") {
			// Do not delegate job which reads from stdin
			// TODO: This can easily be implemented
			delegatable = false;
			preprocessingParameters.append(parameter);
			compilerParameters.append(parameter);
		} else if (parameter == "-MF"
				|| parameter == "-MT"
				|| parameter == "-MQ") {
			// Dependency generation parameters with additional parameter
			preprocessingParameters.append(parameter);
			if (i == parameter.count() - 1) {
				delegatable = false;
				continue;
			}
			i++;
			preprocessingParameters.append(rawParameters[i]);
		} else if (parameter == "-MG"
				|| parameter == "-MP"
				|| parameter == "-MD"
				|| parameter == "-MMD") {
			// Dependency generation parameters
			preprocessingParameters.append(parameter);
		} else if (parameter == "E"
				|| parameter == "-M"
				|| parameter == "-MM") {
			// Do not delegate jobs which only perform preprocessing
			delegatable = false;
			preprocessingParameters.append(parameter);
			compilerParameters.append(parameter);
		} else if (parameter.at(0) == '-'){
			// TODO: Other parameters which do not have any "-" in front of them
			// TODO: File types
			preprocessingParameters.append(parameter);
			compilerParameters.append(parameter);
		} else {
			// This is an input file
			inputFiles.append(parameter);
		}
	}
	if (inputFiles.empty() || outputFiles.count() > 1
			|| (!outputFiles.empty() && inputFiles.count() != 1)) {
		delegatable = false;
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

bool ParameterParser::isDelegatable() {
	return delegatable;
}

QStringList ParameterParser::getOriginalParameters() {
	return originalParameters;
}

QStringList ParameterParser::getPreprocessingParameters() {
	if (!isDelegatable()) {
		return QStringList();
	}
	return preprocessingParameters;
}
QStringList ParameterParser::getCompilerParameters() {
	return compilerParameters;
}

QStringList ParameterParser::getInputFiles() {
	return inputFiles;
}
QStringList ParameterParser::getOutputFiles() {
	return outputFiles;
}
