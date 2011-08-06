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

#include "CompilerServiceAdaptor.h"
#include "CompilerNetworkAdaptor.h"
#include "DBusStructs.h"

#include <QCoreApplication>
#include <QDBusConnection>

int main(int argc, char **argv) {
	QCoreApplication app(argc, argv);
	// Register meta types
	registerCustomDBusTypes();
	qRegisterMetaType<ariba::utility::NodeID>();
	qRegisterMetaType<ariba::utility::LinkID>();
	qRegisterMetaType<ariba::DataMessage>();
	// Initialize crypto framework
	TLS::initialize();
	// Create the compiler network
	CompilerNetwork network;
	new CompilerNetworkAdaptor(&network);
	logging_rootlevel_error();
	// Create the compiler service
	CompilerService service(&network);
	CompilerServiceAdaptor *serviceAdaptor = new CompilerServiceAdaptor(&service);
	QObject::connect (serviceAdaptor,
			 SIGNAL(onShutdownRequest()),
			 &app,
			 SLOT(quit())
	);
	// Create compiler network status interface
	// TODO
	// Connect compiler service and compiler network
	// TODO
	// Create the D-Bus interface
	QDBusConnection::sessionBus().registerObject("/CompilerService", &service);
	QDBusConnection::sessionBus().registerObject("/CompilerNetwork", &network);
	QDBusConnection::sessionBus().registerService("org.ddcn.service");
	// Start the event loop
	return app.exec();
}
