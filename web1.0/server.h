#pragma once
int initListenFd(unsigned short port);
int epollRun(int lfd);