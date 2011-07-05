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
	if (QFile(path).exists()) {
		//TODO zurückgeändert
		QProcess *compiler = new QProcess();
		QProcessEnvironment environment = compiler->processEnvironment();
		environment.value("LANG", "C");
		compiler->setProcessEnvironment(environment);

		QString version = "/";
		QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
		env.insert("LC_ALL", "C");
		compiler->setProcessEnvironment(env);
		compiler->setProcessChannelMode(QProcess::MergedChannels);
		compiler->start(path, QStringList("-v"));
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
