#!/usr/bin/python2.5

import time

import gobject

import dbus
import dbus.mainloop.glib

def emit_notification(iface):
	id = iface.Notify ('test-send.py', '0', 'qgn_list_messagin', 'Jan Arne Petersen', 'Subject 1', ['default', 'default'], { 'category': 'email-message', 'time': dbus.Int64 (time.time() - 1800) }, 0)
	print 'notification emmited. id: ' + str(id)

	return True

def notification_closed_handler(id):
	print 'notification closed. id: ' + str(id)

def action_invoked_handler(id, action_key):
	print 'action invoked. id: ' + str(id) + ', action_key: ' + action_key

if __name__ == '__main__':
	dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

	bus = dbus.SessionBus()

	try:
		proxy = bus.get_object('org.freedesktop.Notifications', '/org/freedesktop/Notifications')
		iface = dbus.Interface(proxy, 'org.freedesktop.Notifications')
		proxy.connect_to_signal('NotificationClosed', notification_closed_handler, dbus_interface='org.freedesktop.Notifications')
		proxy.connect_to_signal('ActionInvoked', action_invoked_handler, dbus_interface='org.freedesktop.Notifications')
	except dbus.DBusException:
		traceback.print_exc()

# iface.SystemNoteInfoprint ('foo1');
# iface.SystemNoteInfoprint ('foo2');
# iface.SystemNoteInfoprint ('foo3');

# iface.Notify ('test-send.py', '0', 'qgn_list_messagin', 'Jan Arne Petersen', 'Subject', ['default', 'default'], { 'category': 'email' }, 0)

	gobject.timeout_add(10000, emit_notification, iface)

	loop = gobject.MainLoop()
	loop.run()
