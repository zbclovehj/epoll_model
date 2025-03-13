# epoll_model_Linux
底层为红黑树在 Linux 内核中，epoll 的实现确实使用了红黑树数据结构来提高事件的管理效率。红黑树是一种自平衡的二叉查找树，具有较快的插入、删除和查找操作的时间复杂度。

在 epoll 的实现中，主要有以下几个关键的数据结构和算法：

红黑树：
epoll 使用红黑树来存储需要监听的文件描述符和事件。
通过红黑树的特性，可以快速查找和管理需要监听的文件描述符，提高事件管理的效率。
双链表：
除了红黑树，epoll 还使用了双链表来存储事件就绪的文件描述符。
当事件就绪时，内核将文件描述符从红黑树中移动到双链表中，以便应用程序能够快速访问就绪的事件。
事件表：
epoll 维护了一个事件表，其中包含红黑树和双链表等数据结构，用于管理事件的注册、删除和就绪通知。
通过使用红黑树等高效的数据结构，epoll 能够有效地处理大量的文件描述符和事件，提供高性能的 I/O 多路复用机制。红黑树的特性使得 epoll 在添加、查找和删除事件时具有较好的时间复杂度，有利于系统的扩展和性能优化。


在 Linux 系统下，常用的 I/O 多路复用机制是 epoll，它是一个高性能的事件通知接口，用于处理大量的文件描述符的 I/O 事件。epoll 通过系统调用实现，可以高效地管理大量的并发连接和事件。

而在 Windows 系统下，对应的高性能 I/O 多路复用机制是 I/O Completion Ports (IOCP)。IOCP 是 Windows 平台上用于高性能异步 I/O 完成通知的机制，适用于处理大量并发的 I/O 操作。









水平触发与边缘触发
水平触发(level-trggered)

只要文件描述符关联的读内核缓冲区非空，有数据可以读取，就一直发出可读信号进行通知，
当文件描述符关联的内核写缓冲区不满，有空间可以写入，就一直发出可写信号进行通知

边缘触发(edge-triggered)

当文件描述符关联的读内核缓冲区由空转化为非空的时候，则发出可读信号进行通知，
当文件描述符关联的内核写缓冲区由满转化为不满的时候，则发出可写信号进行通知

两者的区别？
水平触发是只要读缓冲区有数据，就会一直触发可读信号，而边缘触发仅仅在空变为非空的时候通知一次，举个例子：

读缓冲区刚开始是空的
读缓冲区写入2KB数据
水平触发和边缘触发模式此时都会发出可读信号
收到信号通知后，读取了1kb的数据，读缓冲区还剩余1KB数据
水平触发会再次进行通知，而边缘触发不会再进行通知

所以边缘触发需要一次性的把缓冲区的数据读完为止，也就是一直读，直到读到EGAIN(EGAIN说明缓冲区已经空了)为止，因为这一点，边缘触发需要设置文件句柄为非阻塞。

ET模式在很大程度上减少了epoll事件被重复触发的次数，因此效率要比LT模式高。epoll工作在ET模式的时候，必须使用非阻塞套接口，以避免由于一个文件句柄的阻塞读/阻塞写操作把处理多个文件描述符的任务饿死。














在 Linux 系统中，epoll 是一种高效的 I/O 多路复用机制，适用于处理大量并发连接。下面将详细介绍 epoll 模型涉及的主要函数、它们底层所做的事情以及作用。
1. epoll_create 和 epoll_create1
函数原型
#include <sys/epoll.h>
int epoll_create(int size);
int epoll_create1(int flags);
作用
这两个函数用于创建一个 epoll 实例，返回一个文件描述符，后续的 epoll 操作都将基于这个文件描述符进行。
底层操作
epoll_create：在 Linux 内核中分配一个 epoll 实例所需的内核数据结构，size 参数在较新的内核中已被忽略，但为了兼容性，通常会传入一个大于 0 的值。
epoll_create1：功能与 epoll_create 类似，但通过 flags 参数可以设置一些额外的选项。如果 flags 为 0，则与 epoll_create 效果相同；如果 flags 为 EPOLL_CLOEXEC，则会设置文件描述符的 close-on-exec 标志，确保在执行 exec 系列函数时关闭该文件描述符。
返回值
成功时返回一个非负的 epoll 实例文件描述符；失败时返回 -1，并设置 errno。
2. epoll_ctl
函数原型
#include <sys/epoll.h>
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
作用
该函数用于对 epoll 实例进行控制，包括添加、修改和删除需要监听的文件描述符及其对应的事件。
参数解释
epfd：epoll 实例的文件描述符，由 epoll_create 或 epoll_create1 返回。
op：操作类型，有以下三种取值：
EPOLL_CTL_ADD：将 fd 对应的文件描述符添加到 epoll 实例中，并监听 event 所指定的事件。
EPOLL_CTL_MOD：修改已经在 epoll 实例中注册的 fd 所监听的事件为 event 所指定的事件。
EPOLL_CTL_DEL：将 fd 对应的文件描述符从 epoll 实例中移除，此时 event 参数可以为 NULL。
fd：需要监听的文件描述符，如套接字描述符、文件描述符等。
event：指向 struct epoll_event 结构体的指针，用于指定需要监听的事件类型和相关数据。struct epoll_event 的定义如下：
typedef union epoll_data {
    void        *ptr;
    int          fd;
    uint32_t     u32;
    uint64_t     u64;
} epoll_data_t;

struct epoll_event {
    uint32_t     events;      /* Epoll events */
    epoll_data_t data;        /* User data variable */
};
其中，events 可以是以下事件标志的组合：
EPOLLIN：文件描述符可读。
EPOLLOUT：文件描述符可写。
EPOLLRDHUP：对端关闭连接或半关闭写端。
EPOLLPRI：有紧急数据可读。
EPOLLERR：文件描述符发生错误。
EPOLLHUP：文件描述符被挂起。
EPOLLET：使用边缘触发模式（默认是水平触发模式）。
底层操作
内核会根据 op 参数的不同，对 epoll 实例中的数据结构进行相应的操作。例如，当 op 为 EPOLL_CTL_ADD 时，内核会将 fd 对应的文件描述符添加到 epoll 实例的红黑树中，并关联 event 所指定的事件；当 op 为 EPOLL_CTL_DEL 时，内核会从红黑树中移除该文件描述符。
返回值
成功时返回 0；失败时返回 -1，并设置 errno。
3. epoll_wait
函数原型
#include <sys/epoll.h>
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
作用
该函数用于等待在 epoll 实例中注册的文件描述符上发生的事件。
参数解释
epfd：epoll 实例的文件描述符。
events：指向 struct epoll_event 结构体数组的指针，用于存储发生事件的文件描述符及其相关信息。
maxevents：events 数组的最大元素个数，即 epoll_wait 函数最多能返回的事件数量。
timeout：超时时间，单位为毫秒。有以下几种取值：
-1：表示 epoll_wait 函数会一直阻塞，直到有事件发生才返回。
0：表示函数会立即返回，无论是否有事件发生。
正整数：表示函数会阻塞指定的毫秒数，如果在这段时间内没有事件发生，函数也会返回。
底层操作
内核会维护一个就绪队列，当注册的文件描述符上发生了指定的事件时，内核会将该文件描述符及其相关信息添加到就绪队列中。epoll_wait 函数会检查就绪队列，如果队列不为空，则将队列中的事件信息复制到 events 数组中，并返回发生事件的文件描述符的数量；如果队列是空的，且 timeout 不为 0，则函数会阻塞等待，直到有事件发生或超时。
返回值
成功时返回发生事件的文件描述符的数量；如果超时时间已到但没有事件发生，返回 0；如果发生错误，返回 -1，并设置 errno。

总结
epoll 模型通过 epoll_create 或 epoll_create1 创建实例，epoll_ctl 对监听的文件描述符进行管理，epoll_wait 等待事件发生，从而实现高效的 I/O 多路复用。其底层利用红黑树存储监听的文件描述符，用就绪队列存储发生事件的文件描述符，大大提高了处理大量并发连接的性能。
