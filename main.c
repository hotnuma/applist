#include "appwindow.h"
#include <gtk/gtk.h>
#include <macros.h>

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);

    //GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    /*GtkWidget *window =*/ g_object_new(TYPE_WINDOW, NULL);

    gtk_main();

    return 0;
}


