#include"clsscr.h"
#include<gtk/gtk.h>
#define max_res			16
struct win_struct
{
	GtkWidget *window;
	GtkWidget *fixed;
	GtkWidget *scroll[2];
	GtkWidget *list[3];
	GtkListStore *store[3];
	GtkWidget *bnt[10];
	GtkWidget *entry[5];
	GtkWidget *radio[2];
	GtkWidget *prog;
	GtkStatusIcon *sicon;
	GtkWidget *menu;
	GtkWidget *menu_restore;
	GtkWidget *menu_set;
	GtkWidget *menu_exit;
	GtkWidget *menu_mem;
	GtkWidget *label;
//下列定义与模块及线程相关	
	int		  thread_lock;//线程锁
	unsigned char	  *g_ch;		//通讯缓冲
	unsigned char	  *g_addr[3]; //两个结果地址缓冲,2016-11-5修改为3
	int		  pid;		//进程pid
	int		  sn;		//查找数字
	unsigned int  dseg[4];
	unsigned int  seg_len[2];
	GtkWidget *mbnt[4];
	GtkWidget *darea;
	int		  mend_srh;//内存检索线程的退出标志。
//	unsigned int  bseg;//text seg
	int		  show_mode;//=0显示不同的值，=1显示变小的，=2显示变大的
	GtkWidget *combo;
	GtkWidget *mlist;
	int		  blur_srh;
	GtkWidget *label_nn;
//2016-11-4添加内存比较界面的控件
	GtkWidget *menu_mmem;
	GtkWidget *mmlist;
	GtkWidget *mmbnt[4];
	GtkWidget *label_mm;
};
struct win_struct ws;
//2014-1-13新的union替代了两个结构：struct SEG_DESC struct KVAR_SAD
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
//{{{定义一个与模块相关的结构。
/*struct SEG_DESC
{
    unsigned short pbit:15;         //页占15位
    unsigned short sbit:1;          //段占1位，=0表示代码段，=1数据段。
};
struct KVAR_SAD
{
    union{
        struct SEG_DESC s_b;
        unsigned short  spg;
    };  
    union{
        unsigned short off;
        unsigned char  ofch[2];
    };  
};*///现在的结构一次可传送2000地址。}}}
//2014-1-13新的结构替代了struct KVAR_ADDR
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
	unsigned char cmd;				//命令字节, 0:无操作，1：首次查询命令，2：再次查询命令，3：锁定命令。4：内存检视，5：内存比较
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
	struct KVAR_LOCK ladr[8];		//在锁定操作中，传入内核的最多8个锁定地址和数据。共96字节，总计使用了44+96=140字节
	unsigned char vv[52];			//补齐56字节，该结构总长为192字节，为8K（8192）缓冲的前192字节	
};
///{{{struct KVAR_ADDR
/*{
    unsigned int maxd;          //上限
    unsigned int mind;          //下限
    struct KVAR_SAD ksa;        //地址结构
    unsigned char vv0[4];       //无用，使结构为16字节
};
struct KVAR_AM
{
    unsigned char sync;                 //命令字段中0字节的同步标志，
    unsigned char cmd;                  //命令字段中1字节的主命令标志
    union{
    int   pid;                          //命令字段中的2-5字节，传入的pid
    unsigned char  pch[4];
    };
    unsigned char proc;                 //命令字段中的6字节，存放进度值1-100
    unsigned char end0;                 //命令字段中的7字节，存放本次查询的结束标志，=1结束
    char vv0[2];                        //命令字段中的8,9字节，暂时未用。再次查询时在副本中用到8字节=3表示有效。
    union{
    unsigned long snum;                 //命令字段中的10字节起的查询关键字。
    unsigned char sch[4];
    };
    char oaddr[4];                      //命令字段中的14字节起的导出地址。
    char vv1[22];                       //18-47字节暂时未,可以考虑传送代码段和数据段的段地址。
    union{
    unsigned int data_seg;
    unsigned char dsg[4];
    };
    union{
    unsigned int text_seg;
    unsigned char tsg[4];
    };
    struct KVAR_ADDR vadd[8];           //8个锁定（修改）数据和地址。 
};*/
///}}}
struct SAVE_LOCK
{
	char ch[40];
	struct KVAR_LOCK lk;
//   unsigned int maxd;          //上限
//    unsigned int mind;          //下限
//	union OFFSET add;
};

struct KVAR_AM	 kam;
int idx;
char doff[40];
struct SAVE_LOCK sl[max_res];
struct timeval tm;
union OFFSET mem_u ;
int pn;
unsigned char *mmp,*mnp;
unsigned char cmp[4100];
unsigned char pcc1[4100],pcc2[4100];
unsigned int dst_addr;	//页面比较使用，首次传出的有效的起始地址，也是后续操作允许的标志
//{{{ define
#define AREA_SIZE			100
#define buf_size			8192
#define adr_buf1			8192*(AREA_SIZE+1)*2 
//2016-11-5修改，原来是#define adr_buf2			4096*20
#define adr_buf2			4096*41
#define d_begin				192
#define dlen				8000
#define drv_name			"/dev/memacc_dev0"
#define lock_file_dir		"/workarea/games/savefiles/"

#define	wintitle			"修改大师V2.0"
//window's width & height
#define win_x				640
#define win_y				492
//lab1's width & height
#define lab1_w				70
#define lab1_h				25
#define lab1_px				10
#define lab1_py				lab1_px
//treeview1's width & height
#define t1_w				160
#define t1_h				275
//treeview's position
#define t1_px				10
#define t1_py				40
//scrollbar1's width
#define srl_w				16
#define srl_h				t1_h
//scrollbar1's position
#define srl_px				11+t1_w
#define srl_py				t1_py
//bnt1~2's pos and wid
#define bnt1_w				75
#define bnt1_h				22
#define bnt1_px				t1_px
#define bnt1_py				t1_h+t1_py+10
#define bnt2_w				bnt1_w
#define	bnt2_h				bnt1_h
#define bnt2_px				bnt1_px+bnt1_w+srl_w+10
#define bnt2_py				bnt1_py
//part2 label
#define lab2_w				100
#define lab2_h				lab1_h
#define lab2_px				srl_w+srl_px+5
#define lab2_py				lab1_py
//输入项的位置
#define labi_w				140
#define labi_h				lab2_h
#define labi_px				lab2_px
#define labi_py				lab2_py+lab2_h+5
//part2 label2
#define lab3_w				lab2_w
#define lab3_h				lab2_h
#define lab3_px				lab2_px
#define lab3_py				labi_py+55
//part2 entry1
#define entry1_w			140
#define entry1_h			lab3_h-3
#define entry1_px			lab3_px
#define entry1_py			lab3_py+lab3_h+3
//part2 radiobutton
#define rd1_px				entry1_px
#define rd1_py				entry1_py+entry1_h+5
#define rd2_px				rd1_px+70
#define rd2_py				rd1_py
//part2 progressbar
#define pro_w				entry1_w
#define pro_h				entry1_h-6
#define pro_px				entry1_px
#define pro_py				rd1_py+27
//part2 button
#define bnt3_w				120
#define bnt3_h				bnt2_h
#define bnt3_px				pro_px+10
#define bnt3_py				pro_py+pro_h+18
#define bnt4_w				bnt3_w
#define bnt4_h				bnt3_h
#define bnt4_px				bnt3_px
#define bnt4_py				bnt3_py+bnt3_h+18
#define bnt5_w				bnt4_w
#define bnt5_h				bnt4_h
#define bnt5_px				bnt4_px
#define bnt5_py				bnt4_py+bnt4_h+18
//2016-11-1 add
#define labnn_w				140
#define labnn_h				labi_h
#define labnn_px			bnt5_px
#define labnn_py			bnt5_py+bnt5_h+10
//part3 label
#define lab4_px				lab2_px+150
#define lab4_py				lab2_py
#define lab5_px				lab4_px+150
#define lab5_py				lab4_py
//part3 entry
#define entry2_w			140
#define entry2_h			20
#define entry2_px			lab4_px
#define entry2_py			labi_py
#define entry3_w			entry2_w
#define entry3_h			entry2_h
#define entry3_px			lab5_px
#define entry3_py			entry2_py
//part3 label
#define lab6_px				lab4_px
#define lab6_py				entry2_py+entry2_h+5
#define lab7_px				lab5_px
#define lab7_py				lab6_py
//part3 entry
#define entry4_w			entry3_w
#define entry4_h			entry3_h
#define entry4_px			entry2_px
#define entry4_py			lab6_py+27
#define entry5_w			entry4_w
#define entry5_h			entry4_h
#define entry5_px			lab7_px
#define entry5_py			entry4_py
//part3 button
#define bnt6_w				entry4_w
#define bnt6_h				bnt5_h
#define bnt6_px				entry4_px
#define bnt6_py				entry4_py+entry4_h+10
#define bnt7_w				bnt6_w
#define bnt7_h				bnt6_h
#define bnt7_px				entry5_px
#define bnt7_py				bnt6_py
//part3 treeview 185
#define t2_w				290
#define t2_h				160
#define t2_px				bnt6_px
#define t2_py				bnt6_py+bnt6_h+10
//part3 button
#define bnt8_w				bnt1_w
#define bnt8_h				bnt1_h
#define bnt8_px				t2_px+12
#define bnt8_py				t2_h+t2_py+10
#define bnt9_w				bnt8_w
#define bnt9_h				bnt8_h
#define bnt9_px				bnt8_px+bnt8_w+20
#define bnt9_py				bnt8_py
#define bnt10_w				bnt9_w
#define bnt10_h				bnt9_h
#define bnt10_px			bnt9_px+bnt9_w+20
#define bnt10_py			bnt9_py
//part3 treeview
#define t3_w				win_x-srl_w-20
#define t3_h				130
#define t3_px				t1_px
#define t3_py				bnt10_py+bnt10_h+10
//part3 scroll
#define srl2_w				srl_w
#define srl2_h				t3_h
#define srl2_px				t3_px+t3_w
#define srl2_py				t3_py
//2014-1-25添加检视内存界面的尺寸定义
#define m_wid				440			//win_x-4
#define m_hei				425			//win_y-4
#define rt0_x				1
#define rt0_y				rt0_x
#define rt_w				60
#define rt_h				20
#define in_x				80
#define in_w				22
#define pg0_x				442
#define pg0_y				rt0_y
#define pg0_w				150
#define pg0_h				24

#define pg_x				pg0_x
#define pg_y				pg0_y+pg0_h+5
#define pg_w				100
#define pg_h				pg0_h
#define pg_x1				pg_x+pg_w
#define pg_y1				pg_y
#define pg_w1				pg_w-30
#define pg_h1				pg_h
#define pg1_x				pg_x
#define pg1_y				pg_y+pg_h+5
#define pg1_w				pg_w
#define pg1_h				pg_h
#define pg1_x1				pg_x1
#define pg1_y1				pg1_y
#define pg1_w1				pg_w1
#define pg1_h1				pg1_h
//三个按钮的位置
#define pgb1_x				pg1_x+10
#define pgb1_y				pg1_y+pg1_h+10
#define pgb1_w				120
#define pgb1_h				pg1_h1
#define pgb2_x				pgb1_x
#define pgb2_y				pgb1_y+35
#define pgb2_w				pgb1_w
#define pgb2_h				pgb1_h
#define pgb3_x				pgb2_x
#define pgb3_y				pgb2_y+35
#define pgb3_w				pgb2_w
#define pgb3_h				pgb2_h
//组合框的位置
#define pgc1_x				pgb3_x
#define pgc1_y				pgb3_y+35
#define pgc1_w				pgb3_w
#define pgc1_h				pgb3_h
//列表框
#define pglist_x			pgc1_x
#define pglist_y			pgc1_y+35
#define pglist_w			160
#define pglist_h			140
#define pgb4_x				pgb3_x
#define pgb4_y				pglist_y+pglist_h+11
#define pgb4_w				pgb3_w
#define pgb4_h				pgb3_h
//2016-11-04准备添加多重内存页的比较界面
#define mm_wid				400
#define mm_hei				550
#define mmlab1_w			100
#define mmlab1_h			20
#define mmlab1_x			20
#define mmlab1_y			20
#define mmed1_w				100
#define mmed1_h				20
#define mmed1_x				mmlab1_w+mmlab1_x+10
#define mmed1_y				mmlab1_y
#define mmbnt_w				80
#define mmbnt_h				20
#define mmbnt_x				mmlab1_x
#define mmbnt_y				mmed1_y+mmed1_h+20
#define mmbnt1_w			mmbnt_w
#define mmbnt1_h			mmbnt_h
#define mmbnt1_x			mmbnt_x+mmbnt_w+15
#define mmbnt1_y			mmbnt_y
#define mmbnt2_w			mmbnt1_w
#define mmbnt2_h			mmbnt1_h
#define mmbnt2_x			mmbnt1_x+mmbnt_w+15
#define mmbnt2_y			mmbnt_y
#define mmbnt3_w			mmbnt2_w
#define mmbnt3_h			mmbnt2_h
#define mmbnt3_x			mmbnt2_x+mmbnt_w+15
#define mmbnt3_y			mmbnt_y
#define mmlab2_w			360
#define mmlab2_h			20
#define mmlab2_x			16
#define mmlab2_y			mmbnt2_y+mmbnt2_h+10
#define mmtre_w				mmlab2_w
#define mmtre_h				415
#define mmtre_x				mmlab2_x
#define mmtre_y				mmlab2_y+mmlab2_h+10







///////////////////////////////////
#define tip_statusicon		"修改大师V1.0\ntybitsfox\n2013-12-25"
#define iconfile			"/workarea/cprogram/common_source/20131113114643935_easyicon_net_48.png"
#define myphoto				"/workarea/cprogram/common_source/08888.jpg"

#define ps_cmd				"ps -Ao pid,fname >/tmp/uuuu_mod_tybitsfox.txt"
#define proc_file			"/tmp/uuuu_mod_tybitsfox.txt"

#define labi				"<span style=\"oblique\" color=\"red\" font=\"10\">%s</span>"
#define labin				"<span style=\"oblique\" color=\"red\" font=\"10\">%X</span>"
#define lab1				"<span style=\"oblique\" color=\"blue\" font=\"10\">获取的进程：</span>"
#define lab2				"<span style=\"oblique\" color=\"blue\" font=\"10\">当前指定的进程：</span>"
#define lab3				"<span style=\"oblique\" color=\"blue\" font=\"10\">待查询数据：</span>"
#define lab4				"<span style=\"oblique\" color=\"blue\" font=\"10\">锁定摘要：</span>"
#define lab5				"<span style=\"oblique\" color=\"blue\" font=\"10\">锁定地址：</span>"
#define lab6				"<span style=\"oblique\" color=\"blue\" font=\"10\">锁定下限：</span>"
#define lab7				"<span style=\"oblique\" color=\"blue\" font=\"10\">锁定上限：</span>"
#define rad0				"<span color=\"#ff00ff\" font=\"10\">十进制</span>"
#define rad1				"<span color=\"#ff00ff\" font=\"10\">十六进制</span>"
#define mlab0				"<span style=\"oblique\" color=\"red\" font=\"10\">目标进程PID：%d</span>"
#define mlab1				"<span style=\"oblique\" color=\"red\" font=\"10\">段索引：</span>"
#define mlab2				"<span style=\"oblique\" color=\"red\" font=\"10\">页索引：</span>"
#define labm				"<span style=\"oblique\" color=\"red\" font=\"10\">%s %d</span>"

//}}}
//{{{
//主窗口的建立函数
void crt_window(int a,char **b);
//pixbuf的生成函数
GdkPixbuf *crt_pixbuf(gchar *fname);
//尝试建立treeview
void crt_treeview1();
//建立操作treeview1的两个按钮
void crt_tv1_bnt();
//建立第二列的控件
void crt_part2();
//建立第三列的控件
void crt_part3();
//按钮1 "取得进程"的响应函数
void on_getproc(GtkWidget *widget,gpointer gp);
//按钮2 "重置"的响应函数
void on_reset(GtkWidget *widget,gpointer pg);
//按钮3 "首次查找"的响应函数
void on_first_srh(GtkWidget *widget,gpointer gp);
//消息提示框
void msgbox(char *gc);
//树形列表1的双击响应函数
void on_tree1_dblclk(GtkTreeView *treeview,GtkTreePath *path,GtkTreeViewColumn *col,gpointer userdata);
//建立托盘
void crt_statusicon();
//托盘菜单中：恢复窗口的响应函数。
void restore_window(GtkWidget *widget,gpointer gp);
//托盘菜单中：关于窗口的响应函数
void about_window(GtkWidget *widget,gpointer gp);
//托盘菜单中：退出的响应函数
void exit_window(GtkWidget *widget,gpointer gp);
//托盘图标的右键菜单弹出响应函数
void show_menu(GtkWidget *widget,guint button,guint32 act_time,gpointer gp);
//主窗口隐藏，切换至托盘形式
gint hide_window(GtkWidget *widget,GdkEvent *event,gpointer gp);
//首次查找的按钮响应函数
void on_first_srh(GtkWidget *widget,gpointer gp);
//检查输入的查询关键字函数
int check_input(char *c);
//首次查找的线程函数
gpointer thd_fst(gpointer pt);
//一个毫秒级的定时器
void msleep();
//再次查询按钮的响应函数。
void on_next_srh(GtkWidget *widget,gpointer gp);
//再次查找的线程函数
gpointer thd_next(gpointer pt);
//treeview3的双击响应函数
void on_tree3_dblclk(GtkTreeView *treeview,GtkTreePath *path,GtkTreeViewColumn *col,gpointer userdata);
//测试用，保存首次查询的地址集：
void on_save(GtkWidget *widget,gpointer gp);
//锁定指定地址数据的线程。
gpointer thd_thr(gpointer pt);
//按钮"添加锁定"的响应函数，测试取得treeview的记录条数
void on_addlock(GtkWidget *widget,gpointer gp);
//按钮"删除锁定"的响应函数，测试取得记录的内容
void on_dellock(GtkWidget *widget,gpointer gp);
//锁定信息的输入检查函数。
int check_lock(char *c);
//开始锁定
void on_lock(GtkWidget *widget,gpointer gp);
//地址转换
int tonum(char *c,unsigned int *ui);
//装载记录文件
void on_load(GtkWidget *widget,gpointer gp);
//进制换算
void on_calc(GtkWidget *widget,gpointer gp);
//cairo画图
gboolean on_drawing(GtkWidget *widget,GdkEventExpose *event,gpointer gp);
//内存块的即时显示窗口
void show_mem(GtkWidget *widget,gpointer gp);
//双击锁定列表框的响应函数
void on_tree2_dblclk(GtkTreeView *treeview,GtkTreePath *path,GtkTreeViewColumn *col,gpointer userdata);
//下翻页按钮的响应函数
void on_pagedown(GtkWidget *widget,gpointer gp);
//上翻页按钮的相应函数
void on_pageup(GtkWidget *widget,gpointer gp);
//内存检视按钮的响应函数
void on_memsrh(GtkWidget *widget,gpointer gp);
//内存检索的线程函数
gpointer mthd_srh(gpointer gp);
//2014-2-23添加，combobox的响应函数
void on_combo_changed(GtkWidget *widget,gpointer gp);
//2014-2-26添加，模糊查询按钮的响应函数。
void on_blursrh(GtkWidget *widget,gpointer gp);
//2016-11-4添加多重页面比较的界面显示
void show_mmem(GtkWidget *widget,gpointer gp);
void on_mmbeg(GtkWidget *widget,gpointer gp);
void on_mmdel(GtkWidget *widget,gpointer gp);
void on_mmshow(GtkWidget *widget,gpointer gp);
gpointer mmthd_fst(gpointer gp); //比较开始的线程响应函数
gpointer mmthd_del(gpointer gp); //减少比较的线程响应函数


//}}}


