kqueue_test = """
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/event.h>
#include <sys/time.h>

int main(void)
{
  int i, kqfd, pfd[2];
  struct kevent ke, kev[2];
  const char *str = "Hello world";
  char buf[100];

  if (pipe(pfd) < 0)
    exit(-1);

  kqfd = kqueue();
  if (kqfd < 0)
    exit(-1);

  EV_SET(&ke, pfd[0], EVFILT_READ, EV_ADD, 0, 0, NULL);
  EV_SET(&ke, pfd[1], EVFILT_WRITE, EV_ADD, 0, 0, NULL);

  i = write(pfd[1], str, strlen(str));
  assert(i == strlen(str));
  i = read(pfd[0], buf, sizeof(buf));
  assert(i == strlen(str));

  int nfds = kevent(kqfd, NULL, 0, kev, 2, NULL);
  if (nfds <= 0)
    exit(-1);

  close(kqfd);

  return 0;
}
"""
def CheckKqueue(context):
  context.Message("Checking for kqueue...")
  result = context.TryLink(kqueue_test, '.c')
  context.Result(result)
  return result
