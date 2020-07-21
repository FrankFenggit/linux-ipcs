#ifndef COM_WEI_MYOBJECT_H
#define COM_WEI_MYOBJECT_H

#include <glib.h>
#include <dbus/dbus-glib.h>

typedef struct ComWeiMyObject ComWeiMyObject;
typedef struct ComWeiMyObjectClass ComWeiMyObjectClass;

struct ComWeiMyObject
{
    GObject parent;
};

struct ComWeiMyObjectClass
{
    GObjectClass parent;
};

#define COM_WEI_MYOBJECT_TYPE  (com_wei_myobject_get_type())

GType com_wei_myobject_get_type(void);
gboolean com_wei_test(ComWeiMyObject *obj , const guint IN_x, gdouble *OUT_d_ret, GError **error);

#endif