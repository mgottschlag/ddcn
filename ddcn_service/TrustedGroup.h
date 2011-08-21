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

#ifndef TRUSTEDGROUP_H_INCLUDED
#define TRUSTEDGROUP_H_INCLUDED

#include "McpoGroup.h"

#include <QString>

/**
 * Stores the information about a trusted group.
 *
 * CompilerNetwork holds a list of these and reads/saves them from a file
 */
class TrustedGroup {
public:
	/**
	 * Constructor. Called from CompilerNetwork::addTrustedGroup().
	 */
	TrustedGroup(QString name, const PublicKey &publicKey) : name(name),
			publicKey(publicKey), mcpoGroup(0) {
	}

	/**
	 * Sets the name of the trusted group. This name is purely descriptive and
	 * can be freely set by the service.
	 */
	void setName(QString name) {
		this->name = name;
	}
	/**
	 * Returns the name of the trusted group.
	 */
	QString getName() {
		return name;
	}
	/**
	 * Returns the public key identifying the trusted group.
	 */
	PublicKey getPublicKey() {
		return publicKey;
	}

	/**
	 * Sets the mcpo group which shall be used for this group.
	 *
	 * Multiple TrustedGroup and GroupMembership instances might share an
	 * mcpo group as the mcpo identifier is a simple hash value.
	 */
	void setMcpoGroup(McpoGroup *group) {
		mcpoGroup = group;
	}
	/**
	 * Returns the mcpo group used for this group.
	 */
	McpoGroup *getMcpoGroup() {
		return mcpoGroup;
	}
private:
	QString name;
	PublicKey publicKey;
	McpoGroup *mcpoGroup;
};

#endif
