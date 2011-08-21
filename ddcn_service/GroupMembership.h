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

#ifndef GROUPMEMBERSHIP_H_INCLUDED
#define GROUPMEMBERSHIP_H_INCLUDED

#include "McpoGroup.h"
#include "PublicKey.h"

#include <QString>

/**
 * Information about whether this peer is a member of another group.
 *
 * Group membership is realized via a shared private key. Only peers which have
 * the key can pretend to be a member of a group. Of course this also means that
 * the key must not get lost or passed to untrusted others.
 */
class GroupMembership {
public:
	/**
	 * Constructor. Called from CompilerNetwork::addGroupMembership().
	 */
	GroupMembership(QString name, const PrivateKey &privateKey) : name(name),
			privateKey(privateKey) {
	}
	/**
	 * Sets the name of the group. This name is purely descriptive and can be
	 * freely set by the service.
	 */
	void setName(QString name) {
		this->name = name;
	}
	/**
	 * Returns the name of the group.
	 */
	QString getName() {
		return name;
	}
	/**
	 * Returns the private key identifying the group.
	 */
	PrivateKey getPrivateKey() {
		return privateKey;
	}
	/**
	 * Sets the mcpo group which shall be used for this group.
	 *
	 * Multiple TrustedGroup and GroupMembership instances might share an
	 * mcpo group as the mcpo identifier is a simple hash value.
	 */
	void setMcpoGroup(McpoGroup *mcpoGroup) {
		this->mcpoGroup = mcpoGroup;
	}
	/**
	 * Returns the mcpo group used for this group.
	 */
	McpoGroup *getMcpoGroup() {
		return mcpoGroup;
	}
private:
	QString name;
	PrivateKey privateKey;
	McpoGroup *mcpoGroup;
};

#endif
