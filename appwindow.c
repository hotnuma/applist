#include "config.h"
#include "appwindow.h"
#include "preferences.h"

#include <gtk/gtk.h>
#include <gio/gdesktopappinfo.h>
#include <strings.h>

static void _window_finalize(GObject *object);

static void _window_create_view(AppWindow *window);
static void _window_store_load(AppWindow *window);

static void _treeview_row_activated(GtkTreeView *tree_view, GtkTreePath *path,
                                    GtkTreeViewColumn *column, AppWindow *window);

static bool _window_append_line(AppWindow *window, GAppInfo *info);

static gint _utf8_cmp(gconstpointer a, gconstpointer b);
gboolean _appinfo_show(GAppInfo *info);

static GdkPixbuf* _pixbuf_from_gicon(GtkWidget *widget, GIcon *gicon,
                                     const gchar *id);
static GdkPixbuf* _pixbuf_get_default(GtkWidget *widget, const gchar *id);

static gboolean _window_on_delete(GtkWidget *widget, GdkEvent *event, gpointer data);

enum
{
    COL_INFO = 0,
    COL_ICON,
    COL_TITLE,
    COL_VISIBLE,
    COL_FILE,
    NUM_COLS
};

struct _AppWindow
{
    GtkWindow __parent__;

    GtkListStore *store;
};

G_DEFINE_TYPE(AppWindow, window, GTK_TYPE_WINDOW)

static void window_class_init(AppWindowClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    gobject_class->finalize = _window_finalize;
}

static void _window_finalize(GObject *object)
{
    AppWindow *window = APPWINDOW(object);

    g_object_unref(window->store);

    G_OBJECT_CLASS(window_parent_class)->finalize(object);
}

static gboolean _window_on_delete(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    (void) widget;
    (void) event;
    (void) data;

    GtkWindow *window = GTK_WINDOW(widget);

    if (gtk_widget_get_visible(GTK_WIDGET(widget)))
    {
        GdkWindowState state = gdk_window_get_state(
                                            gtk_widget_get_window(widget));

        Preferences *prefs = get_preferences();
        prefs->window_maximized = ((state & (GDK_WINDOW_STATE_MAXIMIZED
                                             | GDK_WINDOW_STATE_FULLSCREEN))
                                    != 0);

        if (!prefs->window_maximized)
        {
            gtk_window_get_size(window,
                                &prefs->window_width,
                                &prefs->window_height);
        }

        prefs_write();
    }

    gtk_main_quit();

    return false;
}

static void window_init(AppWindow *window)
{
    Preferences *prefs = prefs_file_read();

    gtk_window_set_default_size(GTK_WINDOW(window),
                                prefs->window_width,
                                prefs->window_height);

    if (G_UNLIKELY(prefs->window_maximized))
        gtk_window_maximize(GTK_WINDOW(window));

    gtk_window_set_title(GTK_WINDOW(window), "AppInfo List");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    g_signal_connect(window, "delete-event",
                     G_CALLBACK(_window_on_delete), NULL);

    _window_create_view(window);
    _window_store_load(window);
}

static void _window_create_view(AppWindow *window)
{
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(window), scroll);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
                                        GTK_SHADOW_IN);

    GtkListStore *store = gtk_list_store_new(NUM_COLS,
                                             G_TYPE_APP_INFO,
                                             GDK_TYPE_PIXBUF,
                                             G_TYPE_STRING,
                                             G_TYPE_BOOLEAN,
                                             G_TYPE_STRING);

    GtkWidget *treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    window->store = store; // leak

    GtkTreeViewColumn *col;
    GtkCellRenderer *renderer;

    // Title

    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Title");

    renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(col, renderer, FALSE);
    gtk_tree_view_column_set_attributes(col, renderer,
                                        "pixbuf", COL_ICON,
                                        NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_set_attributes(col, renderer,
                                        "text", COL_TITLE,
                                        NULL);

    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);

    // Visible
    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "Visible");

    renderer = gtk_cell_renderer_toggle_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_set_attributes(col, renderer,
                                        "active", COL_VISIBLE,
                                        NULL);

    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);

    // File
    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "File");

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_set_attributes(col, renderer,
                                        "text", COL_FILE,
                                        NULL);

    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);

    gtk_container_add(GTK_CONTAINER(scroll), treeview);

    g_signal_connect(G_OBJECT(treeview), "row-activated",
                     G_CALLBACK(_treeview_row_activated), window);

}

static void _window_store_load(AppWindow *window)
{
    //Preferences *prefs = get_preferences();

    GList *all = g_app_info_get_all();
    all = g_list_sort(all, _utf8_cmp);

    for (GList *lp = all; lp; lp = lp->next)
    {
        GAppInfo *info = G_APP_INFO(lp->data);

        //if (prefs->show_all == false && !_appinfo_show(info))
        //    continue;

        _window_append_line(window, info);
    }

    g_list_free_full(all, g_object_unref);
}

static gint _utf8_cmp(gconstpointer a, gconstpointer b)
{
    return g_utf8_collate(g_app_info_get_name(G_APP_INFO(a)),
                          g_app_info_get_name(G_APP_INFO(b)));
}

gboolean _appinfo_show(GAppInfo *info)
{
    g_return_val_if_fail(G_IS_DESKTOP_APP_INFO(info), FALSE);

    GDesktopAppInfo *deskinfo = G_DESKTOP_APP_INFO(info);

    if (g_desktop_app_info_get_nodisplay(deskinfo))
        return false;

    return g_desktop_app_info_get_show_in(deskinfo, NULL);
}

static bool _window_append_line(AppWindow *window, GAppInfo *info)
{
    GIcon *gicon = g_app_info_get_icon(info);

    c_autounref GdkPixbuf *pix = _pixbuf_from_gicon(GTK_WIDGET(window),
                                                    gicon,
                                                    g_app_info_get_id(info));

    GtkTreeIter iter;
    gtk_list_store_append(window->store, &iter);
    gtk_list_store_set(window->store, &iter,
                       COL_INFO, info,
                       COL_ICON, pix,
                       COL_TITLE, g_app_info_get_name(info),
                       COL_VISIBLE, _appinfo_show(info),
                       COL_FILE, g_app_info_get_id(info),
                       -1);

    return true;
}

static GdkPixbuf* _pixbuf_from_gicon(GtkWidget *widget, GIcon *gicon, const gchar *id)
{
    if (!gicon)
    {
        //g_print("_pixbuf_from_gicon : %s : gicon = null\n", id);

        return _pixbuf_get_default(widget, id);
    }

    GtkIconTheme *icon_theme =
        gtk_icon_theme_get_for_screen(gtk_widget_get_screen(widget));

    gint scale_factor = gtk_widget_get_scale_factor(widget);
    gint requested_icon_size = 24 * scale_factor;

    c_autounref GtkIconInfo *icon_info =
        gtk_icon_theme_lookup_by_gicon(icon_theme,
                                       gicon,
                                       requested_icon_size,
                                       GTK_ICON_LOOKUP_USE_BUILTIN
                                       | GTK_ICON_LOOKUP_FORCE_SIZE);

    if (G_UNLIKELY(icon_info == NULL))
    {
        //g_print("_pixbuf_from_gicon : %s : icon_info = null\n", id);

        return _pixbuf_get_default(widget, id);
    }

    return gtk_icon_info_load_icon(icon_info, NULL);
}

static GdkPixbuf* _pixbuf_get_default(GtkWidget *widget, const gchar *id)
{
    GtkIconTheme *icon_theme =
        gtk_icon_theme_get_for_screen(gtk_widget_get_screen(widget));

    gint scale_factor = gtk_widget_get_scale_factor(widget);
    gint requested_icon_size = 24 * scale_factor;

    c_autounref GtkIconInfo *icon_info =
        gtk_icon_theme_lookup_icon(icon_theme,
                                   "application-x-executable",
                                   requested_icon_size,
                                   GTK_ICON_LOOKUP_USE_BUILTIN
                                   | GTK_ICON_LOOKUP_FORCE_SIZE);

    if (G_UNLIKELY(icon_info == NULL))
    {
        g_print("_pixbuf_get_default : %s : icon_info = null\n", id);

        return NULL;
    }

    return gtk_icon_info_load_icon(icon_info, NULL);
}

static void _treeview_row_activated(GtkTreeView *tree_view, GtkTreePath *path,
                                    GtkTreeViewColumn *column, AppWindow *window)
{
    (void) column;
    (void) window;

    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);

    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter(model, &iter, path))
        return;

    c_autounref GAppInfo *info = NULL;
    c_autofree char *file = NULL;

    gtk_tree_model_get(model, &iter,
                       COL_INFO, &info,
                       COL_FILE, &file,
                       -1);

    const gchar *filepath = g_desktop_app_info_get_filename(G_DESKTOP_APP_INFO(info));
    gchar *cmd = g_strdup_printf("exo-open %s", filepath);
    g_spawn_command_line_async(cmd, NULL);
    g_free(cmd);
}


