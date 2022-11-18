#include <QCoreApplication>

#include <velib/qt/ve_qitems_dbus.hpp>

class Application : public QCoreApplication
{
	Q_OBJECT

public:
	Application(int &argc, char **argv);

protected slots:
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
};
