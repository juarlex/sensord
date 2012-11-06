#include "tsensor.h"

static void log_temp_per_tsensor(sqlite3 *db, struct tsensor *sensor);
static void temp_per_tsensor(struct tsensor *sensor);
static void temp_as_string(char *t, char *buff);
static float string_as_temp(char *s);
static int str_as_status(char *buff);

void log_temp_from_tsensors(sqlite3 *db) {
  char         *sql = "SELECT id, device FROM sensors";
  sqlite3_stmt *stmt = NULL;
  int          rc;

  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
  if( rc != SQLITE_OK ) {
    syslog(LOG_INFO, "sensord: %s,", sqlite3_errmsg(db));
  }

  while( sqlite3_step(stmt) == SQLITE_ROW ) {
    struct tsensor sensor;
    sensor.sensor_id = (int)sqlite3_column_int(stmt, 0);
    sensor.device = (char *)sqlite3_column_text(stmt, 1);
    log_temp_per_tsensor(db, &sensor);
  }

  sqlite3_finalize(stmt);
}

static void log_temp_per_tsensor(sqlite3 *db, struct tsensor *sensor) {
  int rc;
  sqlite3_stmt *stmt;

  temp_per_tsensor(sensor);
  char *sql = "INSERT INTO logs (sensor_id, temperature, sensor_status) VALUES (?, ?, ?)";

  rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

  if ( rc != SQLITE_OK)
    syslog(LOG_WARNING, "Invalid query: %s", sql);

  sqlite3_bind_int(stmt, 1, sensor->sensor_id);
  sqlite3_bind_double(stmt, 2, sensor->temperature);
  sqlite3_bind_int(stmt, 3, sensor->status);
  rc = sqlite3_step(stmt);
  if (( rc != SQLITE_DONE )&&( rc != SQLITE_ROW ))
    syslog(LOG_WARNING, "I can't execute this query: %s", sql);

  sqlite3_finalize(stmt);
}

static void temp_per_tsensor(struct tsensor *sensor) {
  FILE *fd = fopen(sensor->device, "rb");

  if (fd == NULL)
    syslog(LOG_WARNING, "Failed to open device: %s\n", sensor->device);
  else {
    int line = 0;
		int status = 1;
    char buff[1024];
    char temp_str[1024];

    fseek(fd, 0, SEEK_SET);
    while(!feof(fd))
    {
      memset(buff, 0x00, 1024); // clean buffer

      if (line == 0 ) {
        fscanf(fd, "%[^\n]\n", buff);
        status = str_as_status(buff);
      }

      if (line == 1) {
        fscanf(fd, "%[^\n]\n", buff);
        temp_as_string(temp_str, buff);
      }
      line++;
    }
    fclose(fd);

    sensor->temperature = string_as_temp(temp_str);
    sensor->status = status;
  }
}

static void temp_as_string(char *t, char *buff) {
  char *s = strrchr(buff, '=') + 1;
  size_t l = sizeof(s) <= sizeof(t) ? sizeof(s) : sizeof(t);
  strncpy(t, s, l);
}

static float string_as_temp(char *s) {
  float t = 0;

  if (isdigit(*s))
    t = atoi(s) / 1000.0; // Returns temp in celsius
  else
    syslog(LOG_INFO, "Invalid string can't be converted to temperature: %s", s);

  return t;
}

static int str_as_status(char *buff) {
  int status = 1;
  char *s = strrchr(buff, ' ') + 1;

  if (strcmp(s, "YES"))
    status = 0;

  return status;
}
