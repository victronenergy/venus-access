#include <QCoreApplication>

#include <veutil/qt/daemontools_service.hpp>
#include <veutil/qt/ve_qitems_dbus.hpp>

class Application : public QCoreApplication
{
	Q_OBJECT

public:
	Application(int &argc, char **argv);

protected slots:
	void checkSshServices();
	void onLocalSettingsStateChanged(VeQItem::State state);
	void onLocalSettingsTimeout();
	void remoteSupportChanged(QVariant var);
	void sshLocalChanged(QVariant var);

private:
	void manageDaemontoolsServices();
	void init();

	VeQItemSettings *mSettings;
	VeQItem *mServices;
	QTimer mLocalSettingsTimeout;

	DaemonToolsService *mOpensshSvc = nullptr;
	DaemonToolsService *mTunnelSvc = nullptr;

	VeQItem *mRemoteSupport;
	VeQItem *mSshLocal;
	VeQItem *mTunnelRequest;
};
