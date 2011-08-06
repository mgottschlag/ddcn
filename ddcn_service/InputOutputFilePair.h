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

#ifndef INPUTOUTPUTFILE_H_INCLUDED
#define INPUTOUTPUTFILE_H_INCLUDED

#include <QObject>
#include <QFile>
#include <QString>
#include <QDir>


//TODO this class could implement TemporaryFile!
/**
 * This class holds a pair of files (input and output files).
 */
class InputOutputFilePair : public QObject {
	Q_OBJECT
public:
	/**
	 * Creates a new pair of temporary files with a unique filename.
	 * The filename is created by using the given templateName, a random number and the
	 * given extensions. Both extensions include the leading dot, e.g. ".c".
	 * @param extension1 the extension for the input (source) file.
	 * @param extension2 the extension for the output (target) file.
	 * @param templateName the templateName for the file. This helps to
	 * identify the files as a file belonging to ddcn.
	 */
	InputOutputFilePair(QString extension1, QString extension2,
						QString templateName = "ddcn_tmp_");
	/**
	 * Returns the input (source) file's extension.
	 * @return the input file's extension.
	 */
	QString getInputFileExtension();
	/**
	 * Returns the output (source) file's extension.
	 * @return the output file's extension.
	 */
	QString getOutputFileExtension();
	/**
	 * Returns the filename of the input (source) file.
	 * @return the filename of the input file.
	 */
	QString getInputFilename();
	/**
	 * Returns the filename of the output (source) file.
	 * @return the filename of the output file.
	 */
	QString getOutputFilename();
	/**
	 * Returns the input (target) file as QFile object.
	 * @return  the input file.
	 */
	QFile* getInputFile();
	/**
	 * Returns the output (target) file as QFile object.
	 * @return  the output file.
	 */
	QFile* getOutputFile();
private:
	QString generateFilename(QString templateName);
	void createTemporaryFiles();
	QString randomize();
	QString filename;
	QString extensions[2];
	QString path;
	//TODO onDestroy()
};

#endif