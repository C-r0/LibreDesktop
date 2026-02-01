#include <gtk/gtk.h>
#include <cairo.h>
#include <time.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include <libgen.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>

static cairo_surface_t *wallpaper = NULL;
static gboolean show_welcome = TRUE;

static gboolean hide_welcome(gpointer data) {
    show_welcome = FALSE;
    GtkWidget *widget = GTK_WIDGET(data);
    gtk_widget_queue_draw(widget);
    return FALSE;
}

static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data) {
    GtkAllocation a;
    gtk_widget_get_allocation(widget, &a);

    if (wallpaper) {
        double iw = cairo_image_surface_get_width(wallpaper);
        double ih = cairo_image_surface_get_height(wallpaper);
        double sx = (double)a.width / iw;
        double sy = (double)a.height / ih;
        double scale = sx > sy ? sx : sy;

        cairo_save(cr);
        cairo_translate(cr, (a.width - iw * scale)/2, (a.height - ih*scale)/2);
        cairo_scale(cr, scale, scale);
        cairo_set_source_surface(cr, wallpaper, 0, 0);
        cairo_paint(cr);
        cairo_restore(cr);
    }

    if(show_welcome) {
        cairo_set_font_size(cr, 50);
        cairo_set_source_rgba(cr,1,1,1,0.9);
        cairo_select_font_face(cr,"Sans",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);

        const char *user = getenv("USER");
        char welcome[128];
        snprintf(welcome,sizeof(welcome),"Bem Vindo, %s!",user);

        cairo_text_extents_t ext;
        cairo_text_extents(cr,welcome,&ext);
        cairo_move_to(cr,(a.width-ext.width)/2,(a.height-ext.height)/2);
        cairo_show_text(cr,welcome);
    } else {
        time_t t=time(NULL);
        struct tm *tm=localtime(&t);
        char buf[32],bufdate[64];
        strftime(buf,sizeof(buf),"%H:%M:%S",tm);
        strftime(bufdate,sizeof(bufdate),"%A, %d/%m/%Y",tm);

        cairo_set_source_rgba(cr,1,1,1,0.9);
        cairo_select_font_face(cr,"Sans",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);

        cairo_set_font_size(cr,72);
        cairo_text_extents_t ext;
        cairo_text_extents(cr,buf,&ext);
        cairo_move_to(cr,(a.width-ext.width)/2,(a.height-ext.height)/2-300);
        cairo_show_text(cr,buf);

        cairo_set_font_size(cr,28);
        cairo_text_extents_t ext_date;
        cairo_text_extents(cr,bufdate,&ext_date);
        cairo_move_to(cr,(a.width-ext_date.width)/2,(a.height-ext.height)/2-250);
        cairo_show_text(cr,bufdate);
    }
    return FALSE;
}

static gboolean tick(gpointer widget) {
    gtk_widget_queue_draw(GTK_WIDGET(widget));
    return TRUE;
}

/* Configura janela como desktop real */
static void set_as_desktop(GtkWidget *win) {
    GdkWindow *gdk_win = gtk_widget_get_window(win);
    Display *dpy = gdk_x11_display_get_xdisplay(gdk_window_get_display(gdk_win));
    Window xwin = gdk_x11_window_get_xid(gdk_win);

    Atom wm_type = XInternAtom(dpy,"_NET_WM_WINDOW_TYPE",False);
    Atom desktop = XInternAtom(dpy,"_NET_WM_WINDOW_TYPE_DESKTOP",False);
    XChangeProperty(dpy,xwin,wm_type,XA_ATOM,32,PropModeReplace,(unsigned char *)&desktop,1);

    Atom wm_state = XInternAtom(dpy,"_NET_WM_STATE",False);
    Atom below = XInternAtom(dpy,"_NET_WM_STATE_BELOW",False);
    Atom sticky = XInternAtom(dpy,"_NET_WM_STATE_STICKY",False);
    Atom skip_taskbar = XInternAtom(dpy,"_NET_WM_STATE_SKIP_TASKBAR",False);
    Atom skip_pager = XInternAtom(dpy,"_NET_WM_STATE_SKIP_PAGER",False);

    Atom states[] = { below, sticky, skip_taskbar, skip_pager };
    XChangeProperty(dpy,xwin,wm_state,XA_ATOM,32,PropModeReplace,(unsigned char *)states,4);
    XFlush(dpy);
}

int main(int argc,char **argv){
    gtk_init(&argc,&argv);

    GtkWidget *win=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_decorated(GTK_WINDOW(win),FALSE);

    GdkScreen *screen=gdk_screen_get_default();
    gtk_window_set_default_size(GTK_WINDOW(win),gdk_screen_get_width(screen),gdk_screen_get_height(screen));

    GtkWidget *area=gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(win),area);

    g_signal_connect(area,"draw",G_CALLBACK(draw_cb),NULL);
    g_timeout_add_seconds(1,tick,area);
    g_signal_connect(win,"destroy",G_CALLBACK(gtk_main_quit),NULL);

    /* Carrega wallpaper da pasta do executável */
    char exe_path[PATH_MAX];
    ssize_t len=readlink("/proc/self/exe",exe_path,sizeof(exe_path)-1);
    if(len==-1){perror("readlink");return 1;}
    exe_path[len]='\0';
    char *dir=dirname(exe_path);
    char wallpaper_path[PATH_MAX];
    snprintf(wallpaper_path,sizeof(wallpaper_path),"%s/wallpaper.png",dir);
    wallpaper=cairo_image_surface_create_from_png(wallpaper_path);

    gtk_widget_realize(win);
    set_as_desktop(win);  // <- janela agora fica atrás do XFCE, sem override_redirect

    gtk_widget_show_all(win);

    g_timeout_add_seconds(2,hide_welcome,area);

    gtk_main();
    cairo_surface_destroy(wallpaper);
    return 0;
}
