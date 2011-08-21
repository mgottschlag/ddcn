
#include "LogWriter.h"

#include <QSettings>
#include <QFileInfo>
#include <QDateTime>
#include <cstdlib>
#include <iostream>
#include <cassert>

LogWriter::LogWriter() : logStream(NULL) {
	// This is a singleton, we need to implement it like this to ensure that
	// the destructor is called at the right time (when the variable leaves
	// scope in main())
	if (instance == NULL) {
		qFatal("More than one LogWriter instance created!");
	}
	instance = this;
}
LogWriter::~LogWriter() {
	if (logFile.isOpen()) {
		qDebug("------------------------ Log closed. ------------------------");
		qInstallMsgHandler(0);
		logFile.close();
		delete logStream;
		logStream = NULL;
	}
}

LogWriter &LogWriter::get() {
	return *instance;
}

bool LogWriter::init() {
	// We store the log file along the configuration file
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "ddcn", "ddcn");
	QString settingsDir = QFileInfo(settings.fileName()).absolutePath();
	QString logFileName = settingsDir + "/ddcn.log";
	logFile.setFileName(logFileName);
	if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
		return false;
	}
	logStream = new QTextStream(&logFile);
	qInstallMsgHandler(qtMessageHandler);
	qDebug("------------------------ Log opened. ------------------------");
	return true;
}

void LogWriter::qtMessageHandler(QtMsgType type, const char *message) {
	QDateTime dateTime = QDateTime::currentDateTime();
	QString dateTimeString = dateTime.toString() + ": ";
	QString text = dateTimeString;
	switch (type) {
	case QtDebugMsg:
		text += QString("Debug: %1\n").arg(message);
		break;
	case QtWarningMsg:
		text += QString("Warning: %1\n").arg(message);
		break;
	case QtCriticalMsg:
		text += QString("Critical: %1\n").arg(message);
		break;
	case QtFatalMsg:
		text += QString("Fatal: %1\n").arg(message);
		break;
	}
	*LogWriter::get().logStream << text;
	LogWriter::get().logStream->flush();
	LogWriter::get().logFile.flush();
	std::cout << text.toAscii().data();
	std::flush(std::cout);
	if (type == QtFatalMsg) {
		std::abort();
	}
}

LogWriter *LogWriter::instance = NULL;
