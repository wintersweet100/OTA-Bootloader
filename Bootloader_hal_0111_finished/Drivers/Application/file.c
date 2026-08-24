
#include "file.h"

static struct stat g_rcv_st;
extern unsigned int get_download_address(void);

int fileno(FILE *stream)
{
	return 123;
}

int fstat(int fd, struct stat *statbuf)
{
	*statbuf = g_rcv_st;
	return 0;
}

FILE *popen(const char *command, const char *type)
{
	return (FILE*)1;
}


int pclose(FILE *stream)
{
	return 0;
}

unsigned int sleep(unsigned int seconds)
{
	return 0;
}

//初始化了一个接收数据结构 g_rcv_st，用于处理文件下载或数据接收
//该函数就是用于接收上位机发来的.bin文件，get_download_address是目标地址，我设置的是内存的起始地址0x2000 0000
//不对，这个函数明显只是简单设置啊，根本没有所谓的复制进内存
FILE *fopen2(const char *pathname, const char *mode)
{
	//g_rcv_st.datas   = (unsigned char *)0x20000000;
	g_rcv_st.datas   = (unsigned char *)get_download_address();
	g_rcv_st.offset  = 0;
	g_rcv_st.st_size = 0;
	return (FILE *)1;
}

size_t fwrite2(const void *ptr, size_t size, size_t nmemb,
			  FILE *stream)
{
	unsigned char *pdata = (unsigned char *)ptr;
	int i;
	for (i = 0; i < size; i++)
		g_rcv_st.datas[g_rcv_st.offset++] = pdata[i];

	g_rcv_st.st_size = g_rcv_st.offset;
	return size;
}

			  
int putc2(int c, FILE *stream)
{
	g_rcv_st.datas[g_rcv_st.offset++] = c;
	g_rcv_st.st_size = g_rcv_st.offset;
	return c;
}

int get_file_size(void)
{
	return g_rcv_st.st_size;
}

