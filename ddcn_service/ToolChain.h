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

#ifndef TOOL_CHAIN_INCLUDED
#define TOOL_CHAIN_INCLUDED
#include <QProcess>
#include <QString>
/**
 * Class contains information about the available compiler versions
 */
class ToolChain {
public:
	ToolChain(QString version, QString path) : version(version), path(path) {
	}

	ToolChain(QString path);

	ToolChain() {
	}

	/**
	 * Returns the gcc target triple (platform, system and kernel (order may vary) /gcc-version:
	 * eg. i686-linux-gnu/4.4.5).
	 * @return the gcc target triple
	 */
	QString getVersion() const;

	/**
	 * Returns the path to the compiler.
	 * @return the path to the compiler, a "*" denotes the part where the name
	 * of the compiler for a certain language is inserted (e.g. "gcc" for C).
	 */
	QString getPath() const;
	/**
	 * Returns a special part to the compiler for a certain language.
	 * @param language Language ("c" or "c++").
	 * @return Path to the compiler executable.
	 */
	QString getPath(QString language) const;

	bool operator == (ToolChain o) {
		if (o.path == path && o.version == version) {
			return true;
		} else {
			return false;
		}
	}

	static bool isCompatible(const QString &sourceToolChain,
			const QString &targetToolChain,
			QStringList *compatibilityParameters = NULL);
private:
	QString version;
	QString path;
};

#endif

