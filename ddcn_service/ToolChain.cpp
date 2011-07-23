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


#include <QProcess>
#include <QString>
#include <QFile>
#include "ToolChain.h"

ToolChain::ToolChain(QString path) {
	QString gccPath = QString(path).replace("*", "gcc");
	qDebug("Toolchain path: %s", path.toAscii().data());
	if (QFile(gccPath).exists()) {
		QProcess *compiler = new QProcess();
		QProcessEnvironment environment = compiler->processEnvironment();
		environment.value("LANG", "C");
		compiler->setProcessEnvironment(environment);

		QString version = "/";
		QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
		env.insert("LC_ALL", "C");
		compiler->setProcessEnvironment(env);
		compiler->setProcessChannelMode(QProcess::MergedChannels);
		compiler->start(gccPath, QStringList("-v"));
		if (!compiler->waitForStarted(500)) {
			qCritical("Could not start compiler \"%s\".",
					path.toAscii().data());
			return;
		}
		compiler->waitForFinished(500);
		while (!compiler->atEnd()) {
			QString line = compiler->readLine(1000);
			if (line.startsWith("Target: ")) {
				version = line.mid(8, line.count() - 9).append(version);
			} else if (line.startsWith("gcc version")) {
				// We only need two gcc version numbers
				version.append(line.mid(12, 3));
			}
		}
		delete compiler;
		this->version = version;
		this->path = path;
	} else {
		qCritical("Invalid toochain path.");
	}
}


QString ToolChain::getVersion() const {
	return this->version;
}

QString ToolChain::getPath() const {
	return this->path;
}

QString ToolChain::getPath(QString language) const {
	qDebug("Language: %s", language.toAscii().data());
	qDebug("Path: %s", path.toAscii().data());
	if (language == "c") {
		return QString(path).replace("*", "gcc");
	} else if (language == "c++") {
		return QString(path).replace("*", "g++");
	} else {
		return "";
	}
}


bool ToolChain::isCompatible(const QString &sourceToolChain,
		const QString &targetToolChain, QStringList *compatibilityParameters) {
	if (sourceToolChain == targetToolChain) {
		return true;
	}
	QString sourceTarget = sourceToolChain.left(sourceToolChain.indexOf("/"));
	QString sourceVersion = sourceToolChain.mid(sourceToolChain.indexOf("/") + 1);
	QString targetTarget = targetToolChain.left(targetToolChain.indexOf("/"));
	QString targetVersion = targetToolChain.mid(targetToolChain.indexOf("/") + 1);
	// We only have backwards compatibility
	// (0.1f = comparison delta :) )
	if (sourceVersion.toFloat() <= targetVersion.toFloat() + 0.1f) {
		return false;
	}
	// Check whether the CPU architecture is compatible
	QString sourceArch = sourceTarget.left(sourceTarget.indexOf("-"));
	QString targetArch = targetTarget.left(targetTarget.indexOf("-"));
	bool archCompatible = false;
	if ((sourceArch == "i686" || sourceArch == "i586" || sourceArch == "i486")
			&& (targetArch == "x86_64" || targetArch == "amd64")) {
		archCompatible = true;
		// 64bit intel compilers can usually compile 32bit code
		if (compatibilityParameters != NULL) {
			compatibilityParameters->append("-m32");
		}
	} else if (sourceArch == targetArch) {
		archCompatible = true;
	}
	if (!archCompatible) {
		return false;
	}
	// Check whether the operating system is compatible
	if (sourceTarget.contains("linux") && targetTarget.contains("linux")) {
		// Both are linux
		return true;
	} else if (sourceTarget.mid(sourceTarget.indexOf("-"))
			== targetTarget.mid(targetTarget.indexOf("-"))) {
		// The target triple only has a different but compatible architecture
		return true;
	} else {
		return false;
	}
}

