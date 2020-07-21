
#include <signal.h>
#include "object/com_wei_myobject.h"
#include "interface/wei_server.h"  //注意此两头文件的先后顺序

static void MyExit(int i)
{
	_exit(0);
}

int main(int argc, char ** argv)
{
    signal(SIGINT, MyExit); // 方便中断退出

    DBusGConnection * conn;
    GMainLoop * main_loop = NULL;
    ComWeiMyObject * obj = NULL;
    GError * error = NULL;
    DBusGProxy * bus_proxy;
    int request_name_result;

#if !GLIB_CHECK_VERSION(2,36,0)
    g_type_init();
#endif
    main_loop = g_main_loop_new(NULL,FALSE);
    // 在自动生成的wei_server.h中定义了DBusGObjectInfo dbus_glib_com_wei_object_info， Install introspection information about the given object GType sufficient to allow methods on the object to be invoked by name. 这样，可以在收到信息的时候，可以触发调用它的方法。
    dbus_g_object_type_install_info(COM_WEI_MYOBJECT_TYPE , &dbus_glib_com_wei_object_info);

    //建议与session dbus的连接，在以前学习过
    conn = dbus_g_bus_get(DBUS_BUS_SESSION,&error);
    if(conn == NULL){
        if (error != NULL) {
            g_printerr("Fail to open connection to bus %s\n", error->message);
            g_error_free(error);
        } else {
            g_printerr("Fail to open connection to bus\n");
        }
        return -1;
    }

    // 为连接起一个名字，对于GLib，这个处理比底层的API接口复杂，需要向系统DBus的管理者org.freedesktop.DBus，调用它的接口的方法"RequestName"来实现。居然更复杂，比较奇怪
    bus_proxy = dbus_g_proxy_new_for_name(conn, "org.freedesktop.DBus","/","org.freedesktop.DBus");
    if (bus_proxy == NULL) {
        g_printerr("Create bus_proxy object fail\n");
        if (error != NULL) {
            g_error_free(error);
        }
        return -1;
    }

    if (error != NULL) {
        g_error_free(error);
        error = NULL;
    }

    if(!dbus_g_proxy_call(bus_proxy,"RequestName",&error,
                    G_TYPE_STRING,"com.wei.MyObject", G_TYPE_UINT,0, G_TYPE_INVALID,
                    G_TYPE_UINT, &request_name_result, G_TYPE_INVALID)){
        if (error != NULL) {
            g_printerr("Request name failed %s\n", error->message);
        } else {
            g_printerr("Request name failed\n");
        }
        g_error_free(error);
        g_object_unref(bus_proxy);
        return -1;
    }

    obj = g_object_new(COM_WEI_MYOBJECT_TYPE,NULL);
    //将对象在指定的path注册到连接上。 Registers a GObject at the given path. 当有消息通过连接，经过路径，找到对象后，根据前面前面的dbus_g_object_type_insall_info，能够调用对应的处理方式。
    dbus_g_connection_register_g_object(conn, "/com/wei/MyObject",G_OBJECT(obj));

    g_main_loop_run(main_loop);

    return 0;
}