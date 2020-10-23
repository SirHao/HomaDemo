# HomaDemo
基于HomaModule实现的Server-Client的linux socket代码
+ system: ubuntu 18
+ kernel: 4.15.0-20-generic

### step1: clone & make & install HomaModule
use to install homa into kernel:
```
git clone https://github.com/PlatformLab/HomaModule.git
cd HomaModule
make #need g++ 
insmod homa.ko
```
### step2: use Homa
homaModule sending/recving msg is based on ioctl,so you need some code like ``homa_api.c``'s when you need to send/recv msg;just read the simple demo;
