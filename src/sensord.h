#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <syslog.h>
#include <sqlite3.h>

#define SENSORD_USER "_sensord"
#define INTVL 10 // Interval to sensors checking each INTVL seconds
#define DEBUG 0

struct sensord_conf {
  int  intvl;
  int  debug;
  char *path_db;
  char *path_sensord_pid;
  char *user;
};
