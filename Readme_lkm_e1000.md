##e1000可加载模块说明
李智康
feitianlzk@gmail.com

- - -

[TOC]


###基本介绍
在2015/ucore4edison田博学长已经完成的内核e1000 driver基础上,借鉴2015/KernelModule对linux module的处理方式,最终将e1000 driver改变为ucore内核可加载模块

###主要工作
- e1000驱动可加载模块的改造
- lkm接口框架的进一步规范
- lkm的导入
- 中断注册框架的添加

###测试方法
在Ubuntu14.04下打开qemu,通过主机(Ubuntu)向qemu内guest os 的网络地址ping,如果能收到正确返回,说明e1000驱动的基本功能可以在ucore中使用,即完成了基本移植工作。
具体测试方法为
 1. ifconfig br0 10.0.2.2 netmask 255.255.255.0
 2. sudo ./uCore_run
 3. ping 10.0.2.15
 4. insmod mod-e1000(在ucore的shell)
 5. ping 10.0.2.15
 6. rmmod mod-e1000(在ucore的shell)
 7. ping 10.0.2.15

应该可以看到三次结果分别为
From 10.0.2.2 icmp_seq=1 Destination Host Unreachable
64 bytes from 10.0.2.15: icmp_seq=1 ttl=58 time=6.09 ms
From 10.0.2.2 icmp_seq=1 Destination Host Unreachable
>>说明
>>1. 田博学长在ucore中移植了一个小型的ip栈(lwip),e1000 dirver为lwip的底层函数提供支持，因而ping可以得到正确的返回结果
>>2. 在默认情况下,qemu内的guest os是无法直接从外部网络访问的,需要配置TAP connection。具体配置可以参考[1](http://nairobi-embedded.org/a_qemu_tap_networking_setup.html)
[2](https://tthtlc.wordpress.com/2015/10/21/qemu-how-to-setup-tuntap-bridge-networking/)
>>3. 由于现在还没有在ucore中添加ifconfig,无法通过系统调用完成driver的open和close工作，因此将open和close放在模块的init和exit中
>>4. qemu和host network相连的网关地址是10.0.2.2,qemu内部操作系统的虚拟ip地址是10.0.2.15 qemu network详细内容可参考[1](http://wiki.qemu.org/Documentation/Networking) [2](https://en.wikibooks.org/wiki/QEMU/Networking#TAP_interfaces)
>>5. 为此对uCore_run进行了修改，qemu的启动参数增加
```
EXTRA_OPT+=" -net nic,macaddr=`source genmac.sh` -net tap,ifname=tap    0,script=qemu-ifup,downscript=qemu-ifdown"
```
>>6. 因为ucore slab最大支持128K,而e1000 module > 128k,故修改mm/slab.c MAX_SIZE_ORDER

###loadable kernel module的支持
1. 往届已经比较完善，推荐阅读[Linux 可加载内核模块剖析](http://www.ibm.com/developerworks/cn/linux/l-lkm/),但在加载linux module时,因为linux module和ucore module大小不同,所以会出现关键位置,所以要在ucore module结构中增加padding使之对齐^[1]^,因此在移植新的linux模块的时候，可能要对padding进行修改,参考OsTrain2015/KernelModule，可以通过比较src/kern-ucore/kmodule/modules/mod-test2的结果和项目根目录下mod-hello-test的结果进行快速修改。
2. 对于如何解决linux header和ucore header 函数命名相同的情况下对ucore资源的调用，参考2015/KernelModule的处理方式，设计kmodule文件的结构如下  
-- LinuxAdapter  
|  
| - ucoreFunctions   
|  (只能引用ucore的header,   
|   设计ucore_*函数,  
|	内部调用ucore中的函数)  
|  
| - linuxFunctions   
|  (只能引用ucore的header,    
|   一些linux常用函数的实现,  
|	调用uCoreFunctions中的函数)   
|  
| - include   
|  (一个通过header-gen生成的linux header组)   
|   
-- modules  
|  
| - mod-A   
>	| - A.c  
>    | - A.dummy.c  
>    |	(linux针对A特有函数的实现  
>    |	作为module的一部分）  
>    |	 
>    | - mod-A.dep  
>    |	(各模块依赖关系)  
 
 |   
 | -	...   
    
 比如linux和ucore中都有memset这个函数,现在形成的调用关系为memset(linux)-->ucore_memset(ucoreFunctions)-->memset(ucore)
同时在kmodule/Makefile中添加-ILinuxAdapter/include，通过这两种方式,实现了ucore func和linux func的隔离。在LinuxAdapter/Makefile中ucoreFunctions先于linuxFunctions编译，确保linuxFunctons中函数可以调用ucoreFunctions中函数。在未来添加新的lkm时,在\*.dummy.c中实现对应的函数,就可以实现模块的可加载,为了实现linuxFunctions函数符号的导出,新建了LinuxAdapter/include/ucore_export.h函数，在其中定义了`EXPORT_SYMBOL`
通过引入linuxFunctions文件夹,可以减少\*.dummy.c中函数的重复实现，减小module的大小


总结一下将放在内核的驱动改为内核可加载模块需要做的工作
1. 仿照mod-hello模块新建该模块的目录，仿照hello.c，定义init，exit函数及struct module等模块必备的函数和头文件
2. 补全\*.dummy.c中的函数
3. 若模块需要使用内核中的全局变量或函数，需调用EXPORT_SYMBOL函数将该符号导入符号表，同时在该模块的.c文件中声明该变量
4. 为该模块添加makefile及\*.dep文件，并在modules目录下的makefile中添加该模块的名字

###device driver 的支持
最终的目标是要不加修改的在ucore中使用linux的driver，那么就需要在ucore添加对应的支持，对于物理driver，主要有I/O remap,中断的动态注册,pci总线的支持，主要的文件和函数如下
1. I/O ioremap
	Depending on the computer platform and bus being used, I/O memory may or may
not be accessed through page tables. When access passes though page tables, the
**kernel must first arrange for the physical address to be visible from your driver, and
this usually means that you must call ioremap before doing any I/O**^[2]^
在2015/ucore4edison中已有实现,需要做的只是导出ucore_ioremap
`void *ioremap(uintptr_t pa, size_t size) arch/i386/mm/pmm.c`

2. 中断动态处理框架 
	因为本实验是基于田博的代码,而在田博的代码中中断的硬件是基于APIC,而不是我们现有的8259A,但在目前只有单cpu的前提下，二者没有实质性区别。因为可加载模块涉及到中断的动态的添加和删除,而现有的swtich-case不能满足现有需求，所以需要设计新的框架。基本上采用了2014/RaspberryPi叶紫的框架^[3]^,是参考linux的中断管理,在《深入Linux设备驱动程序内核机制》书的中断处理一章有详细的介绍，在ucore中的简化版本如下    
`arch/i386/driver/picirq.h`
```
typedef int (*ucore_irq_handler_t) (int, void *);
```
`arch/i386/driver/picirq.c`
```
struct irq_action {
  ucore_irq_handler_t handler;
  void *opaque;
};
...
struct irq_action actions[NR_IRQS];
```
对每一中断号都有一个对应的函数handler,在硬件中断进入trap函数后,在通过中断号进入对应的函数,因为handler只是一个函数指针,所以可以十分方便的进行中断处理函数的添加删除，主要利用以下三个函数
 - irq_handler	根据irq进入actions[irq].handler执行处理程序
 - register_irq	完成具体中断函数和actions[irq].handler的对应
 - irq_clear	解除具体中断函数和actions[irq].handler的对应

3. pci	
	ucore内核启动并完成对所有PCI设备进行扫描、登录和分配资源等初始化操作的同时，会建立起系统中所有PCI设备的拓扑结构，目前只检测了PCI_CLASS_BRIDGE和e1000网卡类型，后续有需要可以添加其他类型。这部分代码主要位于`arch/i386/driver/pci.c`,在2015/ucore4edison已有很好的实现，不难理解。另外可以阅读[Linux下PCI设备驱动程序开发](https://www.ibm.com/developerworks/cn/linux/l-pci/)进行参考

4. dma
	下面一段摘自2015/ucore4edison报告
    DMA 的实现实际可以很简单。首先硬件只需要知道 DMA 区域的物理
地址就可以正常的 DMA,并不需要什么干预。由于 ucore 的内存机制比较
简单,分配一块 DMA 区域(dma_alloc_coherent)只需要在 kmalloc 后获取
相应的物理、虚拟地址,而虚拟内存的 DMA 映射(dma_map_single_attrs)
只需要返回虚拟地址的物理地址即可(减去偏移)^[4]^

要想对linux device driver有一个基本的了解， 推荐阅读[ldd3](https://lwn.net/Kernel/LDD3/)chapter9 I/O chapter10 Interrupt  chapter12 PCI

###e1000 lkm设计
主要继承了15/ucore4edison的设计，建立一个dde,抽取e1000中一些函数,对上层来说相当于是一个简化的e1000
由于ucore内核中没有device设备模型,无法支持hot plug,因此做了一些修改,主要集中在
1. `arch/i386/driver/pci.c`
`e1000_func_init`
目前是pci搜索到e1000物理设备后，在内核中新建一个`pci_func e1000_func`去存放e1000的相应信息,e1000 driver在注册的时候直接从内核得到相应信息,(以后可以新建一个链表结构改为在链表中搜索)
`struct net_device_ops e1000_netdev_ops`
使用`e1000_netdev_ops`来提供e1000函数注册到内核使用的接口.目前内部只有一个传输函数`transmit_pkt`,如果后续对e1000功能进行增强,在`net_device_ops`内部添加新成员即可

2. `kmodule/modules/e1000/*.[ch]`
`request_threaded_irq`	调用register_irq注册irq,并ioapicenable使能irq
`pci_register_e1000`	e1000driver在内核的注册,主要包括e1000_dev(pci_dev)的初始化,e1000_dev和对应e1000 netdevice的绑定,e1000函数在内核的注册
`pci_unregister_e1000`	与pci_register_e1000相反

###待完善
以下部分正在继续完善过程中
- [ ] 中断框架的完善,在一根IRQ线上添加多个中断
- [ ] 添加ifconfig函数和ioctl系统调用

###参考资料
[1] [OsTrain2015/KernelModule](http://166.111.68.197:11123/oscourse/OS2015/projects/KernModule)
[2]	[ldd3](https://lwn.net/Kernel/LDD3/)
[3] [OS2014/projects/RaspberryPi](http://166.111.68.197:11123/oscourse/OS2014/projects/RaspberryPi)
[4] [2015/ucore4edison](http://166.111.68.197:11123/oscourse/ucore4edison)
