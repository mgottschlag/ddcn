
#include "LogWriter.h"

#include <QSettings>
#include <QFileInfo>
#include <QDateTime>
#include <cstdlib>
#include <iostream>
#include <cassert>
#include <QStringList>

LogWriter::LogWriter() : logStream(NULL) {
	// This is a singleton, we need to implement it like this to ensure that
	// the destructor is called at the right time (when the variable leaves
	// scope in main())
	if (instance != NULL) {
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
void LogWriter::clearLog() {
	QMutexLocker lock(&logMutex);
	delete logStream;
	logStream = NULL;
	logFile.close();
	logFile.remove();
	if (!logFile.open(QIODevice::Append | QIODevice::Text)) {
		qInstallMsgHandler(0);
		lock.unlock();
		qFatal("clearLog: Failed to reopen the log file.");
	}
	logStream = new QTextStream(&logFile);
	lock.unlock();
	qDebug("------------------------ Log cleared. ------------------------");
}

void LogWriter::qtMessageHandler(QtMsgType type, const char *message) {
	QDateTime dateTime = QDateTime::currentDateTime();
	QString prefix = dateTime.toString() + "\t";
	QStringList lines = QString(message).split('\n');
	switch (type) {
	case QtDebugMsg:
		prefix += "Debug\t";
		break;
	case QtWarningMsg:
		prefix += "Warning\t";
		break;
	case QtCriticalMsg:
		prefix += "Critical\t";
		break;
	case QtFatalMsg:
		prefix += "Fatal\t";
		break;
	}
	QMutexLocker lock(&instance->logMutex);
	foreach (QString line, lines) {
		std::cout << (prefix + line).toAscii().data() << std::endl;
	}
	if (!instance->logFile.isOpen()) {
		return;
	}
	foreach (QString line, lines) {
		*instance->logStream << prefix << line << "\n";
	}
	instance->logStream->flush();
	instance->logFile.flush();
	if (type == QtFatalMsg) {
		std::abort();
	}
}

LogWriter *LogWriter::instance = NULL;
