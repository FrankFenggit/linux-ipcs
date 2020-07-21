#include "com_wei_myobject.h"

G_DEFINE_TYPE(ComWeiMyObject,com_wei_myobject,G_TYPE_OBJECT)

static void com_wei_myobject_init(ComWeiMyObject * object)
{
    //这个两个init函数大概是GObject的套路，在这个简单的小例子中，没有什么特别的初始化处理
}

static void com_wei_myobject_class_init(ComWeiMyObjectClass * klass)
{
    
}

gboolean com_wei_test (ComWeiMyObject * obj, const guint IN_x, gdouble* OUT_d_ret, GError ** error)
{ 
    //我们只做测试，简单检测输入参数，直接回复输出结果
    printf("com_wei_test() get input param: x= %d/n",IN_x);
    *OUT_d_ret = 0.99;
    return TRUE;
}