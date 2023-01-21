#include <gtk/gtk.h>
#include <gio/gdesktopappinfo.h>
#include <cstring.h>
#include <libpath.h>
#include <libapp.h>
#include <stdbool.h>
#include <strings.h>

static GdkPixbuf* _pixbuf_get_default(GtkWidget *widget)
{
    GtkIconTheme *icon_theme = gtk_icon_theme_get_for_screen(
                                                gtk_widget_get_screen(widget));

    gint scale_factor = gtk_widget_get_scale_factor(widget);
    gint requested_icon_size = 24 * scale_factor;

    c_autounref GtkIconInfo *icon_info =
        gtk_icon_theme_lookup_icon(icon_theme,
                                   "application-x-executable",
                                   requested_icon_size,
                                   GTK_ICON_LOOKUP_USE_BUILTIN
                                   | GTK_ICON_LOOKUP_FORCE_SIZE);

    if (G_UNLIKELY(icon_info == NULL))
        return NULL;

    GdkPixbuf *pix_icon = gtk_icon_info_load_icon(icon_info, NULL);

    return pix_icon;
}


static GdkPixbuf* _pixbuf_from_gicon(GtkWidget *widget, GIcon *gicon)
{
    if (!gicon)
        return _pixbuf_get_default(widget);

    GtkIconTheme *icon_theme = gtk_icon_theme_get_for_screen(
                                                gtk_widget_get_screen(widget));

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
        g_print("icon_info = null\n");

        return _pixbuf_get_default(widget);;
    }

    GdkPixbuf *pix_icon = gtk_icon_info_load_icon(icon_info, NULL);

    if (!pix_icon)
        g_print("pix_icon = null\n");

    return pix_icon;
}

enum
{
    COL_ICON = 0,
    COL_TITLE,
    COL_FILE,
    NUM_COLS
};

static GtkListStore* treeview_create(GtkWidget *parent)
{
    c_autounref GtkListStore *store;
    store = gtk_list_store_new(NUM_COLS,
                               GDK_TYPE_PIXBUF,
                               G_TYPE_STRING,
                               G_TYPE_STRING);

    GtkWidget *treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

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

    // File
    col = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(col, "File");

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(col, renderer, TRUE);
    gtk_tree_view_column_set_attributes(col, renderer,
                                        "text", COL_FILE,
                                        NULL);

    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), col);

    gtk_container_add(GTK_CONTAINER(parent), treeview);

    return store;
}

static bool _append_line(GtkWidget *window, GtkListStore *store,
                         GAppInfo *info)
{
    GIcon *gicon = g_app_info_get_icon(info);

    c_autounref GdkPixbuf *pix = _pixbuf_from_gicon(window, gicon);

    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
                       COL_ICON, pix,
                       COL_TITLE, g_app_info_get_name(info),
                       COL_FILE, g_app_info_get_id(info),
                       -1);

    return true;
}

gboolean _app_info_show(GAppInfo *info)
{
    g_return_val_if_fail(G_IS_APP_INFO(info), FALSE);

    if (G_IS_DESKTOP_APP_INFO(info))
    {
        /* NoDisplay=true files should be visible in the interface,
         * because this key is intent to hide mime-helpers from the
         * application menu. Hidden=true is never returned by GIO. */
        return g_desktop_app_info_get_show_in(G_DESKTOP_APP_INFO(info), NULL);
    }

    return TRUE;
}

static gint _sort_app_infos(gconstpointer a, gconstpointer b)
{
    return strcasecmp(g_app_info_get_name(G_APP_INFO(a)),
                      g_app_info_get_name(G_APP_INFO(b)));
}

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "AppInfo List");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
    g_signal_connect(window, "delete_event", gtk_main_quit, NULL);

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(window), scroll);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scroll),
                                        GTK_SHADOW_IN);

    GtkListStore *store = treeview_create(scroll);

    GList *all = g_app_info_get_all();
    all = g_list_sort(all, _sort_app_infos);

    for (GList *lp = all; lp; lp = lp->next)
    {
        GAppInfo *info = G_APP_INFO(lp->data);
//        if (!_app_info_show(info))
//            continue;

        _append_line(window, store, info);
    }

    g_list_free_full(all, g_object_unref);

    gtk_widget_show_all(window);

    gtk_main();


    return 0;
}


