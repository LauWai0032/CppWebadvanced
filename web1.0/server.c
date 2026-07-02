#include "server.h"
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <fcntl.h>
/**
 * @brief 初始化监听套接字
 *
 * 该函数创建一个 TCP 套接字，设置地址复用，绑定指定端口，并开始监听。
 *
 * @param port 监听的端口号 (网络字节序)
 * @return 成功返回监听文件描述符 (lfd)，失败返回 -1
 */
int initListenFd(unsigned short port)
{
  // 1. 创建套接字 (Socket)
  // AF_INET: 使用 IPv4 协议
  // SOCK_STREAM: 使用 TCP 协议 (流式套接字)
  // 0: 协议类型自动推导 (TCP)
  int lfd = socket(AF_INET, SOCK_STREAM, 0);
  if (lfd == -1)
  {
    perror("socket"); // 如果创建失败，打印错误信息
    return -1;
  }

  // 2. 设置套接字选项 (允许地址复用)
  // 目的：解决服务器重启时可能出现的 "Address already in use" 错误
  // SO_REUSEADDR: 允许重用本地地址（端口）
  // 这里使用了 C 语言的复合字面量 (Compound Literal) 来传递整数 1 的地址
  int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
  if (ret == -1)
  {
    perror("setsockopt");
    return -1;
  }

  // 3. 配置服务器地址结构
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;         // 必须设置为 AF_INET
  addr.sin_port = htons(port);       // 将端口号从主机字节序转换为网络字节序
  addr.sin_addr.s_addr = INADDR_ANY; // 绑定任意可用的本地 IP 地址 (0.0.0.0)

  // 4. 绑定 (Bind)
  // 将套接字与本地 IP 和端口关联起来
  ret = bind(lfd, (struct sockaddr *)&addr, sizeof addr);
  if (ret == -1)
  {
    perror("bind");
    return -1;
  }

  // 5. 监听 (Listen)
  // 开始监听连接请求
  // 第二个参数 128 是连接请求队列的最大长度 (Backlog)
  ret = listen(lfd, 128);

  if (ret == -1)
  {
    perror("listen");
    return -1;
  }

  // 6. 返回监听文件描述符
  return lfd;
}

/**
 * @brief 运行 epoll 事件循环
 *
 * 该函数创建 epoll 实例，注册监听套接字，并进入循环处理 I/O 事件。
 *
 * @param lfd 监听套接字文件描述符
 * @return 正常情况下不返回，出错时返回 -1
 */
int epollRun(int lfd)
{
  // 1. 创建 epoll 实例
  // 参数 1 是内核初始监听的文件描述符数量（现在只是一个提示值，内核会动态调整）
  // 返回值 epfd 是 epoll 的文件描述符
  int epfd = epoll_create(1);
  if (-1 == epfd)
  {
    perror("epoll_create");
    return -1;
  }

  // 2. 注册监听套接字到 epoll
  struct epoll_event ev;
  ev.data.fd = lfd;    // 将监听套接字 lfd 关联到事件
  ev.events = EPOLLIN; // 监听可读事件（即有新连接到来）

  int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
  if (-1 == ret)
  {
    perror("epoll_ctl");
    return -1;
  }

  // 3. 准备事件数组，用于接收就绪事件
  int num = 0;
  struct epoll_event evs[1024]; // 栈上分配数组，最多同时处理 1024 个事件
  int size = sizeof(evs) / sizeof(evs[0]);

  // 4. 事件循环
  while (1)
  {
    // 等待文件描述符就绪
    // 第四个参数 -1 表示永久阻塞，直到有事件发生
    num = epoll_wait(epfd, evs, size, -1);

    // 遍历所有就绪的文件描述符
    for (int i = 0; i < num; ++i)
    {
      int fd = evs[i].data.fd;

      // 5. 判断事件来源
      if (fd == lfd)
      {
        acceptClient(lfd, epfd);
        // TODO: 这里应该处理新连接
        // 逻辑：调用 accept 接受新客户端连接，
        // 获取新的通信文件描述符 cfd，
        // 并将 cfd 添加到 epoll 监听列表中 (EPOLL_CTL_ADD)
      }
      else
      {
        // TODO: 这里应该处理已连接套接字的数据读写
        // 逻辑：调用 recv/read 读取数据，
        // 处理业务逻辑，
        // 调用 send/write 发送响应，
        // 如果连接关闭或出错，需要从 epoll 中移除并关闭 fd
      }
    }
  }

  return 0;
}
int acceptClient(int lfd, int epfd)
{
  int cfd = accept(lfd, NULL, NULL);
  if (cfd == -1)
  {
    perror("accept");
    return -1;
  }
  int flag = fcntl(cfd, F_GETFL);
  flag |= O_NONBLOCK;
  fcntl(cfd, F_SETFL, flag);
  struct epoll_event ev;
  ev.data.fd = cfd;
  ev.events = EPOLLIN | EPOLLET;
  int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
  if (-1 == ret)
  {
    perror("epoll_ctl");
    return -1;
  }

  return cfd;
}