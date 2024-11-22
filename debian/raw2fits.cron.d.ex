#
# Regular cron jobs for the raw2fits package
#
0 4	* * *	root	[ -x /usr/bin/raw2fits_maintenance ] && /usr/bin/raw2fits_maintenance
