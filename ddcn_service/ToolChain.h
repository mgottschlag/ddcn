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

#ifndef TOOL_CHAIN_INCLUDED
#define TOOL_CHAIN_INCLUDED

#include <QProcess>
#include <QString>

/**
 * Class contains information about the available compiler versions
 */
class ToolChain {
public:
	/**
	 * Constructor.
	 * Does not check the values passed to it.
	 *
	 * @param version Version of the toolchain (e.g. "i686-linux-gnu/4.4" for
	 * a 32-bit GCC 4.4.x).
	 * @param path Path to the compilers, instead of "gcc" or "g++" this holds
	 * a "*", e.g. "/usr/bin/ *" (without the space) points to "/usr/bin/gcc".
	 */
	ToolChain(QString version, QString path) : version(version), path(path) {
	}

	/**
	 * Constructor.
	 * This checks whether the compiler at the path is valid, and if it is not,
	 * initializes all members of this object to "". If it is valid, the
	 * correct version is retrieved.
	 *
	 * @param path Path to the compilers, instead of "gcc" or "g++" this holds
	 * a "*", e.g. "/usr/bin/ *" (without the space) points to "/usr/bin/gcc".
	 */
	ToolChain(QString path);

	/**
	 * Constructor.
	 * Initializes all members with "".
	 */
	ToolChain() {
	}

	/**
	 * Returns the gcc target triple (platform, system and kernel (order may vary) /gcc-version:
	 * eg. i686-linux-gnu/4.4.5).
	 *
	 * @return the gcc target triple
	 */
	QString getVersion() const;

	/**
	 * Returns the path to the compiler. This is "" (empty string) if the
	 * toolchain is not valid.
	 *
	 * @return the path to the compiler, a "*" denotes the part where the name
	 * of the compiler for a certain language is inserted (e.g. "gcc" for C).
	 */
	QString getPath() const;
	/**
	 * Returns a special part to the compiler for a certain language. This is
	 * "" (the empty string) if the toolchain is not valid.
	 *
	 * @param language Language ("c" or "c++").
	 *
	 * @return Path to the compiler executable.
	 */
	QString getPath(QString language) const;

	/**
	 * Compares the toolchain to another toolchain. Two toolchains are only
	 * equal if both path and version are equal.
	 *
	 * @param o Other toolchain to compare this to.
	 *
	 * @return True if the toolchains are equal.
	 */
	bool operator == (const ToolChain &o) {
		if (o.path == path && o.version == version) {
			return true;
		} else {
			return false;
		}
	}

	/**
	 * Checks whether two toolchain targets/versions are compatible. This is
	 * the case if both architectures are the same or the source arch is x86
	 * and the target arch is x86_64 (in this case, "-m32" is added to
	 * compatibilityParameters), and if the rest of the target triple is either
	 * identical or in both cases contains "linux".
	 *
	 * @param sourceToolChain Source tool chain version, e.g.
	 * "i686-linux-gnu/4.4".
	 * @param targetToolChain Target tool chain version.
	 * @param compatibilityParameters If != NULL, this list is filled with
	 * additional parameters needed for compatibility reasons.
	 *
	 * @return True if the target peer can compile files from the source peer.
	 */
	static bool isCompatible(const QString &sourceToolChain,
			const QString &targetToolChain,
			QStringList *compatibilityParameters = NULL);
private:
	QString version;
	QString path;
};

#endif

