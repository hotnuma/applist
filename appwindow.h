#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <glib-object.h>
#include <libtype.h>
#include <gtk/gtk.h>

#if 0
typedef struct _AppWindowClass AppWindowClass;
typedef struct _AppWindow      AppWindow;

#define TYPE_APPWINDOW (window_get_type())
#define APPWINDOW(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj),  TYPE_APPWINDOW, AppWindow))

GType window_get_type() G_GNUC_CONST;

#endif


#define TYPE_APPWINDOW (window_get_type())
EG_DECLARE_FINAL_TYPE(AppWindow, window, APPWINDOW, GtkWindow)

#endif // APPWINDOW_H


