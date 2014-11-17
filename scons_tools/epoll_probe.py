epoll_test = """
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/time.h>

int main(void)
{
  int i, epfd, pfd[2];
  struct epoll_event ee[2], ee2[2];
  const char *str = "Hello world";
  char buf[100];

  if (pipe(pfd) < 0)
    exit(-1);

  epfd = epoll_create(5);
  if (epfd < 0)
    exit(-1);

  ee[0].events = EPOLLIN;
  ee[0].data.fd = pfd[0];
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, pfd[0], &ee[0]) < 0)
    exit(-1);

  ee[1].events = EPOLLOUT;
  ee[1].data.fd = pfd[1];
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, pfd[1], &ee[1]) < 0)
    exit(-1);

  i = write(pfd[1], str, strlen(str)); 
  assert(i == strlen(str));
  i = read(pfd[0], buf, sizeof(buf));
  assert(i == strlen(str));

  int nfds = epoll_wait(epfd, ee2, 2, 0);
  if (nfds <= 0)
    exit(-1);

  close(epfd);

  return 0;  
}
"""
def CheckEpoll(context):
  context.Message("Checking for epoll...")
  result = context.TryLink(epoll_test, '.c')
  context.Result(result)
  return result
