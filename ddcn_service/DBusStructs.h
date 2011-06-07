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

#ifndef DBUSSTRUCTS_H_INCLUDED
#define DBUSSTRUCTS_H_INCLUDED

#include "ToolChain.h"
#include <QString>
#include <QDBusMetaType>


struct TrustedPeerInfo {
	QString name;
	QString publicKey;
};

Q_DECLARE_METATYPE(TrustedPeerInfo)
Q_DECLARE_METATYPE(QList<TrustedPeerInfo>)

QDBusArgument &operator<<(QDBusArgument &argument, const TrustedPeerInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &argument, TrustedPeerInfo &info);

struct TrustedGroupInfo {
	QString name;
	QString publicKey;
};

Q_DECLARE_METATYPE(TrustedGroupInfo)
Q_DECLARE_METATYPE(QList<TrustedGroupInfo>)

QDBusArgument &operator<<(QDBusArgument &argument, const TrustedGroupInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &argument, TrustedGroupInfo &info);

struct GroupMembershipInfo {
	QString name;
	QString publicKey;
};

Q_DECLARE_METATYPE(GroupMembershipInfo)
Q_DECLARE_METATYPE(QList<GroupMembershipInfo>)

QDBusArgument &operator<<(QDBusArgument &argument, const GroupMembershipInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &argument, GroupMembershipInfo &info);


Q_DECLARE_METATYPE(ToolChain)
Q_DECLARE_METATYPE(QList<ToolChain>)

QDBusArgument &operator<<(QDBusArgument &argument, const ToolChain &info);
const QDBusArgument &operator>>(const QDBusArgument &argument, ToolChain &info);

#endif
