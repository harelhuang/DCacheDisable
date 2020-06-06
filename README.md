## DCacheDisable
Type: linux kernel module

OS: ubuntu18

Platform: Xilinx ZCU104

Cpu: ARM A53 on ZU7

Usage: insmod = disable DCache while rmmod = enable DCache

Detail: 

I do it not to measure performance without DCache for fun but to resolve this controversial issue in TVM community. 

As a result, The consistency problem of vta in zcu 104 platform have been proved to be an internal logic bug in vta according to [RFC][VTA]A HLS C VTA bug by our tvm effort.

Here is the building process.

My platform is cortex-a53, ubuntu18 started at EL2 and switched to EL1 and supported SMP on four cpu cores. Thus, I need to turn off multi-core to ensure L2-cache coherency. Thanks to the feature of cpu-hot-plug, I just run :
```
echo '0' > /sys/devices/system/cpu/cpu1/online,
echo '0' > /sys/devices/system/cpu/cpu2/online,
echo '0' > /sys/devices/system/cpu/cpu3/online
```
then I run dmesg to verify that multi-core has been turn off.

I build the kernel source tree, cause I cannot find it in my linux. You can run uname -r to see your kernel version. And find in `/usr/src` to see whether your linux already have it.

I build the linux module. With gcc inline asm , I flush all cache and set sctlr_el1.c 0.
