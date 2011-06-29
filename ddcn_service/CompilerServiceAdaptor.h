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

#ifndef COMPILERSERVICEADAPTOR_H_INCLUDED
#define COMPILERSERVICEADAPTOR_H_INCLUDED

#include "CompilerService.h"
#include "Job.h"
#include <QList>
#include <QDBusAbstractAdaptor>
#include <QStringList>
#include <QDBusMessage>
#include "DBusStructs.h"

/**
 * Class which exports a CompilerService object.
 */
class CompilerServiceAdaptor : public QDBusAbstractAdaptor {
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.ddcn.CompilerService")
public:
	/**
	 * Creates a dbus adaptor for the compiler service.
	 * @param service Compiler service object to export.
	 */
	explicit CompilerServiceAdaptor(CompilerService *service);
public slots:
	void setMaxThreadCount(int threadCount);
	int getCurrentThreadCount();
	int getMaxThreadCount();
	void shutdown();										//TODO
	QList<ToolChainInfo> getToolChains();
	/**
	 * Creates a new Job and returns its DBus path.
	 * @param inputFiles The (binary) content of the files to pass to the new job.
	 * @param parameters The compilation parameters.
	 * @param toolChain The gcc target triple (platform, system and kernel (order may vary) /gcc-version: eg. i686-linux-gnu/4.4.5).
	 */
	JobResult executeJob(QStringList parameters, QString toolChain,
						QString workingPath, const QDBusMessage &message);
	void addToolChain(QString path);
	void removeToolChain(QString path);
	void localCompilationJobFinished(Job *job);
private slots:
	void onCurrentThreadCountChanged(int threadCount);
	void onMaxThreadCountChanged(int threadCount);
	void requestShutdown();
signals:
	void currentThreadCountChanged(int threadCount);
	void maxThreadCountChanged(int threadCount);
	void onShutdownRequest();
private:
	ToolChainInfo toToolChainInfo(ToolChain toolChain);
	CompilerService *service;
	QMap<Job*, QDBusMessage*> jobDBusMessageMap;
};

#endif

