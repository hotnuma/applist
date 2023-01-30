#include "appwindow.h"
#include "preferences.h"

#include <gtk/gtk.h>

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

    GtkWidget *window = g_object_new(TYPE_APPWINDOW, NULL);

    gtk_widget_show_all(window);

    gtk_main();

    prefs_cleanup();

    return 0;
}


