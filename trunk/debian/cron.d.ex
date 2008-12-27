#
# Regular cron jobs for the whiteboard package
#
0 4	* * *	root	[ -x /usr/bin/whiteboard_maintenance ] && /usr/bin/whiteboard_maintenance
