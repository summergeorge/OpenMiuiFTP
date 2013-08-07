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
HANDLE  ThreadEvent; /** �¼�  */

/**
**  ���Ҳ���ip��ַ��2121�˿��Ƿ񿪷�
**  ����ֵ��FOUND; NOTFOUND;
**/
Status Search_mi(char* ip)
{
    WSADATA wsaData;
    if(0 != WSAStartup(MAKEWORD(2,0),&wsaData)) exit(-1);/** ��ʼ�� winSocket  */
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    SOCKADDR_IN sa;
    sa.sin_addr.s_addr = inet_addr (ip);

    SetEvent(ThreadEvent); /** �����¼�  */

    sa.sin_family=AF_INET;
    sa.sin_port = htons(2121);/** С��FTP�˿�2121  */

    int ret = 0,error = -1,len = sizeof(int);
    unsigned long ul = 1;
    struct timeval tout;
    fd_set se;
    ioctlsocket(sock, FIONBIO, &ul); /** ����Ϊ������ģʽ  */
    if(connect(sock,(SOCKADDR *)&sa,sizeof(sa))== SOCKET_ERROR)
    {
        tout.tv_sec  = 0;
        tout.tv_usec = 500*1000;/** 100����;1�� == 1000����== 1000΢��;  */
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
**  ��ȡ����IP��ַ
**  ����ֵ��char* ip;
**/
char* CheckIP(void)
{
	WSADATA wsaData;
	char name[255];/** ������  */
	char *ip;

	PHOSTENT hostinfo;

	if (WSAStartup(MAKEWORD(2,0), &wsaData) == 0)
	{
		if(gethostname(name,sizeof(name)) == 0)
		{
		    /** ���ɹ�������������򷵻�һ��ָ�룬ָ��hostinfo */
			if((hostinfo = gethostbyname(name)) != NULL)
			{
			    /** inet_ntoa()��������hostinfo�ṹ�����е�h_addr_listת��Ϊ��׼�ĵ�ֱ�ʾ��IP */
				ip = inet_ntoa(*(struct in_addr *)*hostinfo->h_addr_list);

				return ip;
			}
		}
		WSACleanup();
	}
    return NULL;
}

/**
**  ��ȡIP��ַǰ3��
**  ����ֵ��Status;
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
    printf("\n\n\n\t\t\tû���ҵ�С���ֻ���\n\n\n");
    printf("\n�밴���淽����飺\n\n");
    printf("\t1,�ֻ��͵��� ������ͬһ��·������ ��\n\n");
    printf("\t2,�ֻ��� -> �ļ����� -> Զ�̹��� -> �������� �Ѿ������� \n\n");
    printf("\n\t����㶼�ش�: �� ��\t���Ǵ򲻿�������ϵ���ߣ���\n\n\t\t\t\thttp://www.yurfly.com\n\n\n");
    system("pause");
}

int main()
{
    SetConsoleTitle("С������FTP��");
    ThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    char iphead[16] = "";
    IPhead(iphead);/** ��ȡ����IP��ַǰ3��  */
    HANDLE  handle[THREAD_NUM];
    int i;
    for(i = 0;i < THREAD_NUM;i++)
    {
        char tmp[4] = "",ip[16] = "";
        strcat(ip,iphead);
        itoa(100 + i,tmp,10);
        strcat(ip,tmp);/** ƴ����IP��ַ���һ��  */

        /** ��THREAD_NUM���߳̽���ɨ��  */
        handle[i] = (HANDLE)_beginthreadex(NULL,0,thread_func,ip,0,NULL);
        WaitForSingleObject(ThreadEvent, INFINITE); /** �ȴ��¼�������  */
    }
    WaitForMultipleObjects(THREAD_NUM, handle, TRUE,INFINITE);
    CloseHandle(ThreadEvent);/** �����¼�  */
    if(0 == flag) UI();

    return 0;
}
