#include"k05.h"
//{{{ int __init minedev_init(void)
int __init minedev_init(void)
{
	int res,retval;
	int devno;
	mem_class=NULL;
	devno=MKDEV(DRV_MAJOR,DRV_MINOR);
	mp=(char*)kmalloc(K_BUFFER_SIZE,GFP_KERNEL);
	if(mp==NULL)
		printk("<1>kmalloc error!\n");
	else
		memset(mp,0,K_BUFFER_SIZE);
	mp[0]=1;//允许用户进程发送指令。
	tp=(char*)vmalloc(K_BUFFER_SIZE);
	if(tp==NULL)
		printk("<1>vmalloc error!\n");
	else
		memset(tp,0,K_BUFFER_SIZE);
	kcc=(unsigned char*)vmalloc(K_CC_SIZE);
	if(kcc==NULL)
		printk("<1>vmalloc kcc error!\n");
	else
		memset(kcc,0,K_CC_SIZE);
	res=register_chrdev(DRV_MAJOR,drv_name,&mem_fops);//注册字符设备
	if(res)
	{
		printk("<1>register char dev error %d\n",res);
		goto reg_err01;
	}
	mem_class=kzalloc(sizeof(*mem_class),GFP_KERNEL);//实体话设备类
	if(IS_ERR(mem_class))
	{
		kfree(mem_class);
		mem_class=NULL;
		printk("<1>kzalloc error\n");
		goto reg_err02;
	}
	mem_class->name=drv_name;
	mem_class->owner=THIS_MODULE;
	mem_class->class_release=class_create_release;
	retval=class_register(mem_class);//注册设备类
	if(retval)
	{
		kfree(mem_class);
		printk("<1>class_register error\n");
		goto reg_err02;
	}
	device_create(mem_class,NULL,devno,NULL,drv_name);//注册设备文件系统，并建立节点。
	printk("<1>device create successful!!!\n");
	kv1.pin=1;//指定mp作为输入输出缓冲区。
	return 0;
reg_err02:
	unregister_chrdev(DRV_MAJOR,drv_name);//删除字符设备。
reg_err01:
	if(mp!=NULL)
	{	kfree(mp);mp=NULL;}
	if(tp!=NULL)
	{	vfree(tp);tp=NULL;}
	if(kcc!=NULL)
	{ vfree(kcc);kcc=NULL;}
	suc=1;
	return -1;	
}//}}}
//{{{ void __exit minedev_exit(void)
void __exit minedev_exit(void)
{
	if(suc!=0)
		return;
	unregister_chrdev(DRV_MAJOR,drv_name);
	device_destroy(mem_class,MKDEV(DRV_MAJOR,DRV_MINOR));
	if(mem_class!=NULL && (!IS_ERR(mem_class)))
		class_unregister(mem_class);
	if(mp!=NULL)
		kfree(mp);
	if(tp!=NULL)
		vfree(tp);
	if(kcc!=NULL)
		vfree(kcc);
	printk("<1>module eixt ok!\n");
}//}}}
//{{{ int mem_open(struct inode *ind,struct file *filp)
int mem_open(struct inode *ind,struct file *filp)
{
	try_module_get(THIS_MODULE);	//引用计数增加
	printk("<1>device open success!\n");
	return 0;
}//}}}
//{{{ int mem_release(struct inode *ind,struct file *filp)
int mem_release(struct inode *ind,struct file *filp)
{
	module_put(THIS_MODULE);	//计数器减1
	printk("<1>device release success!\n");
	return 0;
}//}}}
//{{{ void class_create_release(struct class *cls)
void class_create_release(struct class *cls)
{
	pr_debug("%s called for %s\n",__func__,cls->name);
	kfree(cls);
}//}}}
//{{{ ssize_t mem_read(struct file *filp,char *buf,size_t size,loff_t *fpos)
ssize_t mem_read(struct file *filp,char *buf,size_t size,loff_t *fpos)
{
	int res;
	char *tmp;
	if(mp==NULL || tp==NULL || kcc==NULL)
	{
		printk("<1>kernel buffer error!\n");
		return 0;
	}
	switch(kv1.pin)
	{
	case 1://mp作为输入输出缓冲
		tmp=mp;
		break;
	case 2://tp作为输入输出缓冲
		tmp=tp;
		break;
	case 3://2016-11-3添加，使用kcc作为输出缓冲
		tmp=kcc;
		if(size>(41*4096))
			size=41*4096;
		res=copy_to_user(buf,tmp,size);
		kv1.pin=1; //一定要重置该值，以保证其他操作可执行
		printk("<1>okokok!\n");
		if(res==0)
			return size;
		else
			return 0;
	default://pin为其它值时，不进行拷贝操作，直接返回0
		return 0;
	};
	if(size>K_BUFFER_SIZE)
		size=K_BUFFER_SIZE;
	res=copy_to_user(buf,tmp,size);
	if(res==0)
		return size;
	else
		return 0;
}//}}}
//{{{ ssize_t mem_write(struct file *filp,char *buf,size_t size,loff_t *fpos)
ssize_t mem_write(struct file *filp,const char *buf,size_t size,loff_t *fpos)
{
	int res,i,k,m;
	unsigned int j;
	char *tmp;
	struct KVAR_LOCK *kvl;
	if(mp==NULL || tp==NULL)
	{
		printk("<1>kernel buffer error\n");
		return 0;
	}
	switch(kv1.pin)
	{
	case 1://mp
		tmp=mp;
		break;
	case 2://tp
		tmp=tp;
		break;
//	case 3://kcc
//		tmp=kcc;
	default:
		return 0;
	};
	if(size>K_BUFFER_SIZE)
		size=K_BUFFER_SIZE;
	res=copy_from_user(tmp,buf,size);
	if(res==0)
	{
		if(tmp[0]!=0 || tmp[2]==1)
			return size;
		if(kv1.thread_lock==0)
		{
			k_am=(struct KVAR_AM*)tmp;
			k_am->sync=0;k_am->end0=0;
			kv1.pid=k_am->pid;//保存pid
			kv1.snum=k_am->snum;//保存搜索数字。
			if(k_am->cmd==1)//首次查找操作。
			{
				kv1.pin=0;//不再接收用户的输入输出。
				kv1.thread_lock=1;//设置线程锁
//				kernel_thread(first_srh,NULL,CLONE_KERNEL);
				kthread_run(first_srh,NULL,"first_srh");
				return size;
			}
			if(k_am->cmd==2)//再次查询。
			{
				k_tp=(struct KVAR_AM*)tp;
				k_am->end1=0;
				kv1.pin=0;
				kv1.thread_lock=1;
//				kernel_thread(next_srh,NULL,CLONE_KERNEL);
				kthread_run(next_srh,NULL,"next_srh");
				return size;
			}
			if(k_am->cmd==3)//锁定操作
			{//需要保存下代码段和数据段的段长度。
				kv1.t_len=k_am->t_len;
				kv1.d_len=k_am->d_len;
				//2016-10-29修改，使用新定义的结构存放锁定信息
				memset((void*)&kloc_s[0],0,sizeof(struct KLOCK_STR));
				for(k=0;k<16;k++)
				{
					j=0xffffffff;m=-1;
					kvl=&k_am->ladr[0];
					for(i=0;i<16;i++)
					{
						if(kvl->offset.ad == 0) //无效
						{
							kvl++;
							continue;
						}
						if(kvl->offset.ad < j)
						{m=i;j=kvl->offset.ad;}
						kvl++;
					}
					if(m != -1)
					{
						kvl=&k_am->ladr[0];
						for(i=1;i<=m;i++)
						{kvl++;}
						memcpy((void *)(&kloc_s[0].lk[k]),kvl,sizeof(struct KVAR_LOCK));
						kloc_s[0].flg[k]=1;
						kvl->offset.ad=0;
					}
				}
				memset(tp,0,K_BUFFER_SIZE);
				//	kv1.pin=1;//锁定操作要随时准备接收用户输入，终止锁定，所以必须要指定mp为缓冲区。
				kv1.pin=2;//改为使用tp作为缓冲区。
				kv1.thread_lock=1;
//				kernel_thread(lock_srh,NULL,CLONE_KERNEL);
				kthread_run(lock_srh,NULL,"lock_srh");
				return size;
			}
			if(k_am->cmd==4)//内存检视
			{
				kv1.t_len=k_am->t_len;
				kv1.d_len=k_am->d_len;
				memset(tp,0,K_BUFFER_SIZE);
				kv1.pin=2;//使用tp
				kv1.thread_lock=1;
//				kernel_thread(mem_srch,NULL,CLONE_KERNEL);
				kthread_run(mem_srch,NULL,"mem_srch");
				return size;
			}
			if(k_am->cmd==5)//内存比较，2016-11-5添加
			{
				kv1.t_len=k_am->t_len;
				kv1.d_len=k_am->d_len;
				memset(kcc,0,K_CC_SIZE);
				kv1.pin=2;//使用kcc
				kv1.thread_lock=1;
				kthread_run(mem_cmp,NULL,"mem_cmp");
				return size;
			}
		}
	}
	s_wait_mm(2);
	return size;
}//}}}
//{{{ int first_srh(void *argc)
int first_srh(void *argc)
{
	unsigned char *c;
	char *mc,*v;
	union OFFSET uf;
	int ret,len,i,j,k,tt;
	struct pid *kpid;
	struct task_struct *t_str;
	struct mm_struct  *mm_str;
	struct vm_area_struct *vadr;
	struct page **pages;
	unsigned int adr;
	union OFFSET *ksa;
//	daemonize("ty_thd1");
	v=NULL;uf.ad=0;
	mm_str=NULL;tt=0;
//	mc=&mp[d_begin];	2016-10-27修改，使其指向内核分配的缓冲，不再分次传出查询地址。
	mc=(char*)kcc;
//	memset(mc,0,dlen);	2016-10-27修改，配合上述修改。
	memset(mc,0,K_CC_SIZE);
	ksa=(union OFFSET *)mc;
	kpid=find_get_pid(kv1.pid);
	if(kpid==NULL)
	{
		printk("<1>find_get_pid error\n");
		goto ferr_01;
	}
	t_str=pid_task(kpid,PIDTYPE_PID);
	if(t_str==NULL)
	{
		printk("<1>pid_task error!\n");
		goto ferr_01;
	}
	mm_str=get_task_mm(t_str);
	if(mm_str==NULL)
	{
		printk("<1>get_task_mm error!\n");
		goto ferr_01;
	}
	kv1.seg[0]=mm_str->start_code;//第一个是代码段。
	kv1.seg[1]=mm_str->start_data;//第二个是数据段
	kv1.seg[2]=mm_str->start_brk; //第三个是分配的堆段
	kv1.seg[3]=mm_str->start_stack;//第四个是堆栈段。
	kv1.t_len=mm_str->end_code-mm_str->start_code;//代码段长度
	kv1.d_len=mm_str->end_data-mm_str->start_data;//数据段长度
	printk("first: snum= %d total pages=%lx stack pages=%lx\n",kv1.snum,mm_str->total_vm,mm_str->stack_vm);
	k_am->tol_pg=1;//mm_str->total_vm;
	k_am->fin_pg=1;//这两项是用于进度条显示进度的。
	vadr=mm_str->mmap;//code seg
	tt=mm_str->map_count;
	kcc_cnt=0;
	for(i=0;i<tt;i++)
	{
		adr=vadr->vm_start;
		len=vma_pages(vadr);
		if(v!=NULL)
		{
			vfree(v);
			v=NULL;
		}
		v=(char*)vmalloc(sizeof(void*)*(len+1));
		pages=(struct page **)v;
		if(pages==NULL)
		{
			printk("<1>vmalloc error11\n");
			goto ferr_01;
		}
		memset((void*)pages,0,sizeof(void*)*(len+1));
		down_read(&mm_str->mmap_sem);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
		ret=get_user_pages(t_str,mm_str,adr,len,0,0,pages,NULL);
#else
		ret=get_user_pages_remote(t_str,mm_str,adr,len,0,pages,NULL);
#endif
		if(ret<=0)
		{
			printk("<1>ret<0 len=%d adr=%u\n",len,adr);
			up_read(&mm_str->mmap_sem);
			//goto ferr_01;
			goto stepf_01;
		}
		k_am->tol_pg=ret;
		for(k=0;k<ret;k++)
		{
			if(pages[k]==NULL)
			{
				printk("<1>search finished\n");
				break;
			}
			if(k>=65535)
			{
				printk("<1>pages counts too large\n");
				break;
			}
			if(kcc_cnt>=AREA_SIZE)
				break;
			c=(unsigned char*)kmap(pages[k]);
			//{{{  0x100
			if(kv1.snum<0x100)
			{
				for(j=0;j<4096;j++)
				{
					if(c[j]==kv1.sch[0])
					{//find
						ksa->ad=j+adr+k*4096;
						ksa++;kcc_cnt++;
						if(kcc_cnt>=AREA_SIZE)
							break;
			/*			if((void *)ksa-(void *)mc>7996)
						{
							k_am->tol_pg=ret;
							k_am->fin_pg=k;//n;//传出已经搜索的页数
							s_sync(1);m++;
							ksa=(union OFFSET *)mc;
							memset(mc,0,dlen);
						} */
					}
				}
				kunmap(pages[k]);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
				page_cache_release(pages[k]);
#else
				put_page(pages[k]);
#endif
				continue;
			}//}}}
			//{{{ 0x10000
			if(kv1.snum<0x10000 && kv1.snum>0xff)
			{
				for(j=0;j<4095;j++)
				{
					if((c[j]==kv1.sch[0]) && (c[j+1]==kv1.sch[1]))
					{
						ksa->ad=j+adr+k*4096;
						ksa++;kcc_cnt++;
						if(kcc_cnt>=AREA_SIZE)
							break;
			/*			if((void *)ksa-(void *)mc>7996)
						{
							k_am->fin_pg=k;//传出已经搜索的页数
							s_sync(1);m++;
							ksa=(union OFFSET *)mc;
							memset(mc,0,dlen);
						}*/
					}
				}
				kunmap(pages[k]);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
				page_cache_release(pages[k]);
#else
				put_page(pages[k]);
#endif
				continue;
			}//}}}
			//{{{ 0x1000000
			if(kv1.snum<0x1000000 && kv1.snum>0xffff)
			{
				for(j=0;j<4094;j++)
				{
					if((c[j]==kv1.sch[0]) && (c[j+1]==kv1.sch[1]) && (c[j+2]==kv1.sch[2]))
					{
						ksa->ad=j+adr+k*4096;
						ksa++;kcc_cnt++;
						if(kcc_cnt>=AREA_SIZE)
							break;
				/*		if((void *)ksa-(void *)mc>7996)
						{
							k_am->fin_pg=k;//传出已经搜索的页数
							s_sync(1);m++;
							ksa=(union OFFSET *)mc;
							memset(mc,0,dlen);
						}*/
					}
				}
				kunmap(pages[k]);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
				page_cache_release(pages[k]);
#else
				put_page(pages[k]);
#endif
				continue;
			}//}}}
			//{{{ 0x100000000
			if(kv1.snum>0xffffff)
			{
				for(j=0;j<4093;j++)
				{
					if((c[j]==kv1.sch[0]) && (c[j+1]==kv1.sch[1]) && (c[j+2]==kv1.sch[2]) && (c[j+3]==kv1.sch[3]))
					{
						ksa->ad=j+adr+k*4096;
						ksa++;kcc_cnt++;
						if(kcc_cnt>=AREA_SIZE)
							break;
			/*			if((void *)ksa-(void *)mc>7996)
						{
							k_am->fin_pg=n;//传出已经搜索的页数
							s_sync(1);m++;
							ksa=(union OFFSET *)mc;
							memset(mc,0,dlen);
						}*/
					}
				}
			}//}}}
			kunmap(pages[k]);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
			page_cache_release(pages[k]);
#else
			put_page(pages[k]);
#endif
		}
		up_read(&mm_str->mmap_sem);
stepf_01:		
		pages=NULL;
		if(kcc_cnt>=AREA_SIZE)
		{//最多保存10万个搜索结果。400k
			printk("<1>max counts:%d\n",kcc_cnt);
			goto ferr_01;
		}
		vadr=vadr->vm_next;
		if(vadr==NULL)
			break;
	};//seg:4
ferr_01:
	if(v!=NULL)
		vfree(v);
//2016-10-27添加，拷贝部分地址到用户空间，保持兼容性。	
	if(kcc_cnt>0)
	{
		i=kcc_cnt>100?100:kcc_cnt*4;
		mc=&mp[d_begin];
		memset(mc,0,dlen);
		memcpy(mc,kcc,i);
	}
	s_sync(2);
	kv1.thread_lock=0;
	printk("<1>kernel_thread finished! tt=%d\n",tt);
	do_exit(0);
	return 0;
}//}}}
//{{{ void s_sync(int t)
void s_sync(int t)
{
	switch(t)
	{
	case 0://内核忙，返回用户的读取字节数为0
		kv1.pin=0;
		return;
	case 1://首次查询，完成一页的地址
		k_am->t_seg=kv1.seg[0];
		k_am->d_seg=kv1.seg[1];
		k_am->b_seg=kv1.seg[2];
		k_am->s_seg=kv1.seg[3];
		k_am->t_len=kv1.t_len;
		k_am->d_len=kv1.d_len;
		k_am->end0=0;
		k_am->sync=1;
		kv1.pin=1;
		s_wait_mm(0);
		kv1.pin=0;
		return;
	case 2://首次查询，全部完成。
		k_am->t_seg=kv1.seg[0];
		k_am->d_seg=kv1.seg[1];
		k_am->b_seg=kv1.seg[2];
		k_am->s_seg=kv1.seg[3];
		k_am->t_len=kv1.t_len;
		k_am->d_len=kv1.d_len;
		k_am->end0=1;
		k_am->sync=1;
		kv1.pin=1;
		s_wait_mm(0);
		return;
	case 10://再次查询，传出结果。
		k_tp->end0=0;
		k_tp->end1=0;
		k_tp->t_seg=kv1.seg[0];
		k_tp->d_seg=kv1.seg[1];
		k_tp->b_seg=kv1.seg[2];
		k_tp->s_seg=kv1.seg[3];
		k_tp->t_len=kv1.t_len;
		k_tp->d_len=kv1.d_len;
		k_tp->sync=1;
		kv1.pin=2;
		s_wait_mm(1);
		kv1.pin=0;
		return;
	case 12://再次查询，接收用户输入的地址
		if(k_am->end0!=0)
		{
			k_tp->end0=1;//与case 10唯一的不同，表示全部完成。
			k_tp->end1=0;
			k_tp->t_seg=kv1.seg[0];
			k_tp->d_seg=kv1.seg[1];
			k_tp->b_seg=kv1.seg[2];
			k_tp->s_seg=kv1.seg[3];
			k_tp->t_len=kv1.t_len;
			k_tp->d_len=kv1.d_len;
			k_tp->sync=1;
			kv1.pin=2;
			s_wait_mm(1);
			kv1.pin=1;
			memset((void*)tp,0,K_BUFFER_SIZE);
			return;
		}
		k_am->t_seg=kv1.seg[0];
		k_am->d_seg=kv1.seg[1];
		k_am->b_seg=kv1.seg[2];
		k_am->s_seg=kv1.seg[3];
		k_am->t_len=kv1.t_len;
		k_am->d_len=kv1.d_len;
		k_am->sync=1;
		k_am->end0=0;
		k_am->end1=1;
		kv1.pin=1;
		s_wait_mm(0);
		kv1.pin=0;
		return;
	};

}//}}}
//{{{ void s_wait_mm(int w)
void s_wait_mm(int w)
{
	char *ch;
	//printk("<1>a sleep\n");
	if(w==0)//mp
		ch=mp;
	else
	{
		if(w==1)
			ch=tp;
		else
		{
			set_current_state(TASK_INTERRUPTIBLE);
			schedule_timeout(HZ);
			return;
		}
	}
	while(1)
	{
		if(ch[0]==0)
			break;
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(HZ);
	}
}//}}}
//{{{  int mem_srch(void *argc)
int mem_srch(void *argc)
{
	struct pid *kpid;
	struct task_struct *t_str;
	struct mm_struct *mm_str;
	struct vm_area_struct *vadr;
	struct page **pages;
	union OFFSET oft;
	char *v,*p;
	unsigned char *c;
	int i,j,ret,len;
	unsigned long adr;
	v=NULL;p=(char*)&(tp[d_begin]);
//	daemonize("ty_thd4");
	kpid=find_get_pid(kv1.pid);
	if(kpid==NULL)
	{
		printk("<1>find_get_pid error\n");
		goto merr_01;
	}
	t_str=pid_task(kpid,PIDTYPE_PID);
	if(t_str==NULL)
	{
		printk("<1>pid_task error\n");
		goto merr_01;
	}
	mm_str=get_task_mm(t_str);
	if(mm_str==NULL)
	{
		printk("<1>get_task_mm error\n");
		goto merr_01;
	}
	i=mm_str->end_code-mm_str->start_code;
	j=mm_str->end_data-mm_str->start_data;
	if((i!=kv1.t_len) || (j!=kv1.d_len))
	{
		printk("<1>target process unequal\n");
		goto merr_01;
	}
	oft.ad=kv1.snum;
	vadr=find_vma(mm_str,oft.ad);
	if(vadr==NULL)
	{
		printk("<1>find_vma error\n");
		goto merr_01;
	}
//	printk("<1>seg=%d page=%d\n",oft.seg,oft.page);
//	vadr=mm_str->mmap;
/*	for(i=0;i<8;i++)
	{
		if(i==oft.seg)
			break;
		if(vadr==NULL)
		{
			printk("<1>vadr is NULL\n");
			goto merr_01;
		}
		vadr=vadr->vm_next;
	} */
	adr=vadr->vm_start;
	len=vma_pages(vadr);
	if(len<=0)
	{
		printk("<1>vma_pages error\n");
		goto merr_01;
	}
	v=(char*)vmalloc(sizeof(void*)*(len+1));
	pages=(struct page **)v;
	if(pages==NULL)
	{
		printk("<1>vmalloc error\n");
		goto merr_01;
	}
	while(1)
	{
		memset((void*)pages,0,sizeof(void*)*(len+1));
//		down_write(&mm_str->mmap_sem);
		down_read(&mm_str->mmap_sem);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
		ret=get_user_pages(t_str,mm_str,adr,len,0,0,pages,NULL);
#else
		ret=get_user_pages_remote(t_str,mm_str,adr,len,0,pages,NULL);
#endif
		if(ret<=0)
		{
			printk("<1> get_user_pages error\n");
//			up_write(&mm_str->mmap_sem);
			up_read(&mm_str->mmap_sem);
			goto merr_01;
		}
		i=(oft.ad-adr)/0x1000;
		c=(unsigned char*)kmap(pages[i]);
		memcpy(p,c,4096);
		kunmap(pages[i]);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
		page_cache_release(pages[i]);
#else
		put_page(pages[i]);
#endif
//		up_write(&mm_str->mmap_sem);
		up_read(&mm_str->mmap_sem);
		if(tp[2]==1)//end
			break;
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(2*HZ);
	}
merr_01:
	if(v!=NULL)
		vfree(v);
	kv1.pin=1;
	kv1.thread_lock=0;
	do_exit(0);
	return 0;
}//}}}
//{{{int next_srh(void *argc)
int next_srh(void *argc)
{
	union OFFSET *kr,*kw,uf;
	char *c,*mc,*v,*tbf; //2016-10-27添加tbf，在内函数中临时申请的内存缓冲，用于保持二次查询的结果
	unsigned char *md;
	int ret,len,i,j,k,m,n,tt,yy;
	struct pid *kpid;
	struct task_struct *t_str;
	struct mm_struct *mm_str;
	struct vm_area_struct *vadr;
	struct page **pages;
	unsigned int adr;
	v=NULL;mm_str=NULL;tbf=NULL;
	tbf=(char*)vmalloc(K_BF_SIZE); //十万
	if(tbf==NULL)
	{
		printk("<1>tbf vmalloc error\n");
		goto nerr_01;
	}
	kpid=find_get_pid(kv1.pid);
	if(kpid==NULL)
	{
		printk("<1> find_get_pid error\n");
		goto nerr_01;
	}
	t_str=pid_task(kpid,PIDTYPE_PID);
	if(t_str==NULL)
	{
		printk("<1>pid_task error\n");
		goto nerr_01;
	}
	mm_str=get_task_mm(t_str);
	if(mm_str==NULL)
	{
		printk("<1>get_task_mm error\n");
		goto nerr_01;
	}
	kv1.seg[0]=mm_str->start_code;
	kv1.seg[1]=mm_str->start_data;
	kv1.seg[2]=mm_str->start_brk;
	kv1.seg[3]=mm_str->start_stack;
	kv1.t_len=mm_str->end_code-mm_str->start_code;
	kv1.d_len=mm_str->end_data-mm_str->start_data;
	printk("next:total pages=%lx stack pages=%lx\n",mm_str->total_vm,mm_str->stack_vm);
	//c为结果缓冲区，mc为地址集
   //2016-10-27修改缓冲区的使用	
//	c=&tp[d_begin];mc=&mp[d_begin];
	c=tbf;mc=(char*)kcc;
	kr=(union OFFSET *)mc;
	kw=(union OFFSET *)c;
//	memset(c,0,dlen);
	memset(c,0,K_BF_SIZE);
	printk("<1>search number=%d\n",k_am->snum);
	vadr=mm_str->mmap;m=0;n=0;
	i=0;yy=mm_str->map_count;tt=0;
	md=mc;
	for(i=0;i<yy;i++)
	{
		adr=vadr->vm_start;
		len=vma_pages(vadr);
		if(v!=NULL)
		{vfree(v);v=NULL;}
		v=(char*)vmalloc(sizeof(void*)*(len+1));
		pages=(struct page **)v;
		if(pages==NULL)
		{
			printk("<1>vmalloc error12\n");
			goto nerr_01;
		}
		memset((void*)pages,0,sizeof(void*)*(len+1));
		down_read(&mm_str->mmap_sem);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
		ret=get_user_pages(t_str,mm_str,adr,len,0,0,pages,NULL);
#else
		ret=get_user_pages_remote(t_str,mm_str,adr,len,0,pages,NULL);
#endif
		if(ret<=0)
		{
			up_read(&mm_str->mmap_sem);
			goto stepn_01;
		}
		//md=mc;tt=-1;
		tt=-1;
		for(j=0;j<ret;j++)
		{
			uf.ad=adr+j*4096;
			if(k_am->end0==1)
			{
				if(kr->ad==0)
				{
					if(tt!=-1)
					{
						kunmap(pages[tt]);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
						page_cache_release(pages[tt]);
#else
						put_page(pages[tt]);
#endif
						tt=-1;
					}
					up_read(&mm_str->mmap_sem);
					printk("<1>next search finished!!!\n");
					goto nerr_01;
				}
			}
			if(kr->seg > uf.seg)
			{
stepn_11:				
				if(tt!=-1)
				{
					kunmap(pages[tt]);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
					page_cache_release(pages[tt]);
#else
					put_page(pages[tt]);
#endif
					tt=-1;
				}
				continue;
			}
			if(kr->seg < uf.seg)
			{
stepn_12:				
				if(tt!=-1)
				{
					kunmap(pages[tt]);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
					page_cache_release(pages[tt]);
#else
					put_page(pages[tt]);
#endif
					tt=-1;
				}
				kr++;m++;
				if(m>=kcc_cnt)
				{
					up_read(&mm_str->mmap_sem);
					memset(kcc,0,K_CC_SIZE);
					kcc_cnt=n;
					if(n>0)
						memcpy(kcc,tbf,n*4);
					goto nerr_01;
				}
/*				if(((void*)kr-(void*)mc)>7996)//需要用户输入新地址集
				{
					s_sync(12);
					kr=(union OFFSET *)mc;
				}*/
				j--; //重新和现在的pages比较
				continue;
			}//执行到这里，seg保持一致了。开始比较page
			if(kr->page > uf.page)//继续加载pages
				goto stepn_11; //与seg比较执行相同的代码
			if(kr->page < uf.page)
				goto stepn_12; //与seg比较执行相同的代码
//执行到这里，就可以保证地址集中当前的地址（绝对地址）与pages一致了
			md=(unsigned char*)kmap(pages[j]);
			tt=j;//表示进入了kmap阶段，退出时要考虑kunmap
stepn_13:			
			k=0; //开始比较
			if(k_am->snum<0x100)
			{
				if(md[kr->off]==k_am->sch[0])
					k=1;//find
				goto stepn_14; //n_01;
			}
			if(k_am->snum<0x10000)
			{
				if((k_am->sch[0]==md[kr->off]) && (k_am->sch[1]==md[kr->off+1]))
					k=1;
				goto stepn_14;
			}
			if(k_am->snum<0x1000000)
			{
				if((k_am->sch[0]==md[kr->off]) &&(k_am->sch[1]==md[kr->off+1]) && (k_am->sch[2]==md[kr->off+2]))
					k=1;
				goto stepn_14;
			}
			if((k_am->sch[0]==md[kr->off]) &&(k_am->sch[1]==md[kr->off+1]) && (k_am->sch[2]==md[kr->off+2]) && (k_am->sch[3]==md[kr->off+3]))
				k=1;
stepn_14:
			if(k==1)//find it
			{
				kw->ad=kr->ad;kw++;n++;
				if(n>AREA_SIZE_T) //二次查找地址满了，直接退出
				{
					if(tt==-1)
					{
						kunmap(pages[tt]);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
						page_cache_release(pages[tt]);
#else
						put_page(pages[tt]);
#endif
						tt=-1;
					}
					kcc_cnt=n;
					memset(kcc,0,K_CC_SIZE);
					memcpy(kcc,tbf,n*4);
					up_read(&mm_str->mmap_sem);
					printk("<1>next search finished!!\n");
					goto nerr_01;
				}
	/*			if(((void*)kw-(void*)c)>7996)//新的地址集写满一页
				{
					s_sync(10);
					memset(c,0,dlen);
					kw=(union OFFSET *)c;
				}*/
			}
			kr++;
		/*	if(((void*)kr-(void*)mc)>7996)//要求输入新的地址集
			{
				s_sync(12);
				kr=(union OFFSET *)mc;
			}*/
			//这里判断下
			if(kr->ad == 0)
			{
				if(tt!=-1)
				{
					kunmap(pages[tt]);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
					page_cache_release(pages[tt]);
#else
					put_page(pages[tt]);
#endif
					tt=-1;
				}
				up_read(&mm_str->mmap_sem);
				memset(kcc,0,K_CC_SIZE);
				kcc_cnt=n;
				if(n>0)
					memcpy(kcc,tbf,n*4);
				printk("<1>next search finished!!!!!\n");
				goto nerr_01;
			}//到这里，不管之前的kr匹配与否，kr都改为新的待查地址了
/*如果新的kr的物理地址仍然在这个pages里面，则仍要继续比较，而这个for
 循环是按照pages递增的，所以，当新的kr仍然在当前这个pages中，还要继
 续比较，这里使用goto完成这个比较*/
			if((kr->seg == uf.seg) && (kr->page == uf.page))
				goto stepn_13;
			if(tt!=-1)
			{
				kunmap(pages[tt]);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
				page_cache_release(pages[tt]);
#else
				put_page(pages[tt]);
#endif
				tt=-1;
			}
		}//ok~~
		up_read(&mm_str->mmap_sem);
stepn_01:
		pages=NULL;
		vadr=vadr->vm_next;
		if(vadr==NULL)
		{
			printk("<1>find enddddd\n");
			break;
		}
//		printk("<3> %d seg search finished\n",i);
		if(tt!=-1)
			printk("<3> fuck me!\n");
	}
	printk("<1>ready to exit!\n");
nerr_01:
	if(tbf!=NULL)
	{vfree(tbf);tbf=NULL;}
	if(v!=NULL)
	{vfree(v);v=NULL;}
	k_am->end0=1;
	k_am->end1=0;
//	c=&tp[d_begin];mc=&mp[d_begin];
	c=&tp[d_begin];
	memset(c,0,dlen);
	if(kcc_cnt>0)
	{
		n=kcc_cnt>100?100:kcc_cnt*4;
		memcpy(c,kcc,n);
	}
	s_sync(12); //写出最后的地址
	kv1.thread_lock=0;
	printk("<1>next search thread exit....\n");
	do_exit(0); //thread exit
	return 0;
}//}}}
//{{{int lock_srh(void *argc)
int lock_srh(void *argc)
{
	struct pid *kpid;
	struct task_struct *t_str;
	struct mm_struct *mm_str;
	struct vm_area_struct *vadr;
	struct page **pages;
//	struct KVAR_LOCK	*lk;
	char *v;
	unsigned char *c;
	unsigned int ui,uj;
	int i,j,k,m,ret,len;
	unsigned long adr;
	struct KLOCK_STR *lock;
	lock=&kloc_s[0];
	memset((void*)&kloc_s[1],0,sizeof(struct KLOCK_STR));
	kloc_s[1].vm=NULL;
//	k_am=(struct KVAR_AM *)mp;
	kpid=find_get_pid(kv1.pid);
	v=NULL;c=NULL;
	if(kpid==NULL)
	{
		printk("<1>find_get_pid error\n");
		goto lerr_01;
	}
	t_str=pid_task(kpid,PIDTYPE_PID);
	if(t_str==NULL)
	{
		printk("<1>pid_task error\n");
		goto lerr_01;
	}
	mm_str=get_task_mm(t_str);
	if(mm_str==NULL)
	{
		printk("<1>get_task_mm error\n");
		goto lerr_01;
	}
	j=0;
	for(i=0;i<16;i++)
	{
		if(lock->lk[i].offset.ad==0)
		{
			lock->flg[i]=0;
			continue;
		}
		vadr=find_vma(mm_str,lock->lk[i].offset.ad);
		if(vadr==NULL)
		{
			printk("<1>find_vma error\n");
			goto lerr_01;
		}
		if(lock->vm==NULL)
		{lock->vm=vadr;lock->flg[i]=1;continue;}
		if(vadr!=lock->vm)
		{
			if((kloc_s[1].vm != NULL) && (kloc_s[1].vm != vadr))
			{//最多允许2个vma
				printk("<1>allowed two vma\n");
				goto lerr_01;
			}
			if(kloc_s[1].vm == NULL)
				kloc_s[1].vm=vadr;
			kloc_s[1].lk[j].offset.ad=lock->lk[i].offset.ad;
			kloc_s[1].lk[j].maxd=lock->lk[i].maxd;
			kloc_s[1].lk[j].mind=lock->lk[i].mind;
			kloc_s[1].flg[j++]=1;
			lock->flg[i]=0;
			lock->lk[i].offset.ad=0;
		}
	}//上面的循环将待锁定的地址分为两个不同的vma(只允许两个)
	i=0;
	while(1)
	{
		if(i==0)
		{i=1;lock=&kloc_s[0];}
		else
		{
			i=0;
			if(kloc_s[1].vm == NULL)
			{//经过测试锁定最好的使用规则是一次全部地址锁定，间隔较长最好
				lock=&kloc_s[0];
			}
			else
				lock=&kloc_s[1];
		}
		vadr=lock->vm;
		adr=vadr->vm_start;
		len=vma_pages(vadr);
		if(len<=0)
		{
			printk("<1>vma_pages error\n");
			goto lerr_01;
		}
		if(v!=NULL)
		{vfree(v);v=NULL;}
		v=(char*)vmalloc(sizeof(void*)*(len+1));
		pages=(struct page **)v;
		if(pages==NULL)
		{
			printk("<1>vmalloc error\n");
			goto lerr_01;
		}
		memset((void*)pages,0,sizeof(void*)*(len+1));
		down_write(&mm_str->mmap_sem);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
		ret=get_user_pages(t_str,mm_str,adr,len,0,0,pages,NULL);
#else
		ret=get_user_pages_remote(t_str,mm_str,adr,len,0,pages,NULL);
#endif
		if(ret<=0)
		{
			up_write(&mm_str->mmap_sem);
			printk("<1>get_user_pages error\n");
			goto lerr_01;
		}
		j=0;
locka_01:
		if(lock->flg[j]==0)
		{
			j++;
			if(j>15)
			{
				up_write(&mm_str->mmap_sem);
				continue;
			}
			goto locka_01;
		}
		ui=lock->lk[j].offset.ad;ui/=0x1000;
		uj=adr/0x1000;m=-1;
		for(k=0;k<ret;k++)
		{
			if(ui == uj)
			{m=k;break;}
			if(uj < ui)
				uj++;
			else
				break;
		}
		if(m == -1) //不可能找不到
		{
			up_write(&mm_str->mmap_sem);
			printk("<1>fuck this!\n");
			goto lerr_01;
		}
		c=(unsigned char *)kmap(pages[m]);
		if(c == NULL)
		{
			up_write(&mm_str->mmap_sem);
			printk("<1>func this too! \n");
			goto lerr_01;
		}
locka_02:
		ui=lock->lk[j].offset.ad;ui%=0x1000;
		if(lock->lk[j].maxd == 0) //不锁定上限
		{
			if(lock->lk[j].mind < 0x100)
			{
				if(lock->lk[j].mind>c[ui])
					c[ui]=lock->lk[j].mind;
				goto lockb_01;
			}
			if(lock->lk[j].mind < 0x10000)
			{
				if(ui>4094)
					goto lockb_01;
				if(lock->lk[j].mind>(c[ui]+c[ui+1]*0x100))
				{
					c[ui]=lock->lk[j].mind%0x100;
					c[ui+1]=lock->lk[j].mind/0x100;
				}
				goto lockb_01;
			}
			if(lock->lk[j].mind < 0x1000000)
			{
				if(ui>4093)
					goto lockb_01;
				if(lock->lk[j].mind>(c[ui]+c[ui+1]*0x100+c[ui+2]*0x10000))
				{
					c[ui]=lock->lk[j].mind%0x100;
					c[ui+1]=(lock->lk[j].mind/0x100)%0x100;
					c[ui+2]=lock->lk[j].mind/0x10000;
				}
				goto lockb_01;
			}
			if(ui>4092)
				goto lockb_01;
			if(lock->lk[j].mind>(c[ui]+c[ui+1]*0x100+c[ui+2]*0x10000+c[ui]*0x1000000))
			{
				c[ui]=lock->lk[j].mind%0x100;
				c[ui+1]=(lock->lk[j].mind/0x100)%0x100;
				c[ui+2]=(lock->lk[j].mind/0x10000)%0x100;
				c[ui+3]=lock->lk[j].mind/0x1000000;
			}
			goto lockb_01;
		}//完成锁定下限的操作。
		else//锁定上限
		{
			if(lock->lk[j].maxd < 0x100)
			{
				if(lock->lk[j].maxd < c[ui])
					c[ui]=lock->lk[j].maxd;
				goto lockb_01;
			}
			if(lock->lk[j].maxd < 0x10000)
			{
				if(ui>4094)
					goto lockb_01;
				if(lock->lk[j].maxd < (c[ui]+c[ui+1]*0x100))
				{
					c[ui]=lock->lk[j].maxd%0x100;
					c[ui+1]=lock->lk[j].maxd/0x100;
				}
				goto lockb_01;
			}
			if(lock->lk[j].maxd < 0x1000000)
			{
				if(ui < 4093)
					goto lockb_01;
				if(lock->lk[j].maxd < (c[ui]+c[ui+1]*0x100+c[ui+2]*0x10000))
				{
					c[ui]=lock->lk[j].maxd%0x100;
					c[ui+1]=(lock->lk[j].maxd/0x100)%0x100;
					c[ui+2]=lock->lk[j].maxd/0x10000;
				}
				goto lockb_01;
			}
			if(ui < 4092)
				goto lockb_01;
			if(lock->lk[j].maxd < (c[ui]+c[ui+1]*0x100+c[ui+2]*0x10000+c[ui+3]*0x1000000))
			{
				c[ui]=lock->lk[j].maxd%0x100;
				c[ui+1]=(lock->lk[j].maxd/0x100)%0x100;
				c[ui+2]=(lock->lk[j].maxd/0x10000)%0x100;
				c[ui+3]=lock->lk[j].maxd/0x1000000;
			}
			goto lockb_01;
		}
lockb_01:
		if(tp[2]==1)
		{
			kunmap(pages[m]);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
			page_cache_release(pages[m]);
#else
			put_page(pages[m]);
#endif
			up_write(&mm_str->mmap_sem);
			goto lerr_01;
		}
		if(j>=15)
		{//本区域查找完
			kunmap(pages[m]);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
			page_cache_release(pages[m]);
#else
			put_page(pages[m]);
#endif
			up_write(&mm_str->mmap_sem);
			set_current_state(TASK_INTERRUPTIBLE);
			schedule_timeout(HZ*6);
			continue;
		//	goto locka_01;
		}
		if(lock->flg[j+1] == 0)
		{j++;goto lockb_01;}
		if((lock->lk[j+1].offset.ad/0x1000) == uj)
		{j++;goto locka_02;} //这里是判断下一个锁定地址是否在同一页内，若是则不用执行kunmap,kmap而直接跳转至locka_02即可
//执行到这里，说明下一地址不在同一页中，需要释放该页，重新锁定新页！
		kunmap(pages[m]);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
		page_cache_release(pages[m]);
#else
		put_page(pages[m]);
#endif
		//每执行完一页，允许中断
		j++;
		goto locka_01;
	}
////////////////
lerr_01:
	if(v!=NULL)
	{vfree(v);v=NULL;}
/*
	for(i=0;i<16;i++)
	{
		if(lock->lk[i].offset.ad==0)
			continue;
		printk("<1>addr: %x max: %d min: %d\n",lock->lk[i].offset.ad,lock->lk[i].maxd,lock->lk[i].mind);
	} */
	printk("<2>lock finished!\n");
	kv1.pin=1;
	kv1.thread_lock=0;
	do_exit(0);
	return 0;
}//}}}
//{{{int mem_cmp(void *argc)
int mem_cmp(void *argc)
{//2016-11-5添加,完成读取40页内存数据的线程函数
	struct pid *kpid;
	struct task_struct *t_str;
	struct mm_struct *mm_str;
	struct vm_area_struct *vadr;
	struct page **pages;
	union OFFSET oft;
	char *v,*p;
	unsigned char *c;
	int i,j,k,ret,len;
	unsigned long adr;
	struct KVAR_AM	*am;
	am=(struct KVAR_AM *)kcc;
	v=NULL;p=(char *)&(kcc[d_begin]);
	kpid=find_get_pid(kv1.pid);
	if(kpid==NULL)
	{
		printk("<1>find_get_pid error\n");
		goto mmerr_01;
	}
	t_str=pid_task(kpid,PIDTYPE_PID);
	if(t_str==NULL)
	{
		printk("<1>pid_task error\n");
		goto mmerr_01;
	}
	mm_str=get_task_mm(t_str);
	if(mm_str==NULL)
	{
		printk("<1>get_task_mm error\n");
		goto mmerr_01;
	}
	kv1.seg[0]=mm_str->start_code;//第一个是代码段。
	kv1.seg[1]=mm_str->start_data;//第二个是数据段
	kv1.seg[2]=mm_str->start_brk; //第三个是分配的堆段
	kv1.seg[3]=mm_str->start_stack;//第四个是堆栈段。
//	kv1.t_len=mm_str->end_code-mm_str->start_code;//代码段长度
//	kv1.d_len=mm_str->end_data-mm_str->start_data;//数据段长度
	i=mm_str->end_code-mm_str->start_code;
	j=mm_str->end_data-mm_str->start_data;
	if((i!=kv1.t_len) || (j!=kv1.d_len))
	{
		printk("<1>target process unequal\n");
		goto mmerr_01;
	}
	oft.ad=kv1.snum;
	vadr=find_vma(mm_str,oft.ad);
	if(vadr==NULL)
	{
		printk("<1>find_vma error\n");
		goto mmerr_01;
	}
	adr=vadr->vm_start;
//	kv1.fpg=(unsigned int)adr;
	len=vma_pages(vadr);
	if(len<=0)
	{
		printk("<1>vma_pages error\n");
		goto mmerr_01;
	}
	v=(char*)vmalloc(sizeof(void*)*(len+1));
	pages=(struct page **)v;
	if(pages==NULL)
	{
		printk("<1>vmalloc error\n");
		goto mmerr_01;
	}
	memset((void*)pages,0,sizeof(void*)*(len+1));
	down_read(&mm_str->mmap_sem);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
	ret=get_user_pages(t_str,mm_str,adr,len,0,0,pages,NULL);
#else
		ret=get_user_pages_remote(t_str,mm_str,adr,len,0,pages,NULL);
#endif
	if(ret<=0)
	{
		printk("<1> get_user_pages error\n");
		up_read(&mm_str->mmap_sem);
		goto mmerr_01;
	}
	k=(oft.ad-adr)/0x1000;
	if(k>=ret)
	{
		printk("<1>adr error\n");
		up_read(&mm_str->mmap_sem);
		goto mmerr_01;
	}
	kv1.fpg=adr+k*0x1000;
	j=(k+40)>ret?ret:(k+40);
	kv1.tpg=j-k;
	for(i=k;i<j;i++)
	{
		c=(unsigned char*)kmap(pages[i]);
		if(c==NULL)
		{
			kunmap(pages[i]);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
			page_cache_release(pages[i]);
#else
			put_page(pages[i]);
#endif
			up_read(&mm_str->mmap_sem);
			printk("<1>kmap error\n");
			goto mmerr_01;
		}
		memcpy(p,c,4096);
		p+=4096;
		kunmap(pages[i]);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
		page_cache_release(pages[i]);
#else
		put_page(pages[i]);
#endif
	}
	up_read(&mm_str->mmap_sem);
	if(v!=NULL)
		vfree(v);
	am->t_seg=kv1.seg[0];
	am->d_seg=kv1.seg[1];
	am->b_seg=kv1.seg[2];
	am->s_seg=kv1.seg[3];
	am->t_len=kv1.t_len;
	am->d_len=kv1.d_len;
	am->fin_pg=kv1.fpg;
	am->tol_pg=kv1.tpg;
	am->sync=1;
	kv1.pin=3;
	kv1.thread_lock=0;
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(2*HZ);
	printk("<1>mem_cmp finished~~~\n");
	do_exit(0);
	return 0;
//	k_am->tol_pg=1;//mm_str->total_vm;
//	k_am->fin_pg=1;//这两项是用于进度条显示进度的。
mmerr_01:
	if(v!=NULL)
		vfree(v);
	kv1.pin=1;//---------------------
	kv1.thread_lock=0;
	kv1.fpg=0;
	kv1.tpg=0;
	do_exit(0);
	return 0;
}//}}}

module_init(minedev_init);
module_exit(minedev_exit);




