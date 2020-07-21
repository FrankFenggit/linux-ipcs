#include <stdio.h>
#include <stdlib.h>
#include <dbus/dbus-glib.h>
#include "interface/wei_client.h"

void wei_request(DBusGProxy * proxy)
{
    gdouble OUT_d_ret;
    GError * error = NULL;

    if(!com_wei_MyObject_Sample_test(proxy,1000,&OUT_d_ret,&error )){
       g_printerr("Error: %s/n",error->message);
       return;
    }

    g_print("Proxy get return d_ret = %f",OUT_d_ret);
}

int main( int argc , char ** argv)
{
    GError *error = NULL;
    DBusGConnection *connection = NULL;
    DBusGProxy *proxy = NULL;

#if !GLIB_CHECK_VERSION(2,36,0)
    g_type_init();
#endif

    /* dbus_g_bus_get用来建立连接，这里和session bus连接，也可以通过DBUS_BUS_SYSTEM与系统总线连接*/
    connection = dbus_g_bus_get (DBUS_BUS_SESSION, & error);
    if(connection == NULL){
        g_printerr ("Failed to open connection to bus : %s/n",error->message);
        g_error_free( error);
        exit( 1 );
    }

    /* Create a proxy object用来代表远端org.freedesktop.Notifications是系统带有的，可以使用DBUS_INTERFACE_INTROSPECTABLE等定义来标识它*/
    proxy = dbus_g_proxy_new_for_name (connection,
                                 "com.wei.MyObject" /* service */ , 
                                 "/com/wei/MyObject" /* path */ ,
                                 "com.wei.MyObject.Sample" /* interface,可以使用宏定义DBUS_INTERFACE_INTROSPECTABLE */ );
    if (proxy == NULL) {
        g_printerr("Create proxy object fail");
        if (error != NULL) {
            g_error_free(error);
        }
        return FALSE;
    }

    // 调用函数
    wei_request(proxy);

    g_object_unref(proxy);
    return 0;
}