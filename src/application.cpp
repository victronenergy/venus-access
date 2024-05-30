#include <veutil/qt/daemontools_service.hpp>
#include <veutil/qt/ve_dbus_connection.hpp>

#include "application.hpp"

class SettingsInfo : public VeQItemSettingsInfo
{
public:
	SettingsInfo()
	{
		if (QFile("/dev/ttyconsole").exists())
			add("Services/Console", 0, 0, 0);
		add("System/RemoteSupport", 0, 0, 1);
		add("System/SSHLocal", 0, 0, 1);
	}
};

Application::Application::Application(int &argc, char **argv) :
	QCoreApplication(argc, argv),
	mLocalSettingsTimeout()
{
	QDBusConnection dbus = VeDbusConnection::getConnection();
	if (!dbus.isConnected()) {
		qCritical() << "DBus connection failed";
		::exit(EXIT_FAILURE);
	}

	VeQItemDbusProducer *producer = new VeQItemDbusProducer(VeQItems::getRoot(), "dbus", false, false);
	producer->setAutoCreateItems(false);
	producer->open(dbus);
	mServices = producer->services();
	mSettings = new VeQItemDbusSettings(mServices, QString("com.victronenergy.settings"));

	VeQItem *item = mServices->itemGetOrCreate("com.victronenergy.settings");
	connect(item, SIGNAL(stateChanged(VeQItem::State)), SLOT(onLocalSettingsStateChanged(VeQItem::State)));
	if (item->getState() == VeQItem::Synchronized) {
		qDebug() << "Localsettings found";
		init();
	} else {
		qDebug() << "Localsettings not found";
		mLocalSettingsTimeout.setSingleShot(true);
		mLocalSettingsTimeout.setInterval(120000);
		connect(&mLocalSettingsTimeout, SIGNAL(timeout()), SLOT(onLocalSettingsTimeout()));
		mLocalSettingsTimeout.start();
	}
}

void Application::onLocalSettingsStateChanged(VeQItem::State state)
{
	mLocalSettingsTimeout.stop();

	if (state != VeQItem::Synchronized) {
		qCritical() << "Localsettings not available" << state;
		::exit(EXIT_FAILURE);
	}

	qDebug() << "Localsettings appeared";
	init();
}

void Application::onLocalSettingsTimeout()
{
	qCritical() << "Localsettings not available!";
	::exit(EXIT_FAILURE);
}

void Application::manageDaemontoolsServices()
{
	if (QFile("/dev/ttyconsole").exists())
		new DeamonToolsConsole(mSettings, "/service/vegetty", "Settings/Services/Console", this, QStringList() << "-s" << "vegetty");

	mOpensshSvc = new DaemonToolsService("/service/openssh", this);
	mOpensshSvc->setSveCtlArgs(QStringList() << "-s" << "openssh");
	mOpensshSvc->setRestart(false);

	mTunnelSvc = new DaemonToolsService("/service/ssh-tunnel", this);
	mTunnelSvc->setSveCtlArgs(QStringList() << "-s" << "ssh-tunnel");
	mTunnelSvc->setRestart(false);
}

void Application::checkSshServices()
{
	bool tunnelUp = mRemoteSupport->getLocalValue().toBool() == true ||
					mTunnelRequest->getLocalValue().toBool() == true;
	bool sshUp = tunnelUp || mSshLocal->getLocalValue().toBool() == true;

	mOpensshSvc->installOrRemove(sshUp);
	mTunnelSvc->installOrRemove(tunnelUp);
}

void Application::remoteSupportChanged(QVariant var)
{
	checkSshServices();

	if (!var.isValid())
		return;

	if (var.toBool())
		QFile::link("/usr/share/support-keys/authorized_keys",
				"/run/ssh_support_keys");
	else
		QFile::remove("/run/ssh_support_keys");
}

void Application::sshLocalChanged(QVariant var)
{
	checkSshServices();

	if (!var.isValid())
		return;

	if (var.toBool())
		(void) system("firewall allow tcp ssh");
	else
		(void) system("firewall deny tcp ssh");
}

void Application::init()
{
	qDebug() << "Creating settings";
	if (!mSettings->addSettings(SettingsInfo())) {
		qCritical() << "Creating settings failed";
		::exit(EXIT_FAILURE);
	}

	manageDaemontoolsServices();

	// Remote support
	mRemoteSupport = mSettings->root()->itemGetOrCreate("Settings/System/RemoteSupport");

	// SSH daemon on LAN
	mSshLocal = mSettings->root()->itemGetOrCreate("Settings/System/SSHLocal");

	// Venus-platform evaluates more complex tunnel request...
	mTunnelRequest = mServices->itemGetOrCreate("com.victronenergy.platform/ConnectVrmTunnel");

	// mind the order, request the values _after_ all items are created.
	mRemoteSupport->getValueAndChanges(this, SLOT(remoteSupportChanged(QVariant)));
	mSshLocal->getValueAndChanges(this, SLOT(sshLocalChanged(QVariant)));
	mTunnelRequest->getValueAndChanges(this, SLOT(checkSshServices()));
}
