# 1. VMware / Ubuntu的UHD环境检查

检查日期：2026-11-03；检查对象是当前Ubuntu虚拟机，不是VMware宿主机设置界面。未修改或安装软件，未启动RF发射。

## 实测结果

| 检查项 | 当前结果 | 判断 |
| --- | --- | --- |
| 客体系统 | Ubuntu 20.04.6 LTS，x86_64，systemd-detect-virt=vmware | 确认运行在VMware |
| USB控制器 | lsusb -t存在xHCI SuperSpeed根集线器，标称10000M | 有虚拟USB 3控制器，不证明B210已按USB 3连接 |
| UHD工具 | /usr/local/bin/uhd_find_devices、uhd_usrp_probe、uhd_config_info | 已安装 |
| UHD版本 | 3.15.0.HEAD-0-gaea0e2de，前缀/usr/local | 保留现有版本，暂不升级 |
| UHD组件 | 包含USB及B200系列支持 | 具备B210驱动基础 |
| 镜像 | /usr/local/share/uhd/images中有usrp_b200_fw.hex、usrp_b210_fpga.bin | 所需镜像文件存在，实际加载待验证 |
| gNB编译 | ENABLE_UHD=ON，使用/usr/local/lib/libuhd.so | 已启用UHD |
| gNB动态链接 | libuhd.so.3.15.0解析到/usr/local/lib | UHD链接可解析 |
| 本机独立PQ UE | ENABLE_UHD=OFF | 当前二进制为ZeroMQ用途，不能直接用于B210 |
| B210探测 | lsusb未发现；uhd_find_devices返回No UHD Devices Found | USB直通及实际收发未验证 |
| udev规则 | /usr/local/lib/uhd/utils/uhd-usrp.rules存在；常见系统规则目录未检出UHD命名文件 | 不能确认普通用户访问权限已配置；以探测结果为准 |

结论：**软件环境已具备基础条件，但硬件环境尚未验收。** 不需要现在重装整套UHD，也不能仅凭版本号宣称USRP环境全部可用。

## 连接和再次检查

1. B210接宿主机USB 3端口，使用USB 3线。
2. VMware设置中确认USB控制器支持USB 3。将B210连接到Ubuntu客体而非由宿主机占用；具体菜单名称随VMware版本不同。
3. 在Ubuntu执行：

```bash
lsusb
lsusb -t
uhd_config_info --version
uhd_find_devices --args 'type=b200'
```

UHD设备类型使用`b200`来匹配B2xx系列，正常输出应显示B210及序列号。B210本身应出现在SuperSpeed分支，通常显示5000M；只看到根集线器的10000M不算通过。

使用本机实际序列号继续：

```bash
export GNB_USRP_SERIAL='替换为实际序列号'
uhd_usrp_probe --args "type=b200,serial=$GNB_USRP_SERIAL"
```

若普通用户探测失败而sudo成功，检查设备权限。需要时才安装已有规则：

```bash
sudo install -m 644 /usr/local/lib/uhd/utils/uhd-usrp.rules \
  /etc/udev/rules.d/uhd-usrp.rules
sudo udevadm control --reload-rules
sudo udevadm trigger
```

之后重新连接/直通设备并再次探测。若sudo也无设备，优先检查物理连接、虚拟机归属、供电、USB速度及固件重枚举后的直通情况，不能只修改udev规则。

当前镜像已存在，不应先下载另一版本镜像替换；只有UHD明确报告镜像缺失/不匹配时，使用与当前UHD配套的下载工具处理。本机工具路径为`/usr/local/lib/uhd/utils/uhd_images_downloader.py`。

## 验收记录

保存设备型号、序列号、USB速度、UHD版本、probe结果；后续运行再观察持续underflow/overflow及时间错误。probe成功只是设备初始化通过，不等于5G RF接入已经通过。
