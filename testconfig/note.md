###configure br0
Inside the linux hosts (I am assuming Ubuntu 64-bit 14.04 host here, for other distro some variation may be needed):

    sudo apt-get install uml-utilities
    sudo apt-get install bridge-utils

Inside the host’s /etc/networking/interfaces file add this:

        auto br0
        iface br0 inet dhcp
        bridge_ports eth0
        bridge_stp off
        bridge_maxwait 0
        bridge_fd 0

Then issue “sudo brctl addbr br0”.   Check using “ifconfig” that “br0” interface is created:
```
br0       Link encap:Ethernet  HWaddr bc:ee:7b:8c:53:9a
          UP BROADCAST MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:0
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)
```
将本文件中的interfaces替换到/etc/networking/interfaces 即可
以下的在代码文件中已提供好
###QEMU VM TAP interfaces and the Linux bridge
The following set of scripts take care of adding/deleting a QEMU VM tapN interface to/from virtual bridge br0.
```
$ cat qemu_br0_ifup.sh
#!/bin/sh
switch=br0
echo "$0: adding tap interface \"$1\" to bridge \"$switch\""
ifconfig $1 0.0.0.0 up
brctl addif ${switch} $1
exit 0

$ cat qemu_br0_ifdown.sh
#!/bin/sh
switch=br0
echo "$0: deleting tap interface \"$1\" from bridge \"$switch\""
brctl delif $switch $1
ifconfig $1 0.0.0.0 down
exit 0
```
Do not forget to make them executable:
```
$ chmod u+x qemu_br0_if*
```

###Instantiating QEMU VMs
Along with specifying unique tapN interfaces for each NIC of (a) VM instance(s), each NIC must have a unique MAC. For this reason, create the following script:
```
$ cat genmac.sh
#!/bin/sh
printf 'DE:AD:BE:EF:%02X:%02X\n' $((RANDOM%256)) $((RANDOM%256))
```
Then fire-up qemu instances. Only the TAP-related options are shown here1:
```
$ sudo qemu-system-$ARCH ... -net nic,macaddr=`source genmac.sh` -net tap,ifname=tap0,script=qemu_br0_ifup.sh,downscript=qemu_br0_ifdown.sh

$ sudo qemu-system-$ARCH ... -net nic,macaddr=`source genmac.sh` -net tap,ifname=tap1,script=qemu_br0_ifup.sh,downscript=qemu_br0_ifdown.sh

(etc)
```

