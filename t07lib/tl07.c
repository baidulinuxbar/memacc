#include"tl07.h"

//{{{ void crt_window(int a,char **b)
void crt_window(int a,char **b)
{
	GtkWidget *item;
	GtkTreeIter iter;
	int i,j,k;
	char ch[36];
//	if(!g_thread_supported())
//		g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&a,&b);
	ws.window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(ws.window),wintitle);
	gtk_widget_set_size_request(ws.window,win_x,win_y);
	gtk_window_set_resizable(GTK_WINDOW(ws.window),FALSE);
	gtk_window_set_position(GTK_WINDOW(ws.window),GTK_WIN_POS_CENTER);
	gtk_window_set_icon(GTK_WINDOW(ws.window),crt_pixbuf(iconfile));
	ws.fixed=gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(ws.window),ws.fixed);
	crt_treeview1();
	crt_tv1_bnt();
	crt_part2();
	crt_part3();
	memset(ch,0,36);
	//gtk_list_store_append(ws.store[1],&iter);
	//gtk_list_store_set(ws.store[1],&iter,0,"生   命   值",1,"0x884893823",2,155,3,245,-1);
	//gtk_list_store_append(ws.store[1],&iter);
	//gtk_list_store_set(ws.store[1],&iter,0,"武器1",1,"0x884920230",2,16,-1);
	crt_statusicon();
	gtk_widget_show_all(ws.window);
	g_signal_connect(G_OBJECT(ws.window),"delete-event",G_CALLBACK(hide_window),NULL);
//	g_signal_connect_swapped(G_OBJECT(ws.window),"destroy",G_CALLBACK(gtk_main_quit),NULL);
	ws.thread_lock=0;ws.pid=0;ws.sn=0;
	for(i=0;i<max_res;i++)
		memset((void*)&sl[i],0,sizeof(sl[i]));
	mem_u.ad=0;
	gdk_threads_enter();
	gtk_main();
	gdk_threads_leave();
}//}}}
//{{{ GdkPixbuf *crt_pixbuf(gchar *fname)
GdkPixbuf *crt_pixbuf(gchar *fname)
{
	GdkPixbuf *pix;
	GError *error=NULL;
	pix=gdk_pixbuf_new_from_file(fname,&error);
	if(!pix)
	{
		g_print("%s\n",error->message);
		g_error_free(error);
	}
	return pix;
}//}}}
//{{{ void crt_treeview1()
void crt_treeview1()
{
	GtkTreeModel *model;
	GtkTreeViewColumn *column;
	GtkCellRenderer	*render;
//把label1的建立代码移至该函数中，该函数建立的三个控件：label1,treeview,scroll1	
	ws.label=gtk_label_new("");
	//gtk_widget_set_size_request(ws.label,lab1_w,lab1_h);
	gtk_label_set_markup(GTK_LABEL(ws.label),lab1);
	ws.store[0]=gtk_list_store_new(2,G_TYPE_INT,G_TYPE_STRING);
	model=GTK_TREE_MODEL(ws.store[0]);
	ws.list[0]=gtk_tree_view_new_with_model(model);
	render=gtk_cell_renderer_text_new();
	column=gtk_tree_view_column_new_with_attributes("PID",render,"text",0,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(ws.list[0]),column);
	render=gtk_cell_renderer_text_new();
//	g_object_set(G_OBJECT(render),"editable",TRUE);  //可编辑
	column=gtk_tree_view_column_new_with_attributes("进程",render,"text",1,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(ws.list[0]),column);
	gtk_widget_set_size_request(ws.list[0],t1_w,t1_h);
	ws.scroll[0]=gtk_vscrollbar_new(gtk_tree_view_get_vadjustment(GTK_TREE_VIEW(ws.list[0])));
	gtk_widget_set_size_request(ws.scroll[0],srl_w,srl_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.label,lab1_px,lab1_py);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.list[0],t1_px,t1_py);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.scroll[0],srl_px,srl_py);
	g_signal_connect(G_OBJECT(ws.list[0]),"row-activated",G_CALLBACK(on_tree1_dblclk),NULL);
}//}}}
//{{{ void crt_tv1_bnt()
void crt_tv1_bnt()
{
	ws.bnt[0]=gtk_button_new_with_label("取得进程");
	gtk_widget_set_size_request(ws.bnt[0],bnt1_w,bnt1_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.bnt[0],bnt1_px,bnt1_py);
	ws.bnt[1]=gtk_button_new_with_label("重    置");
	gtk_widget_set_size_request(ws.bnt[1],bnt2_w,bnt2_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.bnt[1],bnt2_px,bnt2_py);
	g_signal_connect(G_OBJECT(ws.bnt[0]),"clicked",G_CALLBACK(on_getproc),NULL);
	g_signal_connect(G_OBJECT(ws.bnt[1]),"clicked",G_CALLBACK(on_reset),NULL);
}//}}}
//{{{ void crt_part2()
void crt_part2()
{
	GtkWidget *label;
	GSList *l;
	label=gtk_label_new("");
	//gtk_widget_set_size_request(label,lab2_w,lab2_h);
	gtk_label_set_markup(GTK_LABEL(label),lab2);
	gtk_fixed_put(GTK_FIXED(ws.fixed),label,lab2_px,lab2_py);
	ws.label=gtk_label_new("");//从现在起，该指针将保留用于输入label的使用
	//gtk_widget_set_size_request(ws.label,labi_w,labi_h);
	//gtk_label_set_markup(GTK_LABEL(ws.label),lab5);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.label,labi_px,labi_py);
	label=gtk_label_new("");
	//gtk_widget_set_size_request(label,lab3_w,lab3_h);
	gtk_label_set_markup(GTK_LABEL(label),lab3);
	gtk_fixed_put(GTK_FIXED(ws.fixed),label,lab3_px,lab3_py);
	ws.entry[0]=gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(ws.entry[0]),60);
	gtk_widget_set_size_request(ws.entry[0],entry1_w,entry1_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.entry[0],entry1_px,entry1_py);
	ws.radio[0]=gtk_radio_button_new(NULL);
	label=gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(label),rad0);
	gtk_container_add(GTK_CONTAINER(ws.radio[0]),label);
	l=gtk_radio_button_get_group(GTK_RADIO_BUTTON(ws.radio[0]));
	ws.radio[1]=gtk_radio_button_new(l);
	label=gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(label),rad1);
	gtk_container_add(GTK_CONTAINER(ws.radio[1]),label);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.radio[0],rd1_px,rd1_py);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.radio[1],rd2_px,rd2_py);
	ws.prog=gtk_progress_bar_new();
	gtk_widget_set_size_request(ws.prog,pro_w,pro_h);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(ws.prog),"进度%");
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.prog,pro_px,pro_py);
	ws.bnt[2]=gtk_button_new_with_label("首次查找");
	gtk_widget_set_size_request(ws.bnt[2],bnt3_w,bnt3_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.bnt[2],bnt3_px,bnt3_py);
	ws.bnt[3]=gtk_button_new_with_label("再次查找");
	gtk_widget_set_size_request(ws.bnt[3],bnt4_w,bnt4_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.bnt[3],bnt4_px,bnt4_py);
	ws.bnt[4]=gtk_button_new_with_label("进制转换");
	gtk_widget_set_size_request(ws.bnt[4],bnt5_w,bnt5_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.bnt[4],bnt5_px,bnt5_py);
//2016-11-1添加	
	ws.label_nn=gtk_label_new("");
	gtk_widget_set_size_request(ws.label_nn,labnn_w,labnn_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.label_nn,labnn_px,labnn_py);
//添加按钮的消息响应函数
	g_signal_connect(G_OBJECT(ws.bnt[2]),"clicked",G_CALLBACK(on_first_srh),NULL);
	g_signal_connect(G_OBJECT(ws.bnt[3]),"clicked",G_CALLBACK(on_next_srh),NULL);
	g_signal_connect(G_OBJECT(ws.bnt[4]),"clicked",G_CALLBACK(on_calc),NULL);
}//}}}
//{{{ void crt_part3()
void crt_part3()
{
	GtkTreeModel *model;
	GtkTreeViewColumn *column;
	GtkCellRenderer	*render;
	GtkWidget *label;
	label=gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(label),lab4);
	gtk_fixed_put(GTK_FIXED(ws.fixed),label,lab4_px,lab4_py);
	label=gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(label),lab5);
	gtk_fixed_put(GTK_FIXED(ws.fixed),label,lab5_px,lab5_py);
	ws.entry[1]=gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(ws.entry[1]),60);
	gtk_widget_set_size_request(ws.entry[1],entry2_w,entry2_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.entry[1],entry2_px,entry2_py);
	ws.entry[2]=gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(ws.entry[2]),60);
	gtk_widget_set_size_request(ws.entry[2],entry3_w,entry3_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.entry[2],entry3_px,entry3_py);
	label=gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(label),lab6);
	gtk_fixed_put(GTK_FIXED(ws.fixed),label,lab6_px,lab6_py);
	label=gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(label),lab7);
	gtk_fixed_put(GTK_FIXED(ws.fixed),label,lab7_px,lab7_py);
	ws.entry[3]=gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(ws.entry[3]),60);
	gtk_widget_set_size_request(ws.entry[3],entry4_w,entry4_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.entry[3],entry4_px,entry4_py);
	ws.entry[4]=gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(ws.entry[4]),60);
	gtk_widget_set_size_request(ws.entry[4],entry5_w,entry5_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.entry[4],entry5_px,entry5_py);
	ws.bnt[5]=gtk_button_new_with_label("添加锁定");
	gtk_widget_set_size_request(ws.bnt[5],bnt6_w,bnt6_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.bnt[5],bnt6_px,bnt6_py);
	ws.bnt[6]=gtk_button_new_with_label("删除锁定");
	gtk_widget_set_size_request(ws.bnt[6],bnt7_w,bnt7_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.bnt[6],bnt7_px,bnt7_py);
	ws.store[1]=gtk_list_store_new(4,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_INT,G_TYPE_INT);
	model=GTK_TREE_MODEL(ws.store[1]);
	ws.list[1]=gtk_tree_view_new_with_model(model);
	render=gtk_cell_renderer_text_new();
	column=gtk_tree_view_column_new_with_attributes("摘  要",render,"text",0,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(ws.list[1]),column);
	render=gtk_cell_renderer_text_new();
	column=gtk_tree_view_column_new_with_attributes("锁定地址",render,"text",1,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(ws.list[1]),column);
	render=gtk_cell_renderer_text_new();
	column=gtk_tree_view_column_new_with_attributes("下限",render,"text",2,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(ws.list[1]),column);
	render=gtk_cell_renderer_text_new();
	column=gtk_tree_view_column_new_with_attributes("上限",render,"text",3,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(ws.list[1]),column);
	gtk_widget_set_size_request(ws.list[1],t2_w,t2_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.list[1],t2_px,t2_py);
	ws.bnt[7]=gtk_button_new_with_label("保存锁定");
	gtk_widget_set_size_request(ws.bnt[7],bnt8_w,bnt8_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.bnt[7],bnt8_px,bnt8_py);
	ws.bnt[8]=gtk_button_new_with_label("锁    定");
	gtk_widget_set_size_request(ws.bnt[8],bnt9_w,bnt9_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.bnt[8],bnt9_px,bnt9_py);
	ws.bnt[9]=gtk_button_new_with_label("装载锁定");
	gtk_widget_set_size_request(ws.bnt[9],bnt10_w,bnt10_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.bnt[9],bnt10_px,bnt10_py);
	ws.store[2]=gtk_list_store_new(3,G_TYPE_INT,G_TYPE_INT,G_TYPE_STRING);
	model=GTK_TREE_MODEL(ws.store[2]);
	ws.list[2]=gtk_tree_view_new_with_model(model);
	render=gtk_cell_renderer_text_new();
	column=gtk_tree_view_column_new_with_attributes("段序号",render,"text",0,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(ws.list[2]),column);
	render=gtk_cell_renderer_text_new();
	column=gtk_tree_view_column_new_with_attributes("页序号",render,"text",1,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(ws.list[2]),column);
	render=gtk_cell_renderer_text_new();
	column=gtk_tree_view_column_new_with_attributes("扫描地址",render,"text",2,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(ws.list[2]),column);
	gtk_widget_set_size_request(ws.list[2],t3_w,t3_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.list[2],t3_px,t3_py);
	ws.scroll[1]=gtk_vscrollbar_new(gtk_tree_view_get_vadjustment(GTK_TREE_VIEW(ws.list[2])));
	gtk_widget_set_size_request(ws.scroll[1],srl2_w,srl2_h);
	gtk_fixed_put(GTK_FIXED(ws.fixed),ws.scroll[1],srl2_px,srl2_py);
	//消息响应
	g_signal_connect(G_OBJECT(ws.bnt[5]),"clicked",G_CALLBACK(on_addlock),NULL);
	g_signal_connect(G_OBJECT(ws.bnt[6]),"clicked",G_CALLBACK(on_dellock),NULL);
	g_signal_connect(G_OBJECT(ws.bnt[7]),"clicked",G_CALLBACK(on_save),NULL);
	g_signal_connect(G_OBJECT(ws.bnt[8]),"clicked",G_CALLBACK(on_lock),NULL);
	g_signal_connect(G_OBJECT(ws.bnt[9]),"clicked",G_CALLBACK(on_load),NULL);
	g_signal_connect(G_OBJECT(ws.list[2]),"row-activated",G_CALLBACK(on_tree3_dblclk),NULL);
	g_signal_connect(G_OBJECT(ws.list[1]),"row-activated",G_CALLBACK(on_tree2_dblclk),NULL);
}//}}}
//{{{ void on_getproc(GtkWidget *widget,gpointer gp)
void on_getproc(GtkWidget *widget,gpointer gp)
{
	FILE *file;
	int i,j;
	char ch[128],c[128];
	GtkTreeIter iter;
	//先调用清空函数
	gtk_list_store_clear(ws.store[0]);
	system(ps_cmd);
	file=fopen(proc_file,"r");
	if(file==NULL)
	{
		msgbox("打开文件失败！");
		return;
	}
	memset(ch,0,sizeof(ch));i=0;
	while(fgets(ch,sizeof(ch),file)!=NULL)
	{
		if(i==0)
		{//分割空格都是对齐的，这里确定分割空格的位置：
			for(i=0;i<strlen(ch);i++)
			{
				if(ch[i]=='D')
				{i++;break;}
			}//i保存了空格的位置
			memset(ch,0,sizeof(ch));
			continue;
		}
		memset(c,0,sizeof(c));
		memcpy(c,ch,i);
		j=atoi(c);//得到pid
		gtk_list_store_append(ws.store[0],&iter);
		gtk_list_store_set(ws.store[0],&iter,0,j,1,&ch[i+1],-1);
		memset(ch,0,sizeof(ch));	
	}
	fclose(file);
}//}}}
//{{{ void on_reset(GtkWidget *widget,gpointer pg)
void on_reset(GtkWidget *widget,gpointer pg)
{
	gtk_list_store_clear(ws.store[0]);
	gtk_list_store_clear(ws.store[2]);
	gtk_label_set_text(GTK_LABEL(ws.label),"");
}//}}}
//{{{ void msgbox(char *gc)
void msgbox(char *gc)
{
	GtkWidget *dialog;
	GtkMessageType type;
	type=GTK_MESSAGE_INFO;
	dialog=gtk_message_dialog_new(NULL,GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,type,GTK_BUTTONS_OK,(gchar *)gc);
	gtk_window_set_position(GTK_WINDOW(dialog),GTK_WIN_POS_CENTER);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}//}}}
//{{{ void on_tree1_dblclk(GtkTreeView *treeview,GtkTreePath *path,GtkTreeViewColumn *col,gpointer userdata)
void on_tree1_dblclk(GtkTreeView *treeview,GtkTreePath *path,GtkTreeViewColumn *col,gpointer userdata)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GValue value,va;
	char ch[200];
	memset((void*)&value,0,sizeof(value));
	model=gtk_tree_view_get_model(treeview);
	if(gtk_tree_model_get_iter(model,&iter,path))
	{
		gtk_tree_model_get_value(model,&iter,0,&value);
		ws.pid=g_value_get_int(&value);//get pid
		g_value_unset(&value);
		gtk_tree_model_get_value(model,&iter,1,&value);
		memset(ch,0,sizeof(ch));
		snprintf(ch,sizeof(ch),labi,g_value_get_string(&value));
		gtk_label_set_markup(GTK_LABEL(ws.label),ch);
	}
}//}}}
//{{{ void crt_statusicon()
void crt_statusicon()
{
	ws.sicon=gtk_status_icon_new_from_file(iconfile);
	ws.menu=gtk_menu_new();
	ws.menu_restore=gtk_menu_item_new_with_label("恢复窗口");
	ws.menu_mem=gtk_menu_item_new_with_label("内存检视");
	ws.menu_mmem=gtk_menu_item_new_with_label("内存比较");
	ws.menu_set=gtk_menu_item_new_with_label("关    于");
	ws.menu_exit=gtk_menu_item_new_with_label("退    出");
	gtk_menu_shell_append(GTK_MENU_SHELL(ws.menu),ws.menu_restore);
	gtk_menu_shell_append(GTK_MENU_SHELL(ws.menu),ws.menu_mem);
	gtk_menu_shell_append(GTK_MENU_SHELL(ws.menu),ws.menu_mmem);
	gtk_menu_shell_append(GTK_MENU_SHELL(ws.menu),ws.menu_set);
	gtk_menu_shell_append(GTK_MENU_SHELL(ws.menu),ws.menu_exit);
	g_signal_connect(G_OBJECT(ws.menu_restore),"activate",G_CALLBACK(restore_window),NULL);
	g_signal_connect(G_OBJECT(ws.menu_set),"activate",G_CALLBACK(about_window),NULL);
	g_signal_connect(G_OBJECT(ws.menu_exit),"activate",G_CALLBACK(exit_window),NULL);
	g_signal_connect(G_OBJECT(ws.menu_mem),"activate",G_CALLBACK(show_mem),NULL);
	g_signal_connect(G_OBJECT(ws.menu_mmem),"activate",G_CALLBACK(show_mmem),NULL);
	gtk_widget_show_all(ws.menu);
	gtk_status_icon_set_tooltip(ws.sicon,tip_statusicon);
	g_signal_connect(GTK_STATUS_ICON(ws.sicon),"activate",GTK_SIGNAL_FUNC(restore_window),NULL);
	g_signal_connect(GTK_STATUS_ICON(ws.sicon),"popup_menu",GTK_SIGNAL_FUNC(show_menu),NULL);
	gtk_status_icon_set_visible(ws.sicon,FALSE);
}//}}}
//{{{ void restore_window(GtkWidget *widget,gpointer gp)
void restore_window(GtkWidget *widget,gpointer gp)
{
	gtk_widget_show(ws.window);
	gtk_window_deiconify(GTK_WINDOW(ws.window));
	gtk_status_icon_set_visible(ws.sicon,FALSE);
}//}}}
//{{{ void about_window(GtkWidget *widget,gpointer gp)
void about_window(GtkWidget *widget,gpointer gp)
{
//	msgbox("hello world");
	char ch[512];
	GtkWidget *dlg,*fixed,*label,*img;
//	GtkWidget *darea;
	dlg=gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dlg),"关于本软件V:2.0");
	gtk_window_set_position(GTK_WINDOW(dlg),GTK_WIN_POS_CENTER);
	gtk_widget_set_size_request(dlg,300,130);
	gtk_window_set_resizable(GTK_WINDOW(dlg),FALSE);
	fixed=gtk_fixed_new();
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox),fixed,FALSE,FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(dlg),2);/*
	darea=gtk_drawing_area_new();
	gtk_widget_set_size_request(darea,446,296);
	gtk_widget_set_app_paintable(darea,TRUE);
	gtk_fixed_put(GTK_FIXED(fixed),darea,0,0);
	g_signal_connect(G_OBJECT(darea),"expose-event",G_CALLBACK(on_drawing),NULL);*/
	img=gtk_image_new_from_file(myphoto);
	gtk_widget_set_size_request(img,99,110);
	gtk_fixed_put(GTK_FIXED(fixed),img,5,5);
	label=gtk_label_new("");
	memset(ch,0,sizeof(ch));
	snprintf(ch,sizeof(ch),"<span style=\"italic\" face=\"YaHei Consolas Hybrid\" font=\"12\" color=\"#0000ff\">程式设计：tybitsfox</span>");
	gtk_label_set_markup(GTK_LABEL(label),ch);
	gtk_widget_set_size_request(label,200,30);
	gtk_fixed_put(GTK_FIXED(fixed),label,100,40);
	label=gtk_label_new("");
	memset(ch,0,sizeof(ch));
	snprintf(ch,sizeof(ch),"<span font=\"8\" color=\"#ff0000\">bug report:tybitsfox@gmail.com</span>");
	gtk_label_set_markup(GTK_LABEL(label),ch);
	gtk_widget_set_size_request(label,200,30);
	gtk_fixed_put(GTK_FIXED(fixed),label,100,90);
	gtk_widget_show_all(dlg);
	gtk_dialog_run(GTK_DIALOG(dlg));
	gtk_widget_destroy(dlg);
}//}}}

//{{{ gboolean on_drawing(GtkWidget *widget,GdkEventExpose *event,gpointer gp);
gboolean on_drawing(GtkWidget *widget,GdkEventExpose *event,gpointer gp)
{
	int x,y,i,j,k;
	char ch[10];
    cairo_t *cr,*ct,*tt;
	ct=gdk_cairo_create(widget->window);
	cairo_set_source_rgb(ct,0,0,1);
	cairo_set_line_width(ct,0.5);
    cr=gdk_cairo_create(widget->window);
    cairo_set_source_rgb(cr, 1, 0, 1);
    cairo_set_line_width(cr, 0.5);
	x=rt0_x;y=rt0_y;
	cairo_set_font_size(ct,12);
	cairo_set_font_size(cr,12);
	j=16;
	cairo_rectangle(cr,x,y,rt_w,rt_h);
	for(i=0;i<20;i++)
	{
		y+=rt_h;
		cairo_rectangle(cr,x,y,rt_w,rt_h);
		memset(ch,0,sizeof(ch));
		snprintf(ch,sizeof(ch),"%04d",i*16+pn);
		cairo_move_to(ct,x+10,y+j);
		cairo_show_text(ct,ch);
	}
	x=rt0_x+in_x;y=rt0_y;
	j=21*rt_h+y;
	for(i=0;i<17;i++)
	{
		cairo_move_to(cr,x,y);
		cairo_line_to(cr,x,j);
		if(i<16)
		{
			memset(ch,0,sizeof(ch));
			snprintf(ch,sizeof(ch),"%02d",i);
			cairo_move_to(ct,x+5,y+16);
			cairo_show_text(ct,ch);
		}
		x+=in_w;
	}
	x=rt0_x+in_x;y=rt0_y;
	j=x+16*in_w;
	for(i=0;i<22;i++)
	{
		cairo_move_to(cr,x,y);
		cairo_line_to(cr,j,y);
		y+=rt_h;
	}
	//开始写入数据
	for(i=0;i<20;i++)
	{
		x=rt0_x+in_x;y=rt0_y+(i+1)*rt_h;
		for(j=0;j<16;j++)
		{
			k=pn+i*16+j;
			memset(ch,0,sizeof(ch));
			snprintf(ch,sizeof(ch),"%02x",mmp[k]);
			if(ws.show_mode==0)
			{
				if(mmp[k]==mnp[k])
					tt=cr;
				else
					tt=ct;
			}
			else
			{
				if(ws.show_mode==1)//less
				{
					if(mmp[k]<mnp[k])
						tt=cr;
					else
						tt=ct;
				}
				else//more
				{
					if(mmp[k]>mnp[k])
						tt=cr;
					else
						tt=ct;
				}
			}
			cairo_move_to(tt,x+5,y+16);
			cairo_show_text(tt,ch);
			x+=in_w;
		}
	}
	cairo_stroke(ct);
	cairo_destroy(ct);
    cairo_stroke(cr);
    cairo_destroy(cr);
    return FALSE;
}//}}}
//{{{ void show_mem(GtkWidget *widget,gpointer gp)
void show_mem(GtkWidget *widget,gpointer gp)
{
	char ch[100];
	int i;
	GtkListStore *store;
	GtkTreeModel *model;
	GtkTreeViewColumn *column;
	GtkCellRenderer *render;
	GtkWidget *dlg,*fixed;
	GtkWidget *entry[3],*label,*bnt;
	ws.mend_srh=0;
	ws.show_mode=0;//默认的显示模式。
	ws.blur_srh=0;
	pn=0;mmp=(unsigned char*)&(ws.g_ch[d_begin]);
	mnp=(unsigned char*)&(ws.g_addr[0][d_begin]);
	gtk_status_icon_set_visible(ws.sicon,FALSE);
	dlg=gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dlg),"内存检视");
	gtk_window_set_position(GTK_WINDOW(dlg),GTK_WIN_POS_CENTER);
	gtk_widget_set_size_request(dlg,win_x,win_y);
	gtk_window_set_resizable(GTK_WINDOW(dlg),FALSE);
	fixed=gtk_fixed_new();
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox),fixed,FALSE,FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(dlg),2);
	ws.darea=gtk_drawing_area_new();
	gtk_widget_set_size_request(ws.darea,m_wid,m_hei);
	gtk_widget_set_app_paintable(ws.darea,TRUE);
	gtk_fixed_put(GTK_FIXED(fixed),ws.darea,0,0);
	label=gtk_label_new("");
	memset(ch,0,sizeof(ch));
	snprintf(ch,sizeof(ch),mlab0,ws.pid);
	gtk_label_set_markup(GTK_LABEL(label),ch);
	gtk_widget_set_size_request(label,pg0_w,pg0_h);
	gtk_fixed_put(GTK_FIXED(fixed),label,pg0_x,pg0_y);
	label=gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(label),mlab1);
	gtk_widget_set_size_request(label,pg_w,pg_h);
	gtk_fixed_put(GTK_FIXED(fixed),label,pg_x,pg_y);
	entry[0]=gtk_entry_new();
	gtk_widget_set_size_request(entry[0],pg_w1,pg_h1);
	memset(ch,0,sizeof(ch));
	if(mem_u.ad!=0)
	{
		snprintf(ch,sizeof(ch),"%X",mem_u.seg);
	}
	else
		snprintf(ch,sizeof(ch),"%02d",3);
	gtk_entry_set_text(GTK_ENTRY(entry[0]),ch);
	gtk_fixed_put(GTK_FIXED(fixed),entry[0],pg_x1,pg_y1);
	label=gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(label),mlab2);
	gtk_widget_set_size_request(label,pg1_w,pg1_h);
	gtk_fixed_put(GTK_FIXED(fixed),label,pg1_x,pg1_y);
	entry[1]=gtk_entry_new();
	gtk_widget_set_size_request(entry[1],pg1_w1,pg1_h1);
	memset(ch,0,sizeof(ch));
	if(mem_u.ad!=0)
	{
		snprintf(ch,sizeof(ch),"%d",mem_u.page);
	}
	else
		snprintf(ch,sizeof(ch),"%05d",8808);
	gtk_entry_set_text(GTK_ENTRY(entry[1]),ch);
	gtk_fixed_put(GTK_FIXED(fixed),entry[1],pg1_x1,pg1_y1);
	ws.mbnt[2]=gtk_button_new_with_label("开始检索");
	gtk_widget_set_size_request(ws.mbnt[2],pgb1_w,pgb1_h);
	gtk_fixed_put(GTK_FIXED(fixed),ws.mbnt[2],pgb1_x,pgb1_y);
	ws.mbnt[0]=gtk_button_new_with_label("上    翻");
	gtk_widget_set_size_request(ws.mbnt[0],pgb2_w,pgb2_h);
	gtk_widget_set_sensitive(ws.mbnt[0],FALSE);
	gtk_fixed_put(GTK_FIXED(fixed),ws.mbnt[0],pgb2_x,pgb2_y);
	ws.mbnt[1]=gtk_button_new_with_label("下    翻");
	gtk_widget_set_size_request(ws.mbnt[1],pgb3_w,pgb3_h);
	gtk_fixed_put(GTK_FIXED(fixed),ws.mbnt[1],pgb3_x,pgb3_y);
	//2014-2-23添加
	ws.combo=gtk_combo_box_new_text();
	gtk_combo_box_append_text(GTK_COMBO_BOX(ws.combo),"显示不同");
	gtk_combo_box_append_text(GTK_COMBO_BOX(ws.combo),"显示变小");
	gtk_combo_box_append_text(GTK_COMBO_BOX(ws.combo),"显示变大");
	gtk_combo_box_set_active(GTK_COMBO_BOX(ws.combo),0);
	gtk_widget_set_size_request(ws.combo,pgc1_w,pgc1_h);
	gtk_fixed_put(GTK_FIXED(fixed),ws.combo,pgc1_x,pgc1_y);
	store=gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_INT);
	model=GTK_TREE_MODEL(store);
	ws.mlist=gtk_tree_view_new_with_model(model);
	render=gtk_cell_renderer_text_new();
	column=gtk_tree_view_column_new_with_attributes("匹配类型",render,"text",0,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(ws.mlist),column);
	render=gtk_cell_renderer_text_new();
	column=gtk_tree_view_column_new_with_attributes("地址偏移",render,"text",1,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(ws.mlist),column);
	gtk_widget_set_size_request(ws.mlist,pglist_w,pglist_h);
	gtk_fixed_put(GTK_FIXED(fixed),ws.mlist,pglist_x,pglist_y);
	ws.mbnt[3]=gtk_button_new_with_label("模糊查询");
	gtk_widget_set_size_request(ws.mbnt[3],pgb4_w,pgb4_h);
	gtk_fixed_put(GTK_FIXED(fixed),ws.mbnt[3],pgb4_x,pgb4_y);

	g_signal_connect(G_OBJECT(ws.darea),"expose-event",G_CALLBACK(on_drawing),NULL);
	g_signal_connect(G_OBJECT(ws.mbnt[1]),"clicked",G_CALLBACK(on_pagedown),(gpointer)ws.darea);
	g_signal_connect(G_OBJECT(ws.mbnt[0]),"clicked",G_CALLBACK(on_pageup),(gpointer)ws.darea);
	g_signal_connect(G_OBJECT(ws.mbnt[2]),"clicked",G_CALLBACK(on_memsrh),NULL);
	g_signal_connect(G_OBJECT(ws.combo),"changed",G_CALLBACK(on_combo_changed),NULL);
	g_signal_connect(G_OBJECT(ws.mbnt[3]),"clicked",G_CALLBACK(on_blursrh),NULL);
	gtk_widget_show_all(dlg);
	gtk_dialog_run(GTK_DIALOG(dlg));
	if(ws.mend_srh!=0)
	{
		ws.mend_srh=0;//线程退出。
		msleep();
		msgbox("closing...");
	}
	gtk_widget_destroy(dlg);
	gtk_status_icon_set_visible(ws.sicon,TRUE);
}//}}}
//{{{ void on_pagedown(GtkWidget *widget,gpointer gp)
void on_pagedown(GtkWidget *widget,gpointer gp)
{
	if(pn<0 || pn>=0xec0)
		return;
	pn+=0x140;
	if(!GTK_WIDGET_IS_SENSITIVE(ws.mbnt[0]))
		gtk_widget_set_sensitive(ws.mbnt[0],TRUE);
	if(pn==0xec0)
		gtk_widget_set_sensitive(ws.mbnt[1],FALSE);
	gtk_widget_queue_draw((GtkWidget*)gp);
}//}}}
//{{{ void on_pageup(GtkWidget *widget,gpointer gp)
void on_pageup(GtkWidget *widget,gpointer gp)
{
	if(pn<0x140)
		return;
	pn-=0x140;
	if(!GTK_WIDGET_IS_SENSITIVE(ws.mbnt[1]))
		gtk_widget_set_sensitive(ws.mbnt[1],TRUE);
	if(pn==0)
		gtk_widget_set_sensitive(ws.mbnt[0],FALSE);
	gtk_widget_queue_draw((GtkWidget*)gp);
}//}}}
//{{{ void exit_window(GtkWidget *widget,gpointer gp)
void exit_window(GtkWidget *widget,gpointer gp)
{
	gtk_main_quit();
}//}}}
//{{{ void show_menu(GtkWidget *widget,guint button,guint32 act_time,gpointer gp)
void show_menu(GtkWidget *widget,guint button,guint32 act_time,gpointer gp)
{
	gtk_menu_popup(GTK_MENU(ws.menu),NULL,NULL,gtk_status_icon_position_menu,widget,button,act_time);

}//}}}
//{{{ gint hide_window(GtkWidget *widget,GdkEvent *event,gpointer gp)
gint hide_window(GtkWidget *widget,GdkEvent *event,gpointer gp)
{
	gtk_widget_hide(ws.window);
	gtk_status_icon_set_visible(ws.sicon,TRUE);
	return TRUE;
}//}}}
//{{{ void on_first_srh(GtkWidget *widget,gpointer gp)
void on_first_srh(GtkWidget *widget,gpointer gp)
{
	char *p,ch[40];
	if(ws.thread_lock==1)
	{
		msgbox("查询中....请稍后操作");
		return;
	}
	if(ws.pid<1)
	{
		msgbox("请先选择待查询的目标进程");
		return;
	}
	p=(char*)gtk_entry_get_text(GTK_ENTRY(ws.entry[0]));
	if(check_input(p))
	{
		msgbox("输入的查询数据有误");
		return;
	}
//	thd_fst(0);
//	g_thread_create(thd_fst,NULL,FALSE,NULL);
	g_thread_new("thd_fst",thd_fst,NULL);
//	p=(char*)gtk_label_get_text(GTK_LABEL(ws.label));


}//}}}
//{{{ int check_input(char *c)
int check_input(char *c)
{
	int i,j,k;
	char ch[20];
	j=strlen(c);
	if(j>10)
		return 1;
	memset(ch,0,sizeof(ch));
	if(ws.sn==0)
		k=0;
	else
	{k=ws.sn;ws.sn=0;}
//取得数据的进制类型
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ws.radio[0])))
	{//十进制
		for(i=0;i<j;i++)
		{
			if(c[i]>0x39 ||c[i]<0x30) //no number
			{
				ws.sn=0;
				return 1;
			}
			ws.sn*=10;
			ws.sn+=c[i];ws.sn-=0x30;
		}
	}
	else
	{//十六进制
		for(i=0;i<j;i++)
		{
			ch[i]=toupper(c[i]);
			if(ch[i]>=0x30 && ch[i]<=0x39)
			{
				ws.sn*=16;ws.sn+=ch[i];
				ws.sn-=0x30;
			}
			else
			{
				if(ch[i]>='A' && ch[i]<='F')
				{
					ws.sn*=16;ws.sn+=ch[i];
					ws.sn-=0x37;
				}
				else
				{
					ws.sn=0;return 1;
				}
			}
		}
	}
	/*if(k==ws.sn)//与上次的查询数据一样？提示下用户
	{
		msgbox("本次输入的查询数据与前次查询相同？？");
	}*/
	return 0;
}//}}}
//{{{ gpointer thd_fst(gpointer pt)
gpointer thd_fst(gpointer pt)
{
	gdouble gd;
	int i,j,k,l,m,fd;
	char ch[20],*c;
	struct KVAR_AM *k_am;
	union OFFSET *ksa;
	//struct KVAR_SAD *ksa;
	GtkTreeIter iter;
	GtkListStore *store;
	ws.thread_lock=1;//锁定
//	g_print("aaaaaaaa\n");
	k_am=(struct KVAR_AM *)ws.g_ch;
	ksa=(union OFFSET *)&(ws.g_ch[d_begin]);
	memset(ws.g_ch,0,buf_size);
	k_am->cmd=1;					//首次查询命令字
	k_am->pid=ws.pid;			//目标pid 
	k_am->snum=ws.sn;			//查询关键字
	fd=open(drv_name,O_RDWR);
	if(fd<0)
	{
		gdk_threads_enter();
		msgbox("目标模块连接失败");
		gdk_threads_leave();
		goto thd_01;
	}
	i=write(fd,ws.g_ch,buf_size);//发送命令
	if(i!=buf_size)
	{
		gdk_threads_enter();
		msgbox("首次查询指令发送失败");
		gdk_threads_leave();
		goto thd_01;
	}
	msleep();
	//开始进入轮寻等待
	//g_timeout_add(300,(GSourceFunc),(gpointer)fd); //1second=1000
	memset(ws.g_ch,0,buf_size);
	c=ws.g_addr[0];
	memset(c,0,adr_buf1);
	gdk_threads_enter();
	gtk_list_store_clear(ws.store[2]);//先清空列表
	gdk_threads_leave();m=0;
	while(1)
	{
		i=read(fd,ws.g_ch,buf_size);
		if(i!=buf_size)
		{
			msleep();
			msleep();
	//		g_print("aaa\n");
			continue;
		}
		if(k_am->sync!=1)//不可能的错误
		{
			msleep();
			msleep();
			memset(ws.g_ch,0,buf_size);
			continue;
		}
		if(m>=AREA_SIZE)//超出存储缓冲区400k的大小了
			goto n_001;
		ksa=(union OFFSET *)&(ws.g_ch[d_begin]);
		for(l=0;l<2000;l++)
		{
			if(ksa->ad==0 && k_am->end0==1)
				break;
			gdk_threads_enter();
			store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ws.list[2])));
			gtk_list_store_append(store,&iter);
//			gtk_list_store_set(store,&iter,0,ksa->seg,1,ksa->page,2,ksa->off,-1);
			memset(ch,0,sizeof(ch));
			snprintf(ch,sizeof(ch),"%X",ksa->ad);
			gtk_list_store_set(store,&iter,0,0,1,0,2,ch,-1);
			gdk_threads_leave();
			ksa++;
		}
		//进度条显示
		j=k_am->fin_pg;k=k_am->tol_pg;
		if(k==0)
		{g_print("0000\n");k=1;}
		i=j*100/k;gd=(float)j/k;
		memset(ch,0,sizeof(ch));
		snprintf(ch,sizeof(ch),"%d%%",i);
		gdk_threads_enter();
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(ws.prog),ch);
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ws.prog),gd);
		gdk_threads_leave();
		//保存结果
		memcpy((void*)c,(void*)&(ws.g_ch[d_begin]),dlen);
		m++;c+=dlen;
n_001:
		if(k_am->end0==1)//全部结束
		{
			gd=1;
			memset(ch,0,sizeof(ch));
			snprintf(ch,sizeof(ch),"100%%");
			k_am->sync=0;k_am->end0=1;
			i=write(fd,ws.g_ch,buf_size);
			gdk_threads_enter();
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(ws.prog),ch);
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ws.prog),gd);
			msgbox("首次查询结束");
			gdk_threads_leave();
			goto thd_01;
		}//还未结束，则向内核发送标志
		memset((void*)&(ws.g_ch[d_begin]),0,dlen);
		k_am->sync=0;
		i=write(fd,ws.g_ch,buf_size);
		if(i<0)
		{
			gdk_threads_enter();
			msgbox("向内核发送指令失败！");
			gdk_threads_leave();
			goto thd_01;
		}
		msleep();
	}
thd_01:
	close(fd);
	ws.dseg[0]=k_am->t_seg;
	ws.dseg[1]=k_am->d_seg;
	ws.dseg[2]=k_am->b_seg;
	ws.dseg[3]=k_am->s_seg;
	ws.seg_len[0]=k_am->t_len;
	ws.seg_len[1]=k_am->d_len;
	ws.thread_lock=0;
	msleep();
	return 0;	
}//}}}
//{{{ void msleep()
void msleep()
{
  tm.tv_sec=0;
  tm.tv_usec=300000;	//300毫秒
  select(0,NULL,NULL,NULL,&tm);
}//}}}
//{{{ void on_next_srh(GtkWidget *widget,gpointer gp)
void on_next_srh(GtkWidget *widget,gpointer gp)
{
    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter iter;
	char *p;
	if(ws.thread_lock==1)
	{
		msgbox("查询中....请稍后操作");
		return;
	}
	if(ws.pid<1)
	{
		msgbox("请先选择待查询的目标进程");
		return;
	}
	//查看treeview3中是否有记录？没有记录则表示还没有执行首次查找
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ws.list[2])));
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(ws.list[2]));
    if (gtk_tree_model_get_iter_first(model, &iter) == FALSE)
	{
		msgbox("请先进行\"首次查找\"!");
	 	return;
	}
    gtk_list_store_clear(store);//有记录则全部清空。
	p=(char*)gtk_entry_get_text(GTK_ENTRY(ws.entry[0]));
	if(check_input(p))
	{
		msgbox("输入的查询数据有误");
		return;
	}
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(ws.prog),"0%%");
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ws.prog),(gdouble)0);
//	g_thread_create(thd_next,NULL,FALSE,NULL);
	g_thread_new("thd_next",thd_next,NULL);
}//}}}
//{{{ gpointer thd_next(gpointer pt)
gpointer thd_next(gpointer pt)
{
	gdouble gd;
	int i,j,k,l,fd,count;
	char ch[200],*c,*p,*dst;
	struct KVAR_AM *k_am;
	union OFFSET *ksa;
	GtkTreeIter iter;
	GtkListStore *store;
	ws.thread_lock=1;
	k_am=(struct KVAR_AM *)ws.g_ch;
	ksa=(union OFFSET *)&(ws.g_ch[d_begin]);
	memset(ws.g_ch,0,buf_size);
	k_am->cmd=2;
	k_am->pid=ws.pid;
	k_am->snum=ws.sn;
	c=(char*)&(ws.g_ch[d_begin]);
	p=ws.g_addr[0];l=0;j=adr_buf1/dlen;
	k=adr_buf1%dlen;
	if(k>0)
		j++;
	for(i=0;i<j;i++)
	{
		if(p[0]==0 && p[1]==0 && p[2]==0 && p[3]==0)
			break;
		l++;p+=dlen;
	}//l保存了需要向内核传送地址的次数。
	if(l==0)
	{
		gdk_threads_enter();
		msgbox("已没有记录可供比较，查询退出");
		gdk_threads_leave();
		ws.thread_lock=0;
		return 0;
	}
	p=ws.g_addr[0];
	memset(ch,0,sizeof(ch));
	snprintf(ch,sizeof(ch),"%d\n",l);
	g_print(ch);
	dst=ws.g_addr[1];
	memset(dst,0,adr_buf2);
	memset(c,0,dlen);
	memcpy(c,p,dlen);
	p+=dlen;
	fd=open(drv_name,O_RDWR);
	if(fd<0)
	{
		gdk_threads_enter();
		msgbox("目标模块链接失败");
		gdk_threads_leave();
		goto nthd_03;
	}
	i=write(fd,ws.g_ch,buf_size);//send command
	if(i!=buf_size)
	{
		gdk_threads_enter();
		msgbox("首次查询指令发送失败");
		gdk_threads_leave();
		goto nthd_02;
	}
	msleep();k=0;count=0;
	while(1)
	{
		i=read(fd,ws.g_ch,buf_size);
		if(i!=buf_size)
		{
			msleep();
			msleep();
			continue;
		}
		if(k_am->sync!=1)
		{
			msleep();
			msleep();continue;
		}
		//{{{ 保存结果
		if(k_am->end1==0)//传出的结果。
		{
			if(k>=10)
			{
				k_am->sync=0;
				msleep();
				memset(c,0,dlen);
				j=write(fd,ws.g_ch,buf_size);
				if(k_am->end0==1)
					break;
				else
					continue;
			}
			ksa=(union OFFSET*)&(ws.g_ch[d_begin]);
			for(j=0;j<2000;j++)
			{
				if(ksa->ad==0 && k_am->end0==1)
				{
					/*memset(ws.g_ch,0,buf_size);
					j=write(fd,ws.g_ch,buf_size);
					msleep();
					goto nthd_02;*/
					break;
				}
				gdk_threads_enter();
				store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ws.list[2])));
				gtk_list_store_append(store,&iter);
//				gtk_list_store_set(store,&iter,0,ksa->seg,1,ksa->page,2,ksa->off,-1);
				memset(ch,0,sizeof(ch));
				snprintf(ch,sizeof(ch),"%X",ksa->ad);
				gtk_list_store_set(store,&iter,0,0,1,0,2,ch,-1);
				gdk_threads_leave();
				ksa++;
			}
			memset(ch,0,sizeof(ch));
			snprintf(ch,sizeof(ch),labin,k_am->b_seg);
			gtk_label_set_markup(GTK_LABEL(ws.label_nn),ch);
			//进度条
			k++;
			//保存结果
			memcpy((void*)dst,(void*)c,dlen);
			dst+=dlen;
			k_am->sync=0;k_am->end1=0;
			msleep();
			memset(c,0,dlen);
			j=write(fd,ws.g_ch,buf_size);
			if(k_am->end0==1)
				goto nthd_02;
		}//}}}
		//{{{ 传入地址
		else//传入地址
		{
			i=count*100/l;gd=(float)count/l;
			memset(ch,0,sizeof(ch));
			snprintf(ch,sizeof(ch),"%d%%",i);
			gdk_threads_enter();
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(ws.prog),ch);
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ws.prog),gd);
			gdk_threads_leave();
			memset(c,0,dlen);
			memcpy((void*)c,(void*)p,dlen);
			p+=dlen;count++;
			if(count>=l)
				k_am->end0=1;
			else
				k_am->end0=0;
			k_am->end1=1;
			k_am->sync=0;
			msleep();
			j=write(fd,ws.g_ch,buf_size);
			msleep();
		}//}}}
	}
nthd_02:
	close(fd);
nthd_03:
	memset((void*)ws.g_addr[0],0,adr_buf1);
	memcpy((void*)ws.g_addr[0],(void*)ws.g_addr[1],adr_buf2);
	ws.thread_lock=0;
	gd=(float)0;
	memset(ch,0,sizeof(ch));
	snprintf(ch,sizeof(ch),"0%%");
	gdk_threads_enter();
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(ws.prog),ch);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ws.prog),gd);
	msgbox("再次查找完成");
	gdk_threads_leave();
	return 0;
}//}}}
//{{{ void on_save(GtkWidget *widget,gpointer gp)
void on_save(GtkWidget *widget,gpointer gp)
{
	int i,j,k,fd;
	char fname[256],ch[2048];
    char *filename;
	GtkFileFilter* filter;
	GtkWidget *dialog;
	dialog=gtk_file_chooser_dialog_new(
			"open",
			NULL,
			GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,
			NULL
			);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name (filter, "All files");
    gtk_file_filter_add_pattern(filter,"*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog),filter);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name (filter, "Save files");
    //gtk_file_filter_add_pattern(filter,"*.[Tt][Xx][Tt]");
    gtk_file_filter_add_pattern(filter,"*.tws");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog),filter);

    gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), FALSE);
	//设置当前文件夹
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog),lock_file_dir);//"./"
    gtk_window_set_position(GTK_WINDOW(dialog),GTK_WIN_POS_CENTER_ON_PARENT);
	j=0;
    ///等待用户的动作
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        //msgbox(filename);
		i=strlen(filename);
		if(i>256)
		{
			msgbox("文件名太长！");
		}
		else
		{
			memset(fname,0,sizeof(fname));
			memcpy(fname,filename,i);
			j=1;
		}
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
	if(j!=1)
		return;
	fd=open(fname,O_RDWR|O_CREAT|O_TRUNC,0644);
	if(fd<=0)
	{
		msgbox("打开记录文件失败！");
		return;
	}
	/*该记录文件的格式为：前16个字节保存用于验证的代码段和数据段长度。第20字节开始保存若干save_lock结构的数据*/
	if(ws.seg_len[0]==0 || ws.seg_len[1]==0)
	{
		msgbox("保存错误，缺少相应段的长度!");
		close(fd);
		return;
	}
	memset(ch,0,sizeof(ch));
	memcpy((void*)&ch,(void*)&ws.seg_len[0],sizeof(int));
	memcpy((void*)&ch[8],(void*)&ws.seg_len[1],sizeof(int));
	j=0;k=20;
	for(i=0;i<max_res;i++)
	{
		if(sl[i].lk.offset.ad==0) //NULL
			continue;
		memcpy((void*)&ch[k],(void*)&sl[i],sizeof(sl[i]));
		k+=sizeof(sl[i]);
		j++;		
	}
	if(j==0)
	{
		msgbox("没有需要保存的记录");
		close(fd);
		return;
	}
	k=j*sizeof(sl[0])+20;
	i=write(fd,ch,k);
	if(i!=k)
		msgbox("写入错误");
	else
		msgbox("保存成功");
	close(fd);
}//}}}
//{{{ gpointer thd_thr(gpointer pt)
gpointer thd_thr(gpointer pt)
{
	int fd,i;
	ws.thread_lock=1;

	ws.thread_lock=0;
	return 0;
}//}}}
//{{{ void on_tree3_dblclk(GtkTreeView *treeview,GtkTreePath *path,GtkTreeViewColumn *col,gpointer userdata)
void on_tree3_dblclk(GtkTreeView *treeview,GtkTreePath *path,GtkTreeViewColumn *col,gpointer userdata)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GValue value;
	int i,j,k;
	char *ch;
//	char c[20];
	mem_u.ad=0;
	memset((void*)&value,0,sizeof(value));
	model=gtk_tree_view_get_model(treeview);
	if(gtk_tree_model_get_iter(model,&iter,path))
	{//idx和doff分别为全局变量，保存用户的选择。
/*		gtk_tree_model_get_value(model,&iter,0,&value);
		i=g_value_get_int(&value);//get pid
		g_value_unset(&value);
		gtk_tree_model_get_value(model,&iter,1,&value);
		j=g_value_get_int(&value);
		g_value_unset(&value); */
		gtk_tree_model_get_value(model,&iter,2,&value);
		ch=(char *)g_value_get_string(&value);
		memset(doff,0,sizeof(doff));
		snprintf(doff,sizeof(doff),"%s",ch);//g_value_get_string(&value));
		g_value_unset(&value);
		gtk_entry_set_text(GTK_ENTRY(ws.entry[2]),doff);
//		mem_u.seg=i;mem_u.page=j;mem_u.off=k;//内存检视使用.
	}
}//}}}
//{{{ void on_addlock(GtkWidget *widget,gpointer gp)
void on_addlock(GtkWidget *widget,gpointer gp)
{
	int len;
	char ch[40],*c;
	c=(char*)gtk_entry_get_text(GTK_ENTRY(ws.entry[2]));
	len=strlen(c);
	if(len>40)
	{
		msgbox("error");
		return;
	}
	memset(ch,0,sizeof(ch));
	memcpy(ch,c,len);
	check_lock(ch);
}//}}}
//{{{ void on_dellock(GtkWidget *widget,gpointer gp)
void on_dellock(GtkWidget *widget,gpointer gp)
{
	GtkTreeModel *model;
	GtkTreeIter	  iter;
	GtkTreeSelection *sel;
	gchar *str[2];
	gint  gin[2];
	char  ch[100];
	unsigned int ui;
	int i,j;
//	union OFFSET ksa;
//	ksa.ad=0;
	sel=gtk_tree_view_get_selection(GTK_TREE_VIEW(ws.list[1]));
	if(!gtk_tree_selection_get_selected(sel,&model,&iter))
	{
		msgbox("请先选择要删除的记录");
		return;
	}
	gtk_tree_model_get(model,&iter,0,&str[0],1,&str[1],2,&gin[0],3,&gin[1],-1);
	memset(ch,0,sizeof(ch));
	snprintf(ch,sizeof(ch),"%s",str[1]);
/*	gc=strchr(ch,'-');
	if(gc==NULL)
	{
		msgbox("地址格式错误 #01");
		return;
	}
	i=gc-ch;
	memset(c1,0,sizeof(c1));
	memcpy(c1,ch,i);
	ksa.seg=atoi(c1);
	gc++;
	g=strchr(gc,'-');
	if(g==NULL)
	{
		msgbox("地址格式错误 #02");
		return;
	}
	i=g-gc;
	memset(c1,0,sizeof(c1));
	memcpy(c1,gc,i);
	ksa.page=atoi(c1);
	g++;
	ksa.off=atoi(g);  */
	ui=strtoul(ch,NULL,16);
	for(i=0;i<max_res;i++)
	{
		if(sl[i].lk.offset.ad==ui)
		{
			memset((void*)&sl[i],0,sizeof(sl[i]));
			gtk_list_store_remove(GTK_LIST_STORE(model),&iter);
			break;
		}
	}
	g_free(str[0]);g_free(str[1]);
}//}}}
//{{{ int check_lock(char *c)
int check_lock(char *c)
{
	GtkTreeIter iter;
	GtkListStore *store;
	unsigned int ui;
	int i,j,k,l,len,addr,seg,page,offset;
	char c1[20],c2[100];
	char *gc,*g;
	union OFFSET ksa;
	ksa.ad=0;
	len=strlen(c);
//	if(len>40 || len<=0)
	if(len>8 || len<=0)
	{
		memset(c2,0,sizeof(c2));
		snprintf(c2,sizeof(c2),"impossible!");
		goto ckerr_01;
	}
//2016-10-29修改为新的地址表示方式
	g=c;
	memset(c2,0,sizeof(c2));
	for(i=0;i<len;i++)
	{*g=toupper(*g);g++;}
	g=c;
	for(i=0;i<len;i++)
	{
		if((*g)<'0' || (*g)>'F')
		{
			snprintf(c2,sizeof(c2),"format error~!");
			goto ckerr_01;
		}
		if(((*g)>'9') && ((*g)<'A'))
		{
			snprintf(c2,sizeof(c2),"format error~!");
			goto ckerr_01;
		}
		g++;
	}
	ui=strtoul(c,NULL,16);
	for(i=0;i<max_res;i++)
	{
		if(sl[i].lk.offset.ad == ui)
		{
			memset(c2,0,sizeof(c2));
			snprintf(c2,sizeof(c2),"你当前要锁定的地址早已加入锁定列表中 %s %x",c,ui);
			goto ckerr_01;
		}
	}
/*	gc=strchr(c,'-');
	if(gc==NULL)
	{
		memset(c2,0,sizeof(c2));
		snprintf(c2,sizeof(c2),"地址格式错误！ #01");
		goto ckerr_01;
	}
	i=gc-c;
	memset(c1,0,sizeof(c1));
	memcpy(c1,c,i);
	seg=atoi(c1);//取得段索引
	gc++;
	g=strchr(gc,'-');
	if(g==NULL)
	{
		memset(c2,0,sizeof(c2));
		snprintf(c2,sizeof(c2),"地址格式错误！ #02");
		goto ckerr_01;
	}
	i=g-gc;
	memset(c1,0,sizeof(c1));
	memcpy(c1,gc,i);
	page=atoi(c1);//取得页索引
	g++;
	memset(c1,0,sizeof(c1));
	memcpy(c1,g,strlen(g));
	offset=atoi(c1);//取得页内偏移
	ksa.seg=seg;ksa.page=page;ksa.off=offset;
	for(i=0;i<8;i++)
	{
		if(sl[i].lk.offset.ad==ksa.ad)
		{
			memset(c2,0,sizeof(c2));
			snprintf(c2,sizeof(c2),"你当前要锁定的地址早已加入到锁定列表中");
			goto ckerr_01;
		}
	}*/
	memset(c1,0,20);
	gc=(char*)gtk_entry_get_text(GTK_ENTRY(ws.entry[3]));
	i=strlen(gc);
	if(i>20)
	{
		memset(c2,0,sizeof(c2));
		snprintf(c2,sizeof(c2),"设定的最小值长度错误。");
		goto ckerr_01;
	}
	memcpy(c1,gc,i);
	j=atoi(c1);
	if(j<0)
	{
		memset(c2,0,sizeof(c2));
		snprintf(c2,sizeof(c2),"最小值输入错误。");
		goto ckerr_01;
	}
	memset(c1,0,20);
	gc=(char*)gtk_entry_get_text(GTK_ENTRY(ws.entry[4]));
	i=strlen(gc);
	if(i>20)
	{
		memset(c2,0,sizeof(c2));
		snprintf(c2,sizeof(c2),"设定的最大值长度错误。");
		goto ckerr_01;
	}
	memcpy(c1,gc,i);
	k=atoi(c1);
	if(k<0)
	{
		memset(c2,0,sizeof(c2));
		snprintf(c2,sizeof(c2),"最大值输入错误。");
		goto ckerr_01;
	}
	memset(c1,0,20);
	gc=(char*)gtk_entry_get_text(GTK_ENTRY(ws.entry[1]));
	i=strlen(gc);
	if(i>20)
	{
		memset(c2,0,sizeof(c2));
		snprintf(c2,sizeof(c2),"输入的备注长度错误。");
		goto ckerr_01;
	}
	memcpy(c1,gc,i);
	l=0;
	for(i=0;i<max_res;i++)
	{
		if(sl[i].ch[0]==0)//
		{
			l=1;
			memcpy(sl[i].ch,c1,strlen(c1));
			sl[i].lk.maxd=k;
			sl[i].lk.mind=j;
		//	sl[i].lk.offset.ad=ksa.ad;
			sl[i].lk.offset.ad=ui;
			//gc=ws.g_addr[0];
			//gc+=idx*sizeof(int);//定位到地址集中的目标地址的偏移
			//memcpy((void*)&(sl[i].add.ksa),(void*)gc,sizeof(struct KVAR_SAD));
			break;
		}
	}
	if(l!=1)
	{
		memset(c2,0,sizeof(c2));
		snprintf(c2,sizeof(c2),"锁定的目标地址最多16个");
		goto ckerr_01;
	}
	store = GTK_LIST_STORE(gtk_tree_view_get_model
			(GTK_TREE_VIEW(ws.list[1])));
	gtk_list_store_append(store,&iter);
	gtk_list_store_set(store,&iter,0,c1,1,c,2,j,3,k,-1); 
	return 0;
ckerr_01:
	msgbox(c2);
	return 1;	
}//}}}
//{{{ void on_lock(GtkWidget *widget,gpointer gp)
void on_lock(GtkWidget *widget,gpointer gp)
{
	int i,j,k,fd;
	char ch[40],*c,*lb;
	struct KVAR_AM *k_am;
	unsigned int u[2],v,z;
	unsigned short us[2];
	memset(ws.g_ch,0,buf_size);
	k_am=(struct KVAR_AM *)ws.g_ch;
	j=0;
	//这里必须要添加检测代码，因为使用了加载记录文件，要保证:pid已取得，段长度有效，有锁定记录。
	if(ws.pid<=0)
	{
		msgbox("请先指定待锁定的进程。");
		return;
	}
	if(ws.seg_len[0]==0 || ws.seg_len[1]==0)
	{
		msgbox("目标进程的段长度无效！");
		return;
	}
	for(i=0;i<max_res;i++)
	{
		if(sl[i].lk.offset.ad!=0)
		{j=1;break;}
	}
	if(j==0)
	{
		msgbox("没有要锁定的地址！");
		return;
	}
	j=0;
	lb=(char*)gtk_button_get_label(GTK_BUTTON(ws.bnt[8]));
	if(strncmp(lb,"锁    定",strlen(lb))==0)
	{
		if(ws.thread_lock==1)
			return;
		gtk_button_set_label(GTK_BUTTON(ws.bnt[8]),"取消锁定");
		ws.thread_lock=1;//虽然这里没有启用线程，但是为了与线程的形式保持一致，还要设置
		for(i=0;i<max_res;i++)
		{//新的锁定地址达到16个，而与内核通讯的缓冲区结构并没有改变，仍为8个锁定地址
//但是该结构设计的存放锁定地址的缓冲在结构头的尾部，后面为数据区，而锁定操作不需要向内核传递
//什么数据（新的方法已经不仅限于锁定操作，所有操作都再不需要这个数据区了）所以，可以放心的越界
//使用原来设计的数据区存放增加的8个地址			
			if(sl[i].lk.offset.ad!=0)
			{
				memcpy((void*)&(k_am->ladr[j]),(void*)&(sl[i].lk),sizeof(struct KVAR_LOCK));
				j++;
			}
		}
		k_am->sync=0;
		k_am->end0=0;
		k_am->cmd=3;
		k_am->pid=ws.pid;
		k_am->t_len=ws.seg_len[0];
		k_am->d_len=ws.seg_len[1];
		fd=open(drv_name,O_RDWR);
		if(fd<0)
		{
			msgbox("目标模块连接失败");
			return;
		}
		i=write(fd,ws.g_ch,buf_size);//发送命令
		close(fd);
		return;
	}
	else
	{
		gtk_button_set_label(GTK_BUTTON(ws.bnt[8]),"锁    定");
		k_am->sync=0;
		k_am->end0=1;
		k_am->cmd=3;
		fd=open(drv_name,O_RDWR);
		if(fd<0)
		{
			msgbox("目标模块连接失败");
			return;
		}
		i=write(fd,ws.g_ch,buf_size);//发送命令
		close(fd);
		ws.thread_lock=0;
		return;
	}
//锁定操作不用启动线程，直接发送命令即可。
	return;
err_a01: 
	msgbox("error");
	return;
}//}}}
//{{{ int tonum(char *c,unsigned int *ui)
int tonum(char *c,unsigned int *ui)
{//hex to dec convert;
	int i,j,k;
	unsigned u;
	j=strlen(c);
	if(j>10)
		return 1;
	if(c[0]!='0' || c[1]!='x')
		return 1;
	u=0;
	for(i=2;i<j;i++)
	{
		k=toupper(c[i]);
		if(k>=0x30 && k<=0x39)
		{
			u*=0x10;
			u+=k;u-=0x30;
		}
		else
		{
			if(k>='A' && k<='F')
			{
				u*=0x10;
				u+=k;u-=0x37;
			}
			else
				return 1;
		}
	}
	*ui=u;
	return 0;
}//}}}
//{{{ void on_load(GtkWidget *widget,gpointer gp)
void on_load(GtkWidget *widget,gpointer gp)
{
	int i,j,fd;
	char fname[256],ch[1024],*c;
	char *filename;
	GtkFileFilter* filter;
	GtkWidget *dialog;
    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter iter;
	dialog=gtk_file_chooser_dialog_new(
			"打开文件",
			NULL,
			GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,NULL
			);
	filter=gtk_file_filter_new();
	gtk_file_filter_set_name(filter,"All Files");
	gtk_file_filter_add_pattern(filter,"*");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filter);
	filter=gtk_file_filter_new();
	gtk_file_filter_set_name(filter,"Save Files");
	gtk_file_filter_add_pattern(filter,"*.[tT][wW][sS]");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filter);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog),FALSE);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),lock_file_dir);
	gtk_window_set_position(GTK_WINDOW(dialog),GTK_WIN_POS_CENTER_ON_PARENT);
	j=0;
	if(gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_ACCEPT)
	{
		filename=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		i=strlen(filename);
		if(i>256)
		{
			msgbox("文件名太长!");
		}
		else
		{
			memset(fname,0,sizeof(fname));
			memcpy(fname,filename,i);
			j=1;
		}
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
	if(j!=1)
		return;
	fd=open(fname,O_RDONLY);
	if(fd<=0)
	{
		msgbox("打开记录文件失败!");
		return;
	}
	ws.seg_len[0]=0;ws.seg_len[1]=0;
	memset(ch,0,sizeof(ch));
	i=read(fd,ch,sizeof(ch));
	if(i<=0)
	{
		msgbox("读取文件失败！");
		close(fd);
		return;
	}
	memcpy((void*)&ws.seg_len[0],(void*)&ch,sizeof(int));
	memcpy((void*)&ws.seg_len[1],(void*)&ch[8],sizeof(int));
	c=(char*)&ch[20];
	for(j=0;j<max_res;j++)
		memset((void*)&(sl[j]),0,sizeof(sl[0]));
	i-=20;
	j=i/sizeof(sl[0]);
	if(j<1 || j>max_res)
	{
		close(fd);
		msgbox("记录条数错误。");
		return;
	}
	for(i=0;i<j;i++)
	{
		memcpy((void*)&(sl[i]),c,sizeof(sl[0]));
		c+=sizeof(sl[0]);
	}
	close(fd);
//还要写入列表框：
	model=gtk_tree_view_get_model(GTK_TREE_VIEW(ws.list[1]));
	store=GTK_LIST_STORE(model);
	gtk_list_store_clear(store);
	for(i=0;i<max_res;i++)
	{
		if(sl[i].lk.offset.ad==0)
			continue;
		memset(ch,0,sizeof(ch));
		snprintf(ch,sizeof(ch),"%X",sl[i].lk.offset.ad);
		gtk_list_store_append(store,&iter);
		gtk_list_store_set(store,&iter,0,sl[i].ch,1,ch,2,sl[i].lk.mind,3,sl[i].lk.maxd,-1);
	}
}//}}}
//{{{ void on_calc(GtkWidget *widget,gpointer gp)
void on_calc(GtkWidget *widget,gpointer gp)
{
	int i,j,k;
	char c[40],ch[200],*p;
	p=(char*)gtk_entry_get_text(GTK_ENTRY(ws.entry[0]));
	if(check_input(p))
	{
		msgbox("输入的查询数据有误");
		return;
	}
	memset(ch,0,sizeof(ch));
	memset(c,0,sizeof(c));
	j=31;k=ws.sn;
	for(i=0;i<32;i++)
	{
		if(k & 1)
			c[j]='1';
		else
			c[j]='0';
		k=k>>1;j--;
	}
	//itoa(ws.sn,c,2);
	snprintf(ch,sizeof(ch),"十进制：%d\n十六进制：0x%x\n八进制：0%o\n二进制：%s",ws.sn,ws.sn,ws.sn,c);
	msgbox(ch);
}//}}}
//{{{ void on_tree2_dblclk(GtkTreeView *treeview,GtkTreePath *path,GtkTreeViewColumn *col,gpointer userdata)
void on_tree2_dblclk(GtkTreeView *treeview,GtkTreePath *path,GtkTreeViewColumn *col,gpointer userdata)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
	GValue value;
	int i,j;
	char ch[40],ch1[40];
	memset((void*)&value,0,sizeof(value));
	model=gtk_tree_view_get_model(treeview);
	if(gtk_tree_model_get_iter(model,&iter,path))
	{
		gtk_tree_model_get_value(model,&iter,0,&value);
		memset(ch,0,sizeof(ch));
		snprintf(ch,sizeof(ch),"%s",g_value_get_string(&value));
		gtk_entry_set_text(GTK_ENTRY(ws.entry[1]),ch);
		g_value_unset(&value);
		gtk_tree_model_get_value(model,&iter,1,&value);
		memset(ch,0,sizeof(ch));
		snprintf(ch,sizeof(ch),"%s",g_value_get_string(&value));
		gtk_entry_set_text(GTK_ENTRY(ws.entry[2]),ch);
		g_value_unset(&value);
		//保存信息到mem_u以备内存检索使用。
		mem_u.ad=0;
		for(j=0;j<max_res;j++)
		{
			if(sl[j].lk.offset.ad==0)
				continue;
			memset(ch1,0,sizeof(ch1));
			snprintf(ch1,sizeof(ch1),"%X",sl[j].lk.offset.ad);
			if(memcmp(ch,ch1,sizeof(ch))==0)
			{
				mem_u.ad=sl[j].lk.offset.ad;
				break;
			}
		}
		gtk_tree_model_get_value(model,&iter,2,&value);
		i=g_value_get_int(&value);
		memset(ch,0,sizeof(ch));
		snprintf(ch,sizeof(ch),"%d",i);
		g_value_unset(&value);
		gtk_entry_set_text(GTK_ENTRY(ws.entry[3]),ch);
		gtk_tree_model_get_value(model,&iter,3,&value);
		i=g_value_get_int(&value);
		memset(ch,0,sizeof(ch));
		snprintf(ch,sizeof(ch),"%d",i);
		g_value_unset(&value);
		gtk_entry_set_text(GTK_ENTRY(ws.entry[4]),ch);
	}
}//}}}
//{{{ void on_memsrh(GtkWidget *widget,*gpointer gp)
void on_memsrh(GtkWidget *widget,gpointer gp)
{
	int i;
	char *lb;
	if(ws.pid<=0)
	{
		msgbox("请先指定待锁定的进程");
		return;
	}
	if(ws.seg_len[0]==0 || ws.seg_len[1]==0)//检查有效性
	{
		msgbox("目标进程的段长度无效");
		return;
	}
	if(mem_u.ad==0)
	{
		msgbox("没有指定要锁定的页或段");
		return;
	}
	lb=(char*)gtk_button_get_label(GTK_BUTTON(widget));
	if(strncmp(lb,"开始检索",strlen(lb))==0)
	{
		if(ws.thread_lock==1 || ws.mend_srh==1)
			return;
		gtk_button_set_label(GTK_BUTTON(widget),"取消检索");
		ws.mend_srh=1;//允许启动次线程
//		ws.thread_lock=1;
//		g_thread_create(mthd_srh,NULL,FALSE,NULL);
		g_thread_new("mthd_srh",mthd_srh,NULL);
		return;
	}
	else
	{
		ws.mend_srh=0;//线程退出。
		for(i=0;i<100;i++)
		{//2016-10-30添加，这里只能等待线程的正常退出
			msleep();
			if(ws.mend_srh==1)
				break;
		}
		ws.mend_srh=0;
		gtk_button_set_label(GTK_BUTTON(widget),"开始检索");
		ws.thread_lock=0;
		return;
	}
	return;
}//}}}
//{{{ gpointer mthd_srh(gpointer gp)
gpointer mthd_srh(gpointer gp)
{
	unsigned char *p;
	int i,j,k,l,fd;
	struct KVAR_AM *k_am;
	ws.thread_lock=1;
	memset(ws.g_ch,0,buf_size);
	k_am=(struct KVAR_AM *)ws.g_ch;
	k_am->sync=0;
	k_am->end0=0;
	k_am->cmd=4;//4为内存检视的命令
	k_am->pid=ws.pid;
	k_am->t_len=ws.seg_len[0];
	k_am->d_len=ws.seg_len[1];
	k_am->snum=mem_u.ad;//在这里重复使用snum用来保存传入的地址
	fd=open(drv_name,O_RDWR);
	if(fd<0)
	{
		gdk_threads_enter();
		msgbox("目标模块连接失败");
		gtk_button_set_label(GTK_BUTTON(ws.mbnt[2]),"开始检索");
		gdk_threads_leave();
		ws.thread_lock=0;
		ws.mend_srh=0;
		return 0;
	}
	i=write(fd,ws.g_ch,buf_size);//发送命令
	j=0;
	while(1)
	{
		for(i=0;i<7;i++)
		{
			msleep();
			if(ws.mend_srh==0)
				goto m_002;
		}
		if(j==0)
		{
			p=(unsigned char *)ws.g_ch;
			mmp=(unsigned char*)&(ws.g_ch[d_begin]);
			mnp=(unsigned char*)&(ws.g_addr[0][d_begin]);
			j=1;
		}
		else
		{
			p=(unsigned char *)ws.g_addr[0];
			mmp=(unsigned char*)&(ws.g_addr[0][d_begin]);
			mnp=(unsigned char*)&(ws.g_ch[d_begin]);
			j=0;
		}
		memset(mmp,0,dlen);
		i=read(fd,p,buf_size);
		if(i!=buf_size)
		{
			gdk_threads_enter();
			msgbox("数据读取失败！");
			gtk_button_set_label(GTK_BUTTON(ws.mbnt[2]),"开始检索");
			gdk_threads_leave();
			ws.mend_srh=0;
			break;
		}
		//2014-2-26添加模糊查询
		if(ws.blur_srh>0)
		{
			/*if(ws.blur_srh==1)
			{
				memset(cmp,0,sizeof(cmp));
				memcpy(cmp,mmp,4096);
			}*/
			if(ws.blur_srh==2)//else
			{
				for(i=0;i<4096;i++)
				{
					if(ws.show_mode==1)//cheng less
					{
						if(pcc1[i]==1)
						{
							if(mmp[i]>=cmp[i])//search less
								pcc1[i]=0;
						}
						if(pcc2[i]==1)
						{
							l=mmp[i]+mmp[i+1]*0x100;
							k=cmp[i]+cmp[i+1]*0x100;
							if(l>=k)
								pcc2[i]=0;
						}
					}
					if(ws.show_mode==2)//change bigger
					{
						if(pcc1[i]==1)
						{
							if(mmp[i]<=cmp[i])//search less
								pcc1[i]=0;
						}
						if(pcc2[i]==1)
						{
							l=mmp[i]+mmp[i+1]*0x100;
							k=cmp[i]+cmp[i+1]*0x100;
							if(l<=k)
								pcc2[i]=0;
						}
					}
				}
			}
			memset(cmp,0,sizeof(cmp));
			memcpy(cmp,mmp,4096);
			ws.blur_srh=0;
		}
		gdk_threads_enter();
		gtk_widget_queue_draw(ws.darea);
		gdk_threads_leave();
	}
m_002:
	ws.mend_srh=1; //这是正常退出时需设置的  2016-10-30修改	
m_001:	
	memset(ws.g_ch,0,buf_size);
	k_am->sync=0;
	k_am->cmd=4;
	k_am->end0=1;
	i=write(fd,ws.g_ch,buf_size);//send message
	msleep();
	msleep();
	close(fd);
	ws.thread_lock=0;
	g_print("mem thread exit\n");
	return 0;
}//}}}
//{{{ void on_combo_changed(GtkWidget *widget,gpointer gp)
void on_combo_changed(GtkWidget *widget,gpointer gp)
{
	ws.show_mode=gtk_combo_box_get_active(GTK_COMBO_BOX(ws.combo));
}//}}}
//{{{ void on_blursrh(GtkWidget *widget,gpointer gp)
void on_blursrh(GtkWidget *widget,gpointer gp)
{
	int i,j;
	GtkListStore *store;
	GtkTreeModel *model;
	GtkTreeIter  iter;
	if(ws.blur_srh!=0 || ws.mend_srh==0)
		return;
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ws.mlist)));
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(ws.mlist));
    if (gtk_tree_model_get_iter_first(model, &iter) == FALSE)
	{//首次，初始化
		memset(pcc1,1,sizeof(pcc1));
		memset(pcc2,1,sizeof(pcc2));
		for(i=0;i<4096;i++)
		{
			gtk_list_store_append(store,&iter);
			gtk_list_store_set(store,&iter,0,"字节",1,i,-1);
			gtk_list_store_append(store,&iter);
			gtk_list_store_set(store,&iter,0,"字",1,i,-1);
		}
		ws.blur_srh=1;//首次获取
	 	return;
	}
	gtk_list_store_clear(store);//有记录则全部清空
	ws.blur_srh=2;//比较，并显示符合条件的地址。
	j=0;
	while(ws.blur_srh!=0)
	{//等待线程获取并比较完成。
		msleep();
		if(j>500)
			break;
		j++;
	}
	if(j>500)
	{
		g_print("error! waiting\n");
		ws.blur_srh=0;
		ws.mend_srh=0;
		for(j=0;j<20;j++)
			msleep();
		return;
	}
	//比较完成，显示地址
	for(i=0;i<4096;i++)
	{
		if(pcc1[i]==1)
		{
			gtk_list_store_append(store,&iter);
			gtk_list_store_set(store,&iter,0,"字节",1,i,-1);
		}
		if(pcc2[i]==1)
		{
			gtk_list_store_append(store,&iter);
			gtk_list_store_set(store,&iter,0,"字",1,i,-1);
		}
	}
}//}}}

//2016-11-4添加多重页面比较的界面显示
//{{{ void show_mmem(GtkWidget *widget,gpointer gp)
void show_mmem(GtkWidget *widget,gpointer gp)
{
	char ch[200];
	int i;
	gpointer gg;
	GtkListStore *store;
	GtkTreeModel *model;
	GtkTreeViewColumn *column;
	GtkCellRenderer *render;
	GtkWidget *dlg,*fixed;
	GtkWidget *entry,*label;
	dst_addr=0;
	gtk_status_icon_set_visible(ws.sicon,FALSE);
	dlg=gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dlg),"多重内存页比较");
	gtk_window_set_position(GTK_WINDOW(dlg),GTK_WIN_POS_CENTER);
	gtk_widget_set_size_request(dlg,mm_wid,mm_hei);
	gtk_window_set_resizable(GTK_WINDOW(dlg),FALSE);
	fixed=gtk_fixed_new();
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dlg)->vbox),fixed,FALSE,FALSE,0);
	gtk_container_set_border_width(GTK_CONTAINER(dlg),2);
	label=gtk_label_new("");
	memset(ch,0,sizeof(ch));
	snprintf(ch,sizeof(ch),labi,"内存基地址：");
	gtk_label_set_markup(GTK_LABEL(label),ch);
	gtk_widget_set_size_request(label,mmlab1_w,mmlab1_h);
	gtk_fixed_put(GTK_FIXED(fixed),label,mmlab1_x,mmlab1_y);
	entry=gtk_entry_new();
	gtk_widget_set_size_request(entry,mmed1_w,mmed1_h);
	memset(ch,0,sizeof(ch));
	if(mem_u.ad!=0)
		snprintf(ch,sizeof(ch),"%X",mem_u.ad);
	else
		snprintf(ch,sizeof(ch),"00000000");
	gtk_entry_set_text(GTK_ENTRY(entry),ch);
	gtk_fixed_put(GTK_FIXED(fixed),entry,mmed1_x,mmed1_y);
	ws.mmbnt[0]=gtk_button_new_with_label("开  始");
	gtk_widget_set_size_request(ws.mmbnt[0],mmbnt_w,mmbnt_h);
	gtk_fixed_put(GTK_FIXED(fixed),ws.mmbnt[0],mmbnt_x,mmbnt_y);
	ws.mmbnt[1]=gtk_button_new_with_label("减少检索");
	gtk_widget_set_size_request(ws.mmbnt[1],mmbnt1_w,mmbnt1_h);
	gtk_fixed_put(GTK_FIXED(fixed),ws.mmbnt[1],mmbnt1_x,mmbnt1_y);
	ws.mmbnt[2]=gtk_button_new_with_label("增加检索");
	gtk_widget_set_size_request(ws.mmbnt[2],mmbnt2_w,mmbnt2_h);
	gtk_fixed_put(GTK_FIXED(fixed),ws.mmbnt[2],mmbnt2_x,mmbnt2_y);
	ws.mmbnt[3]=gtk_button_new_with_label("显示");
	gtk_widget_set_size_request(ws.mmbnt[3],mmbnt3_w,mmbnt3_h);
	gtk_fixed_put(GTK_FIXED(fixed),ws.mmbnt[3],mmbnt3_x,mmbnt3_y);
	ws.label_mm=gtk_label_new("");
	memset(ch,0,sizeof(ch));
	snprintf(ch,sizeof(ch),labi,"等待的操作：开始");
	gtk_label_set_markup(GTK_LABEL(ws.label_mm),ch);
	gtk_widget_set_size_request(ws.label_mm,mmlab2_w,mmlab2_h);
	gtk_fixed_put(GTK_FIXED(fixed),ws.label_mm,mmlab2_x,mmlab2_y);
	store=gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_STRING);
	model=GTK_TREE_MODEL(store);
	ws.mmlist=gtk_tree_view_new_with_model(model);
	render=gtk_cell_renderer_text_new();
	column=gtk_tree_view_column_new_with_attributes("  addrrss  ",render,"text",0,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(ws.mmlist),column);
	render=gtk_cell_renderer_text_new();
	column=gtk_tree_view_column_new_with_attributes("values",render,"text",1,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(ws.mmlist),column);
	gtk_widget_set_size_request(ws.mmlist,mmtre_w,mmtre_h);
	gtk_fixed_put(GTK_FIXED(fixed),ws.mmlist,mmtre_x,mmtre_y);
	g_signal_connect(G_OBJECT(ws.mmbnt[0]),"clicked",G_CALLBACK(on_mmbeg),NULL);
	gg=0;
	g_signal_connect(G_OBJECT(ws.mmbnt[1]),"clicked",G_CALLBACK(on_mmdel),gg);
	gg=(gpointer)1;
	g_signal_connect(G_OBJECT(ws.mmbnt[2]),"clicked",G_CALLBACK(on_mmdel),gg);
	g_signal_connect(G_OBJECT(ws.mmbnt[3]),"clicked",G_CALLBACK(on_mmshow),NULL);
	gtk_widget_show_all(dlg);
	gtk_dialog_run(GTK_DIALOG(dlg));
	if(ws.mend_srh!=0)
	{
		ws.mend_srh=0;//线程退出。
		msleep();
		msgbox("closing...");
	}
	gtk_widget_destroy(dlg);
	gtk_status_icon_set_visible(ws.sicon,TRUE);
}//}}}
//{{{ void on_mmbeg(GtkWidget *widget,gpointer gp)
void on_mmbeg(GtkWidget *widget,gpointer gp)
{
	if(ws.pid<=0)
	{
		msgbox("请先指定待锁定的进程");
		return;
	}
	if(ws.seg_len[0]==0 || ws.seg_len[1]==0)
	{
		msgbox("目标进程的段长度无效");
		return;
	}
	if(mem_u.ad<(0x848000+20*4096))
	{
		msgbox("没有指定要锁定的地址");
		return;
	}
	if(ws.thread_lock==1)
	{
		msgbox("当前有正在锁定的线程");
		return;
	}
	if(ws.mend_srh==1) //该标志是内存检索界面使用的，在此借用
	{//防止意外
		msgbox("意外的错误");
		return;
	}
	memset(ws.g_addr[1],0,adr_buf2); //160k的内存数据缓冲
	//memset(ws.g_addr[2],0,adr_buf2); //160k的内存数据缓冲
	ws.mend_srh=1;//允许启动线程
	g_thread_new("mmthd_fst",mmthd_fst,NULL);
	return;
}//}}}
//{{{void on_mmdel(GtkWidget *widget,gpointer gp)
void on_mmdel(GtkWidget *widget,gpointer gp)
{
	if(ws.pid<=0)
	{
		msgbox("请先指定待锁定的进程");
		return;
	}
	if(ws.seg_len[0]==0 || ws.seg_len[1]==0)
	{
		msgbox("目标进程的段长度无效");
		return;
	}
	if(mem_u.ad<(0x848000+20*4096))
	{
		msgbox("没有指定要锁定的地址");
		return;
	}
	if(ws.thread_lock==1)
	{
		msgbox("当前有正在锁定的线程");
		return;
	}
	if(ws.mend_srh==1) //该标志是内存检索界面使用的，在此借用
	{//防止意外
		msgbox("意外的错误");
		return;
	};
	if((pn <=0) || (dst_addr <= 0))
	{
		msgbox("请先执行开始");
		return;
	}
	ws.mend_srh=1;//允许启动线程
	g_thread_new("mmthd_del",mmthd_del,gp);
	return;
}//}}}
//{{{void on_mmshow(GtkWidget *widget,gpointer gp)
void on_mmshow(GtkWidget *widget,gpointer gp)
{
	int i,j,k,*ii;
	char ch[20],cc[20];
	unsigned int u;
	GtkListStore *store;
	GtkTreeModel *model;
	GtkTreeIter  iter;
	if((ws.mend_srh == 1) || (ws.thread_lock == 1))
		return;
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(ws.mmlist)));
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(ws.mmlist));
	gtk_list_store_clear(store);//有记录则全部清空
	ii=(int *)ws.g_addr[0];
	k=0;
	for(i=0;i<(pn*4096-1);i++)
	{
		j=*ii;
		if(j != 0)
		{
			u=dst_addr+(unsigned int)j;
			memset(cc,0,sizeof(cc));
			snprintf(cc,sizeof(cc),"%X",u);
			memset(ch,0,sizeof(ch));
			j+=192;
			snprintf(ch,sizeof(ch),"%02X %02X %02X %02X",(unsigned char)ws.g_addr[1][j],\
					(unsigned char)ws.g_addr[1][j+1],(unsigned char)ws.g_addr[1][j+2],(unsigned char)ws.g_addr[1][j+3]);
			gtk_list_store_append(store,&iter);
			gtk_list_store_set(store,&iter,0,cc,1,ch,-1);
			k++;
		}
		ii++;j=*ii;
		if(j != 0)
		{
			u=dst_addr+(unsigned int)j;
			memset(cc,0,sizeof(cc));
			snprintf(cc,sizeof(cc),"%X",u);
			memset(ch,0,sizeof(ch));
			j+=192;
			snprintf(ch,sizeof(ch),"%02X %02X %02X %02X",(unsigned char)ws.g_addr[1][j],\
					(unsigned char)ws.g_addr[1][j+1],(unsigned char)ws.g_addr[1][j+2],(unsigned char)ws.g_addr[1][j+3]);
			gtk_list_store_append(store,&iter);
			gtk_list_store_set(store,&iter,0,cc,1,ch,-1);
			k++;
		}
		ii++;
		if(k>40)
			break;
	}
	return;
}//}}}
//{{{gpointer mmthd_fst(gpointer gp)
gpointer mmthd_fst(gpointer gp)
{
	unsigned int ui;
	char ch[200];
	int fd,i,*ii;
	struct KVAR_AM	*k_am;
	ws.thread_lock=1;
	memset(ws.g_ch,0,buf_size);
	k_am=(struct KVAR_AM *)ws.g_ch;
	k_am->sync=0;
	k_am->cmd=5;
	k_am->pid=ws.pid;
	k_am->t_len=ws.seg_len[0];
	k_am->d_len=ws.seg_len[1];
	k_am->snum=mem_u.ad-4096*20;//指定地址的前后各20页
	k_am->fin_pg=0;//该值为传出时指定的起始地址
	k_am->tol_pg=0;//该值为传出时指定总的页数
	pn=0;//该变量为内存检视页面使用，在此借用。
	dst_addr=0;
	fd=open(drv_name,O_RDWR);
	if(fd<0)
	{
		memset(ch,0,sizeof(ch));
		snprintf(ch,sizeof(ch),"目标模块连接失败");
		goto mm_err01;
	}
	i=write(fd,ws.g_ch,buf_size);//发送命令
	if(i!=(int)buf_size)
	{
		memset(ch,0,sizeof(ch));
		snprintf(ch,sizeof(ch),"传入数据错误");
		goto mm_err02;
	}
	msleep();
	while(1)
	{
		i=read(fd,ws.g_addr[1],adr_buf2);//接收缓冲区为g_addr[1]
		if(i<=0)
		{
			memset(ch,0,sizeof(ch));
			snprintf(ch,sizeof(ch),"接收数据错误");
			goto mm_err02;
		}
		if(ws.g_addr[1][0]!=1)
		{
			msleep();
			memset(ws.g_addr[1],0,adr_buf2);
		}
		else
			break;
	}
	close(fd);
	k_am=(struct KVAR_AM *)ws.g_addr[1];
	dst_addr=k_am->fin_pg; //取得真正的内存起始地址
	pn=(int)(k_am->tol_pg);//取得真正获取的页数，默认为40
	//这里还有重要的一步，将地址集缓冲初始化
	memset(ws.g_addr[0],0,adr_buf1); //用于存放匹配的地址
	ii=(int *)ws.g_addr[0];
	for(i=0;i<(4096*pn-1);i++)//这里要有单字节和双字节两个地址集
	{
		*ii=i;ii++;
		*ii=i;ii++;
	}
	ws.thread_lock=0;
	ws.mend_srh=0;
	gdk_threads_enter();
	memset(ch,0,sizeof(ch));
	snprintf(ch,sizeof(ch),labm,"锁定开始,期待进行增加、减少检索...",pn);
	gtk_label_set_markup(GTK_LABEL(ws.label_mm),ch);
	gdk_threads_leave();
	return 0;
mm_err02:
	close(fd);	
mm_err01:
	gdk_threads_enter();
	msgbox(ch);
	gdk_threads_leave();
	ws.thread_lock=0;
	ws.mend_srh=0;
	return 0;
}//}}}
//{{{gpointer mmthd_del(gpointer gp)
gpointer mmthd_del(gpointer gp)
{
	unsigned int ui;
	char ch[200],cc[40];
	unsigned char *p1,*p2;
	int fd,i,j,k,m,n;
	int *ii;
	struct KVAR_AM *k_am;
	ws.thread_lock=1;
	ws.mend_srh=1;
	if((dst_addr <= 0) || (pn <= 0))
	{
		memset(ch,0,sizeof(ch));
		snprintf(ch,sizeof(ch),"没有初始数据");
		goto mmd_err01;
	}
	memset(ws.g_addr[2],0,adr_buf2);
	memcpy(ws.g_addr[2],ws.g_addr[1],adr_buf2);//移动上次取得的数据
	memset(ws.g_addr[1],0,adr_buf2);
	memset(ws.g_ch,0,buf_size);
	k_am=(struct KVAR_AM *)ws.g_ch;
	k_am->sync=0;
	k_am->cmd=5;
	k_am->pid=ws.pid;
	k_am->t_len=ws.seg_len[0];
	k_am->d_len=ws.seg_len[1];
	k_am->snum=mem_u.ad-4096*20;//dst_addr;
	k_am->fin_pg=0;k_am->tol_pg=0;
	fd=open(drv_name,O_RDWR);
	if(fd<0)
	{
		memset(ch,0,sizeof(ch));
		snprintf(ch,sizeof(ch),"目标模块连接失败");
		goto mmd_err01;
	}
	i=write(fd,ws.g_ch,buf_size);//发送命令
	if(i!=(int)buf_size)
	{
		memset(ch,0,sizeof(ch));
		snprintf(ch,sizeof(ch),"传入数据错误");
		goto mmd_err02;
	}
	msleep();
	while(1)
	{
		i=read(fd,ws.g_addr[1],adr_buf2);//接收缓冲区为g_addr[1]
		if(i<=0)
		{
			memset(ch,0,sizeof(ch));
			snprintf(ch,sizeof(ch),"接收数据错误");
			goto mmd_err02;
		}
		if(ws.g_addr[1][0]!=1)
		{
			msleep();
			memset(ws.g_addr[1],0,adr_buf2);
			msleep();
		}
		else
			break;
	}
	close(fd);
	k_am=(struct KVAR_AM *)ws.g_addr[1];
	if((k_am->fin_pg != dst_addr) || ((int)(k_am->tol_pg) != pn))
	{
		memset(ch,0,sizeof(ch));
		snprintf(ch,sizeof(ch),"地址不匹配错误");
		goto mmd_err01;
	}//开始按地址集中的地址进行比较
	ii=(int*)ws.g_addr[0];
	for(i=0,j=0;i<(pn*4096-1);i++)
	{
		k=*ii;
		if(k!=0)
		{
			k+=d_begin;
			if(gp == 0)
			{
				if(ws.g_addr[1][k]<ws.g_addr[2][k])
					j++;
				else
					*ii=0; //不符合
			}
			else
			{
				if(ws.g_addr[1][k]>ws.g_addr[2][k])
					j++;
				else
					*ii=0; //不符合
			}
		}
		ii++;
		k=*ii;
		if(k!=0)
		{
			k+=d_begin;
			m=ws.g_addr[1][k]+ws.g_addr[1][k+1]*0x100;
			n=ws.g_addr[2][k]+ws.g_addr[2][k+1]*0x100;
			if(gp == 0)
			{
				if(m<n)
					j++;
				else
					*ii=0;
			}
			else
			{
				if(m>n)
					j++;
				else
					*ii=0;
			}
		}
		ii++;
	}
	ws.thread_lock=0;
	ws.mend_srh=0;
	gdk_threads_enter();
	memset(ch,0,sizeof(ch));
	ii=(int*)&ws.g_addr[1][333];
	memset(ch,0,sizeof(ch));
	if(gp == 0)
		snprintf(cc,sizeof(cc),"正在进行减少检索，符合的记录数：");
	else
		snprintf(cc,sizeof(cc),"正在进行增加检索，符合的记录数：");
	snprintf(ch,sizeof(ch),labm,cc,j);
	gtk_label_set_markup(GTK_LABEL(ws.label_mm),ch);
	gdk_threads_leave();
	return 0;
mmd_err02:
	close(fd);
mmd_err01:
	gdk_threads_enter();
	msgbox(ch);
	gdk_threads_leave();
	ws.thread_lock=0;
	ws.mend_srh=0;
	dst_addr=0;pn=0;
	return 0;
}//}}}

