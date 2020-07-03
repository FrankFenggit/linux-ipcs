# linux-IPCs
[TOC]



# 进程间通信方式汇总
1. 管道(PIPE)
2. FIFO(有名管道)
3. XSI消息队列
4. XSI信号量
5. XSI共享内存
6. POSIX信号量
7. 域套接字(Domain Socket)
8. 信号(Signal)
9. 互斥量(Mutex)

其中信号(signal)和信号量(semaphore)本质上并不算是进程间通信方式，应该是进程间同步的方式，但是也可以起到一定的通信作用，故也列在上面。

另外普通的mutex是作用线程间同步用的，但是可以将进程A和进程B共享的内存中初始化一个mutex，这样就可以用将此mutex用作进程间通信用了。

# 扩展
## 进程与内核通信
其实本来的计划是分两个大块，一块写进程间通信，一块写内核与用户空间通信。后来时间有限，内核与进程间只写了一个netlink，所以没有放到这里，等以后有时间了再补充吧。
## 线程间同步
同一个进程的多个线程在同一个地址空间，通信是很容易的事情，因此多线程间要同步就好了。写了一个linux多线程的同步方式的汇总，在以下仓库中[clpsz/linux-itss](https://github.com/clpsz/linux-itss)，供参考，欢迎讨论。

## IPC常用shell命令

```shell
#查看消息队列
ipcs -q
# 显示资源限制
ipcs -l
#删除消息队列
ipcrm -q <MessageID>
#更多命令请查系统帮助
ipcs --help
ipcrm --help
```



# [IPC-消息队列](https://www.jianshu.com/p/9351878be73b)

## 1.0 **消息队列概念**

  消息队列是IPC（进程间通信，inter process communication）中常用的一种方式，相比与其他的通信方式消息队列具有在短消息处理和消息类别上有突出的表现。在理解消息队列前，需要了解一点关于Linux内核的知识。

![img](https:////upload-images.jianshu.io/upload_images/6883138-9f0843aa41555275?imageMogr2/auto-orient/strip|imageView2/2/w/700/format/webp)

  如图所示，Linux/unix的体系架构可以抽像成三层结构：应用程序（用户程序，软件等），内核（用于操作底层硬件）以及硬件。所以用户编写的程序是不能够直接操纵底层硬件的，需要通过系统内核。因此这就可以解释为什么共享内存的方式是IPC最快的方式，因为共享内存没有在内核中，而其他的包括消息队列是在内核中开辟空间，所以在访问速度上直接访问用户内存比内核空间要快。

## **1.1 消息队列结构**

  消息队列虽然不能够进程大量数据通信，但是却有一个很明显的优点，那就是在同一个消息队列中可以包含不同类型的消息，接收端可以根据自己的情况接受相应的消息，那么消息队列是怎样来保证这样的特点的呢？先看一下它的结构：

![img](https:////upload-images.jianshu.io/upload_images/6883138-02864e380927d665?imageMogr2/auto-orient/strip|imageView2/2/w/576/format/webp)

  如图在消息队列其实就是一个链表，其中可以存在不同的type，每一个type可能会有多条消息，接受端根据自己需要的type，一次从链表中获取第一个，第二个消息。



## **1.2 如何使用消息队列**

  消息队列的使用很简单，使用msgget( ), msgsnd( )，msgrcv( )以及msgctl( )就完全搞定。注意限制，请查看

### *创建一个消息队列*

　　生成一个消息队列或者获取已有消息队列id

```C++
       #include <sys/types.h>
       #include <sys/ipc.h>
       #include <sys/msg.h>
       int msgget(key_t key, int msgflg);
```

msgget函数返回key值对应的消息队列id。

　　1. key是一个用来与一个ipc对象进行对应的东西，起到在内核中标识的作用。

　　2.返回的id起到的是外部也就是我们应用层的标识作用，例如所有操作消息队列的函数，都是用msgid来唯一确定一个消息队列。

　　3.msgflg用来指定消息队列的权限，操作属性，高位为操作属性，地位为操作权限，比如msgflg通常使用的高位值:

　　　　**IPC_CREAT**: 用来创建一个消息队列

　　　　**IPC_EXCL**: IPC_CREAT  和  IPC_EXCL都指定，ipc资源已经存在，返回-1(有坑)

　　　　**IPC_NOWAIT**: 之后的消息队列操作都为非阻塞

　　　　

　　当key值为 IPC_PRIVATE 时，或者key值不为它但是msgflg指定了 IPC_CREAT ，则创建一个新的消息队列，如果这个消息队列不存在时。指定 IPC_PRIVATE 为key值时，总是创建一个新的消息队列，生成的消息队列key值为0。

 

由此可见获得消息队列操作 ID是很关键的，一般有3种方法获得id：

　　1.指定key值为 IPC_PRIVATE ,创建一个新的消息队列，让后将id值写入一个文件，另一个进程读取该文件，获得id值，这样2个进程就可以通过这个消息队列通信了。

　　2.手动指定key值为某个值，多个进程都看看得到这个key,这样做的问题就是可能有一个key值和指定key值一样的消息队列存在了，需要处理这样的错误，换一个key值。

　　3.使用ftok函数生成一个key，同一个key调用msgget得到的id肯定是相同的。

```c++
       #include <sys/types.h>
       #include <sys/ipc.h>

       key_t ftok(const char *pathname, int proj_id);　　（proj_id is 1-255）
```

 

　　ftok函数的实现是通过stat函数获取pathname的st_dev，st_ino成员（部分位）和proj_id（只用低8位）进行组合生成一个key值，这样做的话，只要pathname和proj_id确定，那么key值基本就确定。

　　1.但是这个函数依然存在一种可能就是pathname不一样，proj_id一样，仍然得到一个一样的key，这是因为st_dev,st_ino成员数据被截断了，可能刚好保留的数据是相同的。

　　2.还有一点就是必须确保pathname这个文件全程都不会被改动，否则A进程获得key之后，在B进程获取key之前，修改pathname这个文件，从而影响st_dev,st_ino，导致B得到的key和A不一样，虽然pathname,proj_id并没有变。

 

### **消息队列属性描述**

　　通过 msgctl可以获得，设置，删除消息队列

```c++
       #include <sys/types.h>
       #include <sys/ipc.h>
       #include <sys/msg.h>

       int msgctl(int msqid, int cmd, struct msqid_ds *buf);
```

 

　　它的使用类似io操作中的ioctl。cmd有如下值：

　　IPC_STAT:获取由msgid指定的消息队列的描述结构体，存放于buf中。

　　IPC_SET:设置消息队列的描述结构体

　　IPC_RMID:立刻删除指定key值的消息队列，key值存放于buf结构体中。如果删除后仍有进程读写这个消息队列，则返回EIDRM错误。

一般，如果消息队列出错了，使用IPC_RMID删除消息队列，释放它在内核中占有的资源。

 

### **消息的发送和接收**

　　消息队列是在一定的空间内建立一个链表，每次都将最近一个发送的消息放在队列链表的末尾，当取走一个消息时，它占据的相应空间就释放出来用以给后面要加入的消息使用。

　　通过函数msgsnd,msgrcv可以实现对指定消息队列进行消息发送和接收。

```c++
       #include <sys/types.h>
       #include <sys/ipc.h>
       #include <sys/msg.h>

       int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);

       ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
```

 

　　**msgsnd**：向msgid指定的消息队列末尾追加一个由msgp指向的消息，消息内容大小为msgsz。通常消息通过一个结构体进行描述,一般形式如下:

```c++
           struct msgbuf {
               long mtype;       /* message type, must be > 0 */
               char mtext[1];    /* message data */
           };
```

 

这个结构体可分为2个部分，mtype用以标识消息类型，mtext这个部分就是消息的内容，可以是你想要的描述消息内容的任何形式如数组，结构体等等。**msgsnd,msgrcv里面所指定的消息大小msgsz指的是消息结构体内容mtext部分的长度，不包括mtype!消息类型必须大于0，至于为什么，看到msgrcv函数时就可以知道。**

　　msgflg:通常有2种取值：

　　　　**0**：默认值，忽略标识位。消息发送时，当消息队列里面的空间超过限制值时，msgsnd将发生阻塞，直到空间腾出来可以满足需求（就是有人读走消息），或者msgid指定的消息队列已被删除，或者捕获一个信号，否则立马返回。

　　　　**IPC_NOWAIT**：非阻塞，空间不足时不阻塞，而是返回一个错误值**EAGAIN**。

　　一个阻塞的msgsnd是可以被信号中断的。中断后返回错误，错误值**EINTR**。msgsnd函数一旦被信号中断，永远都不会重启调用，即便安装信号处理函数时指定了重启标识**SA_RESTART**。因此对于一个阻塞的消息队列，要注意对其错误的处理。

 

　　**msgrcv**：从msgid指定的消息队列里面取出由msgtyp指定类型的消息存放于msgp指向的空间。取出的消息数据大小由msgsz指定。成功时，返回拷贝到mtext中实际的字节数。

　　msgtyp有3种情况，用以控制取出消息的方式：

　　　　等于0:取出队列中的第一个消息，这样可以以先进先出的方式取消息（因为msgsnd都是把消息添加到消息队列的最后面）。

　　　　大于0:取出mtype于msgtyp相同的消息。

　　　　小于0:取出消息队列中mtype值小于等于msgtyp绝对值的所有消息中mtype值最小的那个消息。（假设mtype设定为消息的优先级，这种方式可以用于控制消息队列取消息的优先级）

　　从上面的描述可以看出这就是为什么消息结构体中mtype值为什么一定要大于0的原因。

　　msgflg有多种组合方式:

　　　　**0**:忽略标识位，当消息队列中无指定消息时阻塞，直到有指定类型消息，或者消息队列被删除，或者被信号中断。后面2种都会返回错误。

　　　　**IPC_NOWAIT**:非阻塞方式读取消息。无消息时返回**ENOMSG**

　　　　**MSG_NOERROR**：如果消息类型匹配上了，但是消息数据大小大于msgrcv指定的msgsz,则返回错误**E2BIG。**但是如果指定了**MSG_NOERROR**标识，则过大的消息数据按照msgsz指定的大小截断。

　　　　**MSG_EXCEPT**：如果msgtyp大于0，指定此标识表示按照先进先出的方式取出第一个非msgtyp的消息。

　

　　一个消息队列中的消息大小是可以不一样的，即便是同一类型的消息，这样当消息数据是变长时，避免出现空间的浪费。配合**不**使用**MSG_NOERROR**和对错误的判断可以提取出同一类型消息中指定长度的消息。

消息队列的demo

消息队列如何使用？一个简单的例子：

发送端：

![img](https:////upload-images.jianshu.io/upload_images/6883138-9267e4038d3c1474?imageMogr2/auto-orient/strip|imageView2/2/w/640/format/webp)

接受端：

![img](https:////upload-images.jianshu.io/upload_images/6883138-0370d1bfedce89e4?imageMogr2/auto-orient/strip|imageView2/2/w/709/format/webp)

### 踩坑

#### ftok 得到的key一样，同过msgget 得到的msgid不一定相同。比如删除消息队列重新创建消息队列。

```
实验环境：虚拟机 Ubuntu 20.04 LTS

实验过程：删除消息队列重新创建

结果：通过 ipcs -q 观测，msgid自动加1
```



#### 

## 1.3 消息队列的进阶

  在使用消息队列的时候，你是否会有这样的疑问：如果多个进程同时向一个消息队列里面同一个Type发送消息，那么是否会出现资源竞争同步的问题呢？

![img](https:////upload-images.jianshu.io/upload_images/6883138-6d54b90c8d82aa1b?imageMogr2/auto-orient/strip|imageView2/2/w/847/format/webp)

  是否会出现上面图中的局面呢，也就是说多个进程同时往消息队列里面写，那么就会同时认为msg1是链表的末尾，于是都把自己的消息添加到msg1的后面，导致了上面的现象，在接收端取数据的时候就会出现问题了。

  正确答案是，系统并没有这么傻，对于消息队列而言，一定要保证单链表，在内核代码中其实是做了同步的，**也就是说当多个进程同时写的话，内核里面也会一个一个的进行链表的连接。我们大可放心****，且不像共享内存一样需要用户自己同步，在使用消息队列的时候用户不需要进行同步**。

  内核代码里面有一个ipc_lock_check()来进行同步，保证资源竞争。



