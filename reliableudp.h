#pragma comment(lib, "Ws2_32.lib")
#include<iostream>
#include<WinSock2.h>
#include<string>
#include<string.h>
#include<time.h>
#include<fstream>
#include<stdio.h>
#include<vector>
#include<thread>
#include<mutex>
using namespace std;
SOCKET server, client;
const int timeout = 200;//��ʱ�趨
const int datalen = 14994;//���ݰ����ݳ���
const int pkgheaderlen = 6;//���ݰ�ͷ������
const int pkglength = 15000;//���ݰ�����
const int maxn = 2e8;//�ڴ�����󻺴����ݰ�����
SOCKADDR_IN serverAddr, clientAddr;
unsigned char checksum(char* ch, int len)
{
	if (len == 0) return 0xff;
	unsigned char cksum = ch[0];
	for (int i = 1; i < len; i++)
	{
		unsigned int tmp = cksum + (unsigned char)ch[i];
		tmp = (tmp >> 8) + (tmp & 0xff);
		tmp = (tmp >> 8) + (tmp & 0xff);
		cksum = tmp;
	}
	return ~cksum;
}