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
QDBusArgument &operator<<(QDBusArgument &argument, const ToolChain &info)
{
	argument.beginStructure();
	argument << info.getVersion() << info.getPath();
	argument.endStructure();
	return argument;
}
const QDBusArgument &operator>>(const QDBusArgument &argument, ToolChain &info)
{
	argument.beginStructure();
	QString version;
	QString path;
	argument >> version >> path;
	argument.endStructure();
	info = ToolChain(version, path);
	return argument;
}
