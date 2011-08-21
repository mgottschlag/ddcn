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

#ifndef PARAMETERPARSER_H_INCLUDED
#define PARAMETERPARSER_H_INCLUDED

#include <QStringList>

/**
 * Parses the parameters for a gcc call and splits them up into preprocessor
 * and compiler parameter sets.
 *
 * For local compilation then the original parameters are used whereas for
 * remote compilation only the compiler parameters are sent to prevent double
 * evaluation of macros.
 */
class ParameterParser {
public:
	/**
	 * Constructor. Parses the given set of parameters.
	 *
	 * @param rawParameters Set of parameters passed to gcc, shall not contain
	 * the first parameter which contains the program name only.
	 */
	ParameterParser(const QStringList &rawParameters);
	/**
	 * Constructor. Creates empty parameter lists.
	 */
	ParameterParser();

	/**
	 * Parses a set of parameters. This is called implicitly from the
	 * constructor which takes a string list parameter.
	 */
	void parse(const QStringList &rawParameters);

	/**
	 * Returns true if the job can be delegated to a different peer. This is the
	 * case if only preprocessing is done or if linking shall be done, or if the
	 * parser encounters parameters which are not yet supported.
	 */
	bool isDelegatable();

	/**
	 * Returns the original unaltered set of parameters.
	 */
	QStringList getOriginalParameters();

	/**
	 * Returns the set of parameters which shall be used for local preprocessing
	 * before a job is sent out.
	 */
	QStringList getPreprocessingParameters();
	/**
	 * Returns the set of compiler parameters which shall be used by other
	 * peers.
	 */
	QStringList getCompilerParameters();

	/**
	 * Returns a list with the input file names which have to be preprocessed.
	 */
	QStringList getInputFiles();
	/**
	 * Returns a list with the resulting output file names. The returning array
	 * has to be as long as the array returned by getInputFiles().
	 */
	QStringList getOutputFiles();
private:
	bool delegatable;

	QStringList originalParameters;

	QStringList preprocessingParameters;
	QStringList compilerParameters;

	QStringList inputFiles;
	QStringList outputFiles;
};

#endif
