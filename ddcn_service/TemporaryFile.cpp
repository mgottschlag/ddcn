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
#include <QSettings>
#include "TemporaryFile.h"

TemporaryFile::TemporaryFile(QString extension, QString templateName,
							 QString filename) {
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ddcn", "ddcn");
	this->path = settings.value("tempFilePath", "/tmp/").toString();
	this->path = this->path.endsWith("/") ? this->path : this->path + "/";
	this->extension = extension.startsWith(".") ? extension : "." + extension;
	this->name = generateFilename(templateName, filename);
	file = new QFile(this->path + this->name + this->extension);
	if(file->open(QIODevice::WriteOnly)) {
		file->close();
	} else {
		qFatal("Can not write temporary file \"%s\".", file->fileName().toAscii().data());
	}
}

QString TemporaryFile::generateFilename(QString templateName, QString filename) {
	QString returnValue = templateName + filename + randomize();
	while (QFile().exists(this->path + returnValue + this->extension)) {
		returnValue = templateName + filename + randomize();
	}
	return returnValue;
}

QString TemporaryFile::randomize() {
	return QString::number(qrand() % 100000);
}

QString TemporaryFile::getExtension() {
	return this->extension;
}

QFile *TemporaryFile::getFile() {
	return this->file;
}

QString TemporaryFile::getFilename() {
	return this->path + this->name + this->extension;
}

