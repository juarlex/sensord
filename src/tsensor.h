#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <syslog.h>
#include <sqlite3.h>

struct tsensor {
  float temperature;
  int   status;
  int   sensor_id;
  char  *device;
};

void log_temp_from_tsensors(sqlite3 *db);
