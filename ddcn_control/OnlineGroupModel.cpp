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

#include "OnlineGroupModel.h"

int OnlineGroupModel::columnCount(const QModelIndex &parent) const {
	// Columns: Trusted status, name, key fingerprint, member count, load, member of trusted group
	return 6;
}
QVariant OnlineGroupModel::data(const QModelIndex &index, int role) const {
	// TODO
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
	return QModelIndex();
	// TODO
}
int OnlineGroupModel::rowCount(const QModelIndex &parent) const {
	return 0;
}
