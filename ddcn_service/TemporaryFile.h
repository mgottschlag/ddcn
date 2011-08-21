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

#include <QString>
#include <QFile>
#include <boost/concept_check.hpp>

/**
 * Represents a temporary file.
 */
class TemporaryFile {
public:
	/**
	 * Creates a new temporary file according to the given extension, the templateName which by
	 * default is "ddcn_tmp_" and the filename.
	 * A random number will be added to the filename.
	 * Note:  the temporary file will be written to the folder specified as temporary files folder in
	 * the settings.
	 * The full filename will be of the form: path/templateName + filename + random number.extension
	 * @param extension the extension for the file.
	 * @param templateName the templateName for the new file.
	 * @param filename the filename for the new file.
	 */
	TemporaryFile(QString extension, QString templateName = "ddcn_tmp_",
				  QString filename = "");
	/**
	 * Returns the full filename of the file of the form: path/templateName + filename + random number.extension
	 * @return the full filename.
	 */
	QString getFilename();

	/**
	 * Returns the extension of the file.
	 * @return the extension of the file.
	 */
	QString getExtension();

	/**
	 * Returns the file as QFile object.
	 * @return the file.
	 */
	QFile *getFile();
private:
	/**
	 * Generates a new unique (meaning: not yet existent in the temporary files folder) filename
	 * consisting of the given templateName, the filename and a random number.
	 * @param templateName the templateName.
	 * @param filename the filename.
	 * @return the generated filename.
	 */
	QString generateFilename(QString templateName, QString filename);

	/**
	 * Returns a random integer number with maximum 6 digits.
	 * @return a random integer number.
	 */
	QString randomize();
	QString name;
	QString path;
	QString extension;
	QFile *file;
};