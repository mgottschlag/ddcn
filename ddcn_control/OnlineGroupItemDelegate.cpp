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

#include "OnlineGroupItemDelegate.h"

#include <QApplication>
#include <QPainter>

OnlineGroupItemDelegate::OnlineGroupItemDelegate() {
	if (!trustedIcon.load(":/icons/icons/security-high.png")) {
		qCritical("Could not load trusted icon.");
	}
	untrustedIcon.load(":/icons/icons/security-low.png");
}

void OnlineGroupItemDelegate::paint(QPainter *painter,
		const QStyleOptionViewItem &option, const QModelIndex &index) const {
	if (index.column() == 0) {
		bool trusted = index.data().toBool();
		if (trusted) {
			painter->drawPixmap(0, 0, trustedIcon);
		} else {
			painter->drawPixmap(0, 0, untrustedIcon);
		}
	} else if (index.column() == 4) {
		float load = index.data().toFloat();

		QStyleOptionProgressBar progressBarOption;
		progressBarOption.rect = option.rect;
		progressBarOption.minimum = 0;
		progressBarOption.maximum = 100;
		progressBarOption.progress = qRound(load * 100.0f);
		progressBarOption.text = QString::number(progressBarOption.progress) + "%";
		progressBarOption.textVisible = true;

		QApplication::style()->drawControl(QStyle::CE_ProgressBar,
				&progressBarOption, painter);
	} else {
		QStyledItemDelegate::paint(painter, option, index);
	}
}
QSize OnlineGroupItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
	if (index.column() == 0) {
		return QSize(22, 22);
	} else {
		return QStyledItemDelegate::sizeHint(option, index);
	}
}
