select_test = """
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>

int main(void)
{
  fd_set rfds, wfds;
  struct timeval tv;
  int i, maxfd, pfd[2];
  const char *str = "Hello world";
  char buf[100];

  if (pipe(pfd) < 0)
    exit(-1);

  maxfd = pfd[0] < pfd[1] ? pfd[1] : pfd[0];

  FD_SET(pfd[0], &rfds);
  FD_SET(pfd[1], &wfds);

  i = write(pfd[1], str, strlen(str)); 
  assert(i == strlen(str));
  i = read(pfd[0], buf, sizeof(buf));
  assert(i == strlen(str));

  tv.tv_sec = 0;
  tv.tv_usec = 0;
  int nfds = select(maxfd + 1, &rfds, &wfds, NULL, &tv);
  if (nfds <= 0)
    exit(-1);

  return 0;  
}
"""
def CheckSelect(context):
  context.Message("Checking for select...")
  result = context.TryLink(select_test, '.c')
  context.Result(result)
  return result
