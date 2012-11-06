#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/file.h>
#include "sensord.h"
#include "pathnames.h"
#include "tsensor.h"

int main(int argc, char **argv);
void sighdlr(int);
pid_t sensord_main(int pipe_prnt[2], struct passwd *pw, struct sensord_conf conf);
void sensord_sighdlr(int);
void usage(void);

volatile sig_atomic_t quit = 0;
volatile sig_atomic_t reconfig = 0;
volatile sig_atomic_t sigchld = 0;
volatile sig_atomic_t report = 0;

int main(int argc, char **argv) {

  struct passwd *pw;
  struct sensord_conf conf;
  int pipe_chld[2], ch;

  conf.intvl = INTVL;
  conf.path_db = PATH_DB;
  conf.path_sensord_pid = PATH_SENSORD_PID;
  conf.user = SENSORD_USER;
  conf.debug = DEBUG;

  while ((ch = getopt(argc, argv, ":i::d::u::")) != -1) {
    switch (ch) {
      case 'i':
        conf.intvl = atoi(optarg);
        break;
      case 'd':
        conf.path_db = optarg;
        break;
      case 'u':
        conf.user = optarg;
        break;
      default:
        usage();
    }
  }

  if (geteuid())
    printf("need root privileges\n");

  if ((pw = getpwnam(conf.user)) == NULL)
    fprintf(stderr, "unknown user %s\n", conf.user);

  signal(SIGCHLD, sighdlr);

  /* fork child process */
  sensord_main(pipe_chld, pw, conf);
  signal(SIGTERM, sighdlr);
  signal(SIGINT, sighdlr);
  signal(SIGHUP, sighdlr);
  close(pipe_chld[1]);

  return 0;
}

void sighdlr(int sig)
{
  switch (sig) {
    case SIGTERM:
    case SIGINT:
      quit = 1;
      break;
    case SIGCHLD:
      sigchld = 1;
      break;
    case SIGHUP:
      reconfig = 1;
      break;
  }
}

pid_t sensord_main(int pipe_prnt[2], struct passwd *pw, struct sensord_conf conf)
{
  pid_t pid;

  switch (pid = fork()) {
    case -1:
      printf("cannot fork\n");
      break;
    case 0:
      break;
    default:
      return pid;
  }

  int pid_file = open(conf.path_sensord_pid, O_CREAT | O_RDWR, 0666);
  int fl = flock(pid_file, LOCK_EX | LOCK_NB);

  if (fl) {
    if (errno == EACCES || errno == EAGAIN) {
      printf("Another instance of sensord is running, kill the process and delete the file %s.\n", conf.path_sensord_pid);
      exit(1);
    }
  } else {

    close(pipe_prnt[0]);

    if (setgroups(1, &pw->pw_gid) ||
        setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
        setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid)) {
      printf("can't drop privileges\n");
      exit(1);
    }

    signal(SIGTERM, sensord_sighdlr);
    signal(SIGINT, sensord_sighdlr);
    signal(SIGPWR, sensord_sighdlr);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGCHLD, SIG_DFL);

    sqlite3         *db = NULL;
    int             rc;

    syslog(LOG_NOTICE, "sensord started by user %d", getuid ());
    sqlite3_initialize();
    rc = sqlite3_open(conf.path_db, &db);

    if (rc) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      exit(1);

    } else {
      while (quit == 0) {
        log_temp_from_tsensors(db);
        sleep(conf.intvl);
      }
      sqlite3_close(db);
      sqlite3_shutdown();
      exit(0);
    }
  }
  return pid;
}

void sensord_sighdlr(int sig)
{
  switch (sig) {
    case SIGINT:
    case SIGTERM:
      quit = 1;
      break;
    case SIGPWR:
      report = 1;
      break;
  }
}

void usage(void)
{
  extern char *__progname;
  (void)fprintf(stderr, "usage:		%s [-i interval_in_seconds] [-d /path/db] [-u non-root-user]\n", __progname);
  exit(1);
}

