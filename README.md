venus-access controls some services on venus devices needed to access the device.

Some services are disabled by default, like ssh daemon and console access
on a Color Control. They can be enabled with settings. This process makes
sure they are actually started when the setting is enabled. Unlike
venus-platform, it is only responsible for services which are required for
(debug) access, like ssh / console / remote support, making it possible to
enhance / develop venus-platform, without a risk of locking yourself out.


