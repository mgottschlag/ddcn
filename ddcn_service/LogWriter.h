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

#ifndef LOGWRITER_H_INCLUDED
#define LOGWRITER_H_INCLUDED

#include <QtGlobal>
#include <QFile>
#include <QTextStream>

/**
 * Class which logs all output to a file which comes from qDebug, qCritical or
 * qWarning.
 *
 * The log file is placed in the same directory as the config file.
 *
 * This class is a singleton, only one instance can exist at a time.
 */
class LogWriter {
public:
	/**
	 * Constructor.
	 *
	 * Aborts when you try to create more than one instance at a time.
	 */
	LogWriter();
	/**
	 * Destructor.
	 *
	 * Closes the log file.
	 */
	~LogWriter();

	/**
	 * Returns the instance of the log writer.
	 *
	 * @return Instance of the log writer.
	 */
	static LogWriter &get();

	/**
	 * Opens the log file.
	 */
	bool init();
private:
	static void qtMessageHandler(QtMsgType type, const char *message);
	Q_DISABLE_COPY(LogWriter);

	QFile logFile;
	QTextStream *logStream;

	static LogWriter *instance;
};

#endif
