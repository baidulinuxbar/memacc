#include"tl07.h"

int main(int argc,char** argv)
{
	ws.g_ch=(unsigned char*)malloc(buf_size);
	if(ws.g_ch==NULL)
	{
		printf("malloc error\n");
		return 0;
	}
	ws.g_addr[0]=(unsigned char*)malloc(adr_buf1);
	if(ws.g_addr[0]==NULL)
	{
		free(ws.g_ch);
		printf("malloc error1\n");
		return 0;
	}
	ws.g_addr[1]=(unsigned char*)malloc(adr_buf2);
	if(ws.g_addr[1]==NULL)
	{
		free(ws.g_ch);
		free(ws.g_addr[0]);
		printf("malloc error2\n");
		return 0;
	}
	ws.g_addr[2]=(unsigned char*)malloc(adr_buf2);
	if(ws.g_addr[2]==NULL)
	{
		free(ws.g_ch);
		free(ws.g_addr[0]);
		free(ws.g_addr[1]);
		printf("malloc error2\n");
		return 0;
	}
	crt_window(argc,argv);
	free(ws.g_ch);
	free(ws.g_addr[0]);
	free(ws.g_addr[1]);
	free(ws.g_addr[2]);
	return 0;
}



















