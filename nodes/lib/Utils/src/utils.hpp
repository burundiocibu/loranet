// -*- coding: utf-8 -*-

// seconds node has been up
double uptime();

// a HH:MM:SS.mmm string of uptime
String runtime();

// compute time interval from millis() timer
// accounts only for a single rollover.
long dt(unsigned long start_time);
