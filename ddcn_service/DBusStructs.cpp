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

#include "DBusStructs.h"

QDBusArgument &operator<<(QDBusArgument &argument, const TrustedPeerInfo &info)
{
	argument.beginStructure();
	argument << info.name << info.publicKey;
	argument.endStructure();
	return argument;
}
const QDBusArgument &operator>>(const QDBusArgument &argument, TrustedPeerInfo &info)
{
	argument.beginStructure();
	argument >> info.name >> info.publicKey;
	argument.endStructure();
	return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const TrustedGroupInfo &info)
{
	argument.beginStructure();
	argument << info.name << info.publicKey;
	argument.endStructure();
	return argument;
}
const QDBusArgument &operator>>(const QDBusArgument &argument, TrustedGroupInfo &info)
{
	argument.beginStructure();
	argument >> info.name >> info.publicKey;
	argument.endStructure();
	return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const GroupMembershipInfo &info)
{
	argument.beginStructure();
	argument << info.name << info.publicKey;
	argument.endStructure();
	return argument;
}
const QDBusArgument &operator>>(const QDBusArgument &argument, GroupMembershipInfo &info)
{
	argument.beginStructure();
	argument >> info.name >> info.publicKey;
	argument.endStructure();
	return argument;
}
QDBusArgument &operator<<(QDBusArgument &argument, const ToolChainInfo &info)
{
	argument.beginStructure();
	argument << info.version << info.path;
	argument.endStructure();
	return argument;
}
const QDBusArgument &operator>>(const QDBusArgument &argument, ToolChainInfo &info)
{
	argument.beginStructure();
	QString version;
	QString path;
	argument >> version >> path;
	argument.endStructure();
	info.version = version;
	info.path = path;
	return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const JobResult &jobResult) {
	argument.beginStructure();
	argument << jobResult.stdout << jobResult.stderr << jobResult.returnValue;
	argument.endStructure();
	return argument;
}
const QDBusArgument &operator>>(const QDBusArgument &argument, JobResult &jobResult) {
	argument.beginStructure();
	argument >> jobResult.stdout >> jobResult.stderr >> jobResult.returnValue;
	argument.endStructure();
	return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const NodeStatus &nodeStatusInfo) {
	argument.beginStructure();
	argument << nodeStatusInfo.maxThreads;
	argument << nodeStatusInfo.currentThreads;
	argument << nodeStatusInfo.localJobs;
	argument << nodeStatusInfo.delegatedJobs;
	argument << nodeStatusInfo.remoteJobs;
	argument.endStructure();
	return argument;
}
const QDBusArgument &operator>>(const QDBusArgument &argument, NodeStatus &nodeStatusInfo) {
	argument.beginStructure();
	argument >> nodeStatusInfo.maxThreads;
	argument >> nodeStatusInfo.currentThreads;
	argument >> nodeStatusInfo.localJobs;
	argument >> nodeStatusInfo.delegatedJobs;
	argument >> nodeStatusInfo.remoteJobs;
	argument.endStructure();
	return argument;
}

void registerCustomDBusTypes() {
	qDBusRegisterMetaType<TrustedPeerInfo>();
	qDBusRegisterMetaType<QList<TrustedPeerInfo> >();
	qDBusRegisterMetaType<TrustedGroupInfo>();
	qDBusRegisterMetaType<QList<TrustedGroupInfo> >();
	qDBusRegisterMetaType<GroupMembershipInfo>();
	qDBusRegisterMetaType<QList<GroupMembershipInfo> >();
	qDBusRegisterMetaType<ToolChainInfo>();
	qDBusRegisterMetaType<QList<ToolChainInfo> >();
	qDBusRegisterMetaType<NodeStatus>();
	qDBusRegisterMetaType<JobResult>();
}
