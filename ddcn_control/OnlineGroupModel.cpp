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

#include "OnlineGroupModel.h"

#include <cassert>

OnlineGroupModel::OnlineGroupModel() {
}

void OnlineGroupModel::clear() {
	onlineGroups.clear();
	// TODO: Emit proper change signal
	reset();
}

void OnlineGroupModel::addNodeToGroup(QString name, QString key,
		QString fingerprint, bool trusted, float load, bool member) {
	bool found = false;
	for (int i = 0; i < onlineGroups.size(); i++) {
		if (onlineGroups[i].key == key) {
			found = true;
			onlineGroups[i].load = onlineGroups[i].load * onlineGroups[i].memberCount + load;
			onlineGroups[i].load /= onlineGroups[i].memberCount + 1;
			onlineGroups[i].memberCount++;
		}
	}
	if (!found) {
		OnlineGroupInfo newGroup;
		newGroup.name = name;
		newGroup.key = key;
		newGroup.fingerprint = fingerprint;
		newGroup.trusted = trusted;
		newGroup.load = load;
		newGroup.member = member;
		newGroup.memberCount = 1;
		onlineGroups.append(newGroup);
	}
	// TODO: Emit proper change signals, this is way too slow
	reset();
}

bool OnlineGroupModel::isTrusted(const QModelIndex &index) {
	if (!index.isValid()) {
		return false;
	}
	assert(index.row() < onlineGroups.count());
	return onlineGroups[index.row()].trusted;
}
QString OnlineGroupModel::getKey(const QModelIndex &index) {
	if (!index.isValid()) {
		return "";
	}
	assert(index.row() < onlineGroups.count());
	return onlineGroups[index.row()].key;
}
QString OnlineGroupModel::getFingerprint(const QModelIndex &index) {
	if (!index.isValid()) {
		return "";
	}
	assert(index.row() < onlineGroups.count());
	return onlineGroups[index.row()].fingerprint;
}
QString OnlineGroupModel::getName(const QModelIndex &index) {
	if (!index.isValid()) {
		return "";
	}
	assert(index.row() < onlineGroups.count());
	return onlineGroups[index.row()].name;
}
void OnlineGroupModel::setTrusted(const QModelIndex &index, bool trusted) {
	if (!index.isValid()) {
		return;
	}
	assert(index.row() < onlineGroups.count());
	onlineGroups[index.row()].trusted = trusted;
	// TODO: Emit proper change signals, this is way too slow
	reset();
}

int OnlineGroupModel::columnCount(const QModelIndex &parent) const {
	if (parent.isValid()) {
		return 0;
	}
	// Columns: Trusted status, name, key fingerprint, member count, load, member of trusted group
	return 6;
}
QVariant OnlineGroupModel::data(const QModelIndex &index, int role) const {
	if (index.row() < 0 || index.row() >= onlineGroups.size()) {
		return QVariant();
	}
	if (role != Qt::DisplayRole) {
		return QVariant();
	}
	OnlineGroupInfo group = onlineGroups[index.row()];
	switch (index.column()) {
		case 0:
			return group.trusted;
		case 1:
			return group.name;
		case 2:
			return group.fingerprint;
		case 3:
			return group.memberCount;
		case 4:
			return group.load;
		case 5:
			if (group.member) {
				return "yes";
			} else {
				return "no";
			}
		default:
			return QVariant();
	}
}
QVariant OnlineGroupModel::headerData(int section, Qt::Orientation orientation, int role) const {
	switch (section) {
		case 0:
			return QVariant();
		case 1:
			return QString("Name");
		case 2:
			return QString("Key fingerprint");
		case 3:
			return QString("Members");
		case 4:
			return QString("Load");
		case 5:
			return QString("Group membership");
		default:
			return QVariant();
	}
}
QModelIndex OnlineGroupModel::index(int row, int column, const QModelIndex &parent) const {
	return createIndex(row, column);
}
int OnlineGroupModel::rowCount(const QModelIndex &parent) const {
	if (parent.isValid()) {
		return 0;
	}
	return onlineGroups.count();
}
bool OnlineGroupModel::hasChildren(const QModelIndex &parent) const {
	// The root has children
	if (!parent.isValid()) {
		return true;
	} else {
		return false;
	}
}
