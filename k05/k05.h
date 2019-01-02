#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/slab.h>
#include<linux/vmalloc.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<asm/uaccess.h>
#include<linux/types.h>
#include<linux/moduleparam.h>
#include<linux/pci.h>
#include<asm/unistd.h>
#include<linux/device.h>
#include<linux/sched.h>
#include<linux/pid.h>
#include<linux/mm_types.h>
//get_user_pages
#include<linux/mm.h>
#include<linux/security.h>
#include<linux/pagemap.h>
//kmap,kunmap
#include<linux/highmem.h>
//sleep_on_timeout
#include<linux/wait.h>
//kthread_run
#include<linux/kthread.h>
#include<linux/version.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("tybitsfox ([email]tybitsfox@126.com[/email])");
MODULE_DESCRIPTION("kernel memory access module.");
//#define CLONE_KERNEL 	(CLONE_FS | CLONE_FILES | CLONE_SIGHAND)

//{{{ --所用结构的定义---
/*下面定义的联合用于描述地址结构
off:表示页内偏移，不超过4096（12位即可）
seg:表示所在的段，0：代码段，1：数据段，2：堆段，3：堆栈段
page:表示页索引，本次修改后，页索引最大60000,为240M，再加上4个段
最多可扫描1G的空间。
*/  
union OFFSET
{
	struct{
	unsigned int off:12;
	unsigned int page:16;
	unsigned int seg:4;
	};
	unsigned int ad;
	//unsigned char adch[4];
};
/*下面定义的结构用于描述锁定地址和上下限的*/
struct KVAR_LOCK
{
	union{
	unsigned int maxd;			//上限
	unsigned char mxc[4];
	};
	union{
	unsigned int mind;			//下限
	unsigned char mic[4];
	};
	union OFFSET offset;		//锁定地址
};
/*下面定义的结构用于描述命令头结构的*/
struct KVAR_AM
{
	unsigned char sync;				//同步标志，用户端置0,内核端置1。
	unsigned char cmd;				//命令字节, 0:无操作，1：首次查询命令，2：再次查询命令，3：锁定命令。
	unsigned char end0;				//本次操作完成标志。
	unsigned char end1;				//再次查询时由内核设置，区分是向外传送结果（0）还是要求用户继续输入地址集(1)。
	union{
	unsigned int pid;				//由用户传入的待搜索的进程pid
	unsigned char pch[4];
	};
	union{
	unsigned int snum;				//由用户传入的搜索数值。
	unsigned char sch[4];
	};
	unsigned int fin_pg;			//由内核传出的已搜索完的页数
	unsigned int tol_pg;			//由内核传出的总的搜索页数
	unsigned int t_seg;				//由内核传出（首次搜索）的代码段基地址。
	unsigned int d_seg;				//由内核传出（首次搜索）的数据段基地址。
	unsigned int b_seg;				//由内核传出（首次搜索）的已分配的堆的基地址。
	unsigned int s_seg;				//由内核传出（首次搜索）的堆栈断地址。
	unsigned int t_len;				//代码段长度，该字段保存与锁定地址的文件中，用于确定锁定的目标进程是否相符。
	unsigned int d_len;				//数据段长度，同上。
//到此为44字节了（0base）
//2016-10-29修改，原来的锁定个数限制为8个，现修改为最大16个，不过该数据结构不做改变，只是在使用时取得ladr[0]的地址
//按该地址连续取最多16个锁定。	
	struct KVAR_LOCK ladr[8];		//在锁定操作中，传入内核的最多8个锁定地址和数据。共96字节，总计使用了44+96=140字节
	unsigned char vv[52];			//补齐56字节，该结构总长为192字节，为8K（8192）缓冲的前192字节	
};
/*下面定义的结构用于描述和保存内核中使用的一些关键标志*/
struct KVAR_TY
{
	unsigned int thread_lock;		//线程运行锁。=1线程正在运行。
	unsigned int pid;				//保存传入的pid的副本。
	union{
	unsigned int snum;				//保存的待搜索的数字。
	unsigned char sch[4];
	};
	unsigned int fpg;				//已搜索完成的页数
	unsigned int tpg;				//总的页数
	unsigned int seg[4];			//4个搜索的段基地址。
	unsigned int t_len;					//代码段长度。
	unsigned int d_len;					//数据段长度。
	unsigned int pin;				//输出缓冲区的确定标志。
};
//2016-10-29定义新的结构，用户数据的锁定操作，该结构将传入的锁定地址和vma挂钩，同属于一个vma的放入同一个结构中
struct KLOCK_STR
{
	struct vm_area_struct *vm;
	struct KVAR_LOCK lk[16];
	char flg[16];
};
//}}}


//{{{ --所用常量的定义
//2017-6-23 增大缓冲区AREA_SIZE由原来的1000*1024改为2000*1024,K_CC_SIZE 4*1024*1024->8*1024*1024,K_BF_SIZE 4*1024*100->8*1024*100
//AREA_SIZE_T 90*1024->180*1024
#define AREA_SIZE			2000*1024
#define K_BUFFER_SIZE		8192
#define DRV_MAJOR			247
#define DRV_MINOR			0
#define drv_name			"memacc_dev0"
#define d_begin				192
#define dlen				8000
//下面的常量是定义内核保存搜索地址的缓冲区大小，2016-10-27定义
#define K_CC_SIZE			8*1024*1024
#define K_BF_SIZE			8*1024*100
#define AREA_SIZE_T			180*1024
//}}}

//{{{  --全局变量的定义
unsigned char *mp,*tp;			//定义的两个缓冲区指针
struct KVAR_TY kv1;		//
struct KVAR_AM *k_am,*k_tp;   //
struct class *mem_class;
unsigned char suc=0;
///2016-10-27添加
unsigned char *kcc=NULL; //在内核中保存首次搜索的地址，最多保存100万个 4*1024*1024
int kcc_cnt=0;
struct KLOCK_STR kloc_s[2];//最多两个vma
//}}}

//{{{ --函数定义
static int __init minedev_init(void);
static void __exit minedev_exit(void);
static int mem_open(struct inode *ind,struct file *filp);
static int mem_release(struct inode *ind,struct file *filp);
static ssize_t mem_read(struct file *filp,char __user *buf,size_t size,loff_t *fpos);
static ssize_t mem_write(struct file *filp,const char __user *buf,size_t size,loff_t *fpos);
static void class_create_release(struct class *cls);
struct file_operations mem_fops=
{
	.owner=THIS_MODULE,
	.open=mem_open,
	.release=mem_release,
	.read=mem_read,
	.write=mem_write,
};
static int first_srh(void *argc);
static int next_srh(void *argc);
static int lock_srh(void *argc);
static void s_sync(int t);
static void s_wait_mm(int w);
static int mem_srch(void *argc);
static int mem_cmp(void *argc);



//}}}





