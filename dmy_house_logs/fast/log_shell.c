/* Lock the house logs file and run a shell command on it.

   This wrapper assumes that the shell command has no locking
   semantics, hence the locking done by this script.

   Actually, because this is Unix... there are incompatible locking
   conventions, so this script ensures that we use the same,
   compatible locking convention.  On Linux, the `fcntl ()' locks do
   not interact with the `flock ()' locks.  The old Perl
   implementation uses `flock ()' locks under the hood.  */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int
main (int argc, char *argv[])
{
  int fd;
  pid_t child;
  int status;
  struct flock lock_info;

  if (argc < 2) {
    fputs ("Incorrect command line.\n", stderr);
    return 1;
  }

  /* Open with an exclusive write lock, read-write.  */
  fd = open (argv[1], O_RDWR);
  if (fd == -1) {
    perror ("Error opening log file");
    return 1;
  }
  lock_info.l_type = F_WRLCK;
  lock_info.l_whence = SEEK_SET;
  lock_info.l_start = 0;
  lock_info.l_len = 0;
  lock_info.l_pid = 0;
  if (fcntl (fd, F_SETLK, &lock_info) == -1) {
    perror ("Error locking log file");
    close (fd);
    return 1;
  }

  /* Execute our shell command.  */
  child = fork ();
  if (child == 0) {
    /* Child */
    argv += 2;
    execvp (*argv, argv);
    /* If we reach this, there is an error, so exit non-zero.  */
    exit (1);
  }
  /* else if (child == -1) error */
  /* else */
  /* Parent */
  if (child == -1 || waitpid (child, &status, 0) == -1 ||
      !WIFEXITED (status)) {
    perror ("Error executing command");
    return 1;
  }

  /* Close.  */
  if (close (fd) == -1) {
    perror ("Error closing log file");
    return 1;
  }

  return WEXITSTATUS (status);
}
