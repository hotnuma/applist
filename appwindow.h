#ifndef APPWINDOW_H
#define APPWINDOW_H

#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _AppWindowClass AppWindowClass;
typedef struct _AppWindow      AppWindow;

#define TYPE_WINDOW (window_get_type())
#define WINDOW(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_WINDOW, AppWindow))
#define WINDOW_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass),  TYPE_WINDOW, AppWindowClass))
#define IS_WINDOW(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_WINDOW))
#define IS_WINDOW_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass),  TYPE_WINDOW))
#define WINDOW_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj),  TYPE_WINDOW, AppWindowClass))

GType window_get_type() G_GNUC_CONST;

G_END_DECLS

#endif // APPWINDOW_H


