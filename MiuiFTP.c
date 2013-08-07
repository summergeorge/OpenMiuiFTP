#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <process.h>

#define OK 1
#define FOUND 1
#define NOTFOUND 0
#define Status int

#define THREAD_NUM 20
int flag = THREAD_NUM;
HANDLE  ThreadEvent; /** 事件  */

/**
**  查找参数ip地址的2121端口是否开放
**  返回值：FOUND; NOTFOUND;
**/
Status Search_mi(char* ip)
{
    WSADATA wsaData;
    if(0 != WSAStartup(MAKEWORD(2,0),&wsaData)) exit(-1);/** 初始化 winSocket  */
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    SOCKADDR_IN sa;
    sa.sin_addr.s_addr = inet_addr (ip);

    SetEvent(ThreadEvent); /** 触发事件  */

    sa.sin_family=AF_INET;
    sa.sin_port = htons(2121);/** 小米FTP端口2121  */

    int ret = 0,error = -1,len = sizeof(int);
    unsigned long ul = 1;
    struct timeval tout;
    fd_set se;
    ioctlsocket(sock, FIONBIO, &ul); /** 设置为非阻塞模式  */
    if(connect(sock,(SOCKADDR *)&sa,sizeof(sa))== SOCKET_ERROR)
    {
        tout.tv_sec  = 0;
        tout.tv_usec = 500*1000;/** 100毫秒;1秒 == 1000豪秒== 1000微秒;  */
        FD_ZERO(&se);
        FD_SET(sock, &se);
        if( select(sock+1, NULL, &se, NULL, &tout) > 0)
        {
            getsockopt(sock, SOL_SOCKET, SO_ERROR, (char *)&error,&len);
            if(error == 0)
            {
                ret = 1;
            }
            else
            {
                ret = 0;
            }
        }
        else
        {
            ret = 0;
        }
    }
    else
    {
         ret = 1;
    }
    if(1 == ret)
    {
        closesocket(sock);
        WSACleanup();
        return FOUND;
    }
    else
    {
        closesocket(sock);
        WSACleanup();
        return NOTFOUND;
    }
}

/**
**  获取本机IP地址
**  返回值：char* ip;
**/
char* CheckIP(void)
{
	WSADATA wsaData;
	char name[255];/** 主机名  */
	char *ip;

	PHOSTENT hostinfo;

	if (WSAStartup(MAKEWORD(2,0), &wsaData) == 0)
	{
		if(gethostname(name,sizeof(name)) == 0)
		{
		    /** 若成功获得主机名，则返回一个指针，指向hostinfo */
			if((hostinfo = gethostbyname(name)) != NULL)
			{
			    /** inet_ntoa()函数，将hostinfo结构变量中的h_addr_list转化为标准的点分表示的IP */
				ip = inet_ntoa(*(struct in_addr *)*hostinfo->h_addr_list);

				return ip;
			}
		}
		WSACleanup();
	}
    return NULL;
}

/**
**  获取IP地址前3段
**  返回值：Status;
**/
Status IPhead(char* ip)
{
    char *startip = CheckIP();
    strcat(ip,strtok(startip, "."));
    strcat(ip,".");
    strcat(ip,strtok (NULL, "."));
    strcat(ip,".");
    strcat(ip,strtok (NULL, "."));
    strcat(ip,".");

    return OK;
}

unsigned  __stdcall  thread_func(void *ip)
{
    char cmd[50] = "explorer.exe ftp://";
    strcat(cmd,(char *)ip);
    strcat(cmd,":2121");

    if(FOUND == Search_mi(ip))
    {
        system(cmd);
    }
    else
    {
        flag--;
    }

    return 0;
}

void UI(void)
{
    printf("\n\n\n\t\t\t没有找到小米手机！\n\n\n");
    printf("\n请按下面方法检查：\n\n");
    printf("\t1,手机和电脑 连的是同一个路由器吗 ？\n\n");
    printf("\t2,手机上 -> 文件管理 -> 远程管理 -> 启动服务 已经打开了吗？ \n\n");
    printf("\n\t如果你都回答: 是 ！\t还是打不开？请联系作者！！\n\n\t\t\t\thttp://www.yurfly.com\n\n\n");
    system("pause");
}

int main()
{
    SetConsoleTitle("小米无线FTP！");
    ThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    char iphead[16] = "";
    IPhead(iphead);/** 获取网关IP地址前3段  */
    HANDLE  handle[THREAD_NUM];
    int i;
    for(i = 0;i < THREAD_NUM;i++)
    {
        char tmp[4] = "",ip[16] = "";
        strcat(ip,iphead);
        itoa(100 + i,tmp,10);
        strcat(ip,tmp);/** 拼接上IP地址最后一段  */

        /** 开THREAD_NUM个线程进行扫描  */
        handle[i] = (HANDLE)_beginthreadex(NULL,0,thread_func,ip,0,NULL);
        WaitForSingleObject(ThreadEvent, INFINITE); /** 等待事件被触发  */
    }
    WaitForMultipleObjects(THREAD_NUM, handle, TRUE,INFINITE);
    CloseHandle(ThreadEvent);/** 销毁事件  */
    if(0 == flag) UI();

    return 0;
}
