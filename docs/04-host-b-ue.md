# 4. 主机B：下载、构建和运行UHD版PQAKA srsUE

第二台主机环境由用户配置。本文以Ubuntu 20.04/22.04类环境为起点，不表示已经在该主机验证。更高版本的mbedTLS/CMake/UHD可能需要兼容性检查，不直接替换当前源码版本。

## 1. 安装依赖并确认设备

```bash
sudo apt update
sudo apt install build-essential cmake git pkg-config python3 \
  libfftw3-dev libmbedtls-dev libboost-program-options-dev \
  libboost-system-dev libboost-thread-dev libboost-test-dev \
  libconfig++-dev libsctp-dev libuhd-dev uhd-host iproute2 iputils-ping
```

使用一个明确的UHD安装来源。若已从源码装在/usr/local，不盲目叠加发行版libuhd；核对`which uhd_find_devices`、`uhd_config_info --version`和最终二进制ldd，避免工具和库版本混用。

```bash
lsusb -t
uhd_config_info --version
uhd_find_devices --args 'type=b200'
uhd_usrp_probe --args 'type=b200,serial=替换为主机B的序列号'
```

如果报告镜像缺失，查找该UHD安装配套的下载工具再执行：

```bash
command -v uhd_images_downloader || dpkg -L uhd-host | grep uhd_images_downloader
```

若只返回.py的绝对路径，使用该路径及其所需解释器执行，而不是照搬主机A的/usr/local路径。设备权限、USB 3、镜像和探测要求与[主机A检查](01-vmware-uhd-check.md)相同。

## 2. 克隆和构建

```bash
git clone --branch main https://github.com/huahuaLover/6g-pqaka-usrp.git
cd 6g-pqaka-usrp
./scripts/build-ue.sh
```

脚本先下载并校验固定liboqs，构建最小ML-KEM-512静态库，再编译UHD版srsUE，不需要sudo构建。内存充足时可通过`PQAKA_BUILD_JOBS`调整并行数。

产物：

```text
artifacts/srsue-uhd-build/srsue/src/srsue
```

必须看到CMake找到UHD；仅ENABLE_UHD=ON不足以证明已发现库。可检查：

```bash
grep -E 'ENABLE_UHD:|UHD_LIBRARIES:' artifacts/srsue-uhd-build/CMakeCache.txt
ldd artifacts/srsue-uhd-build/srsue/src/srsue | grep -E 'uhd|not found'
```

某些RF驱动以动态插件形式链接，ldd主程序未直接列出UHD时还需检查构建输出和RF插件；运行时应显示UHD可用并打开正确B210。先确认没有缺失库。

## 3. 填写本地私有配置

```bash
mkdir -p secrets/ue logs
chmod 700 secrets secrets/ue
cp config/ue-b210.conf.example config/ue-b210.local.conf
cp config/runtime.env.example secrets/ue/runtime.env
chmod 600 config/ue-b210.local.conf secrets/ue/runtime.env
```

通过私有方式接收：

- 主机A实际测试订阅的IMSI、K和OPc，及相应IMEI配置。
- 与UDM当前KeyID匹配的800字节HN公钥，保存为`secrets/ue/hn.pub`。
- 可信指纹和KeyID。当前实验KeyID为7；以主机A实际部署为准。

主机A的公钥位于：

```text
/home/huahua/pqaka-dev/secrets/pqaka-live/ue/hn.pub
```

这是私有部署目录，读取可能需要sudo。只传公钥和必要UE参数，不把整个secrets目录传到主机B，不重新生成另一对HN密钥。

编辑`config/ue-b210.local.conf`：替换B210序列号、K/OPc/IMSI/IMEI；确认`device_name=uhd`、PLMN相关订阅、小区频段/带宽匹配。初始tx_gain=0并非最终接入参数；需核对RF连线及调整时序/增益。

编辑`secrets/ue/runtime.env`，填入主机B上的绝对路径：

```text
PQAKA_UE_ENABLED=1
PQAKA_UE_PUBLIC_KEY_FILE=/实际仓库路径/secrets/ue/hn.pub
PQAKA_UE_PUBLIC_KEY_SHA256=来自可信配置的64位十六进制指纹
PQAKA_UE_KEY_ID=7
PQAKA_UE_GUTI_FILE=/实际仓库路径/secrets/ue/guti.bin
PQAKA_PROTOCOL_TRACE=1
```

用`sha256sum secrets/ue/hn.pub`核对可信指纹，不输出K/OPc。文件暂不存在的guti.bin由成功接入后的UE创建；不要复制另一台正在运行UE的GUTI来制造冲突。

模板保留n3/FDD、20MHz、15kHz、23.04Msps、106PRB、DNN internet和切片1/010203。无外部参考时使用internal。若使用外部10MHz参考，需要实际接线并同步修改两端clock配置；它与主机时间同步不同。

## 4. 启动与观察

先在主机A启动核心网及gNB，并确认NG Setup。然后主机B运行：

```bash
cd /实际仓库路径
sudo ./scripts/run-ue.sh
```

该脚本读取主机B私有配置，创建`pqaka-ue` namespace及本地logs目录，以前台方式启动UE。不要将整个UE进程放进该namespace运行；srsUE通过gw配置管理TUN所在namespace。

另开终端观察打点：

```bash
tail -F logs/ue.log | grep --line-buffered -F '[PQAKA-TRACE]'
```

成功标志：UHD打开B210、RRC连接、HNMAC验证成功、主机A AMF认证成功、Security Mode完成、PDU分配IP及RRC重配置成功。只看到UHD初始化不等于PQAKA通过。

## 5. 数据和身份路径检查

取得IP后设置UE namespace默认路由：

```bash
sudo ip netns exec pqaka-ue ip -br addr show tun_srsue
sudo ip netns exec pqaka-ue ip route replace default dev tun_srsue
sudo ip netns exec pqaka-ue ping -c 4 -W 3 10.101.200.1
sudo ip netns exec pqaka-ue ping -c 4 -W 3 1.1.1.1
```

公网失败时结合主机A出口/NAT定位。当前阶段只是功能验证，ping不解释为认证时延。

第一次无GUTI时应为path=1。有效GUTI复现需保留核心网映射并触发完整重新注册，普通Ctrl+C可能执行注销而使映射失效；原`restart-pqaka-ran.sh`绑定本机ZeroMQ和同机PID，不能直接在主机B使用。双机无线掉线/恢复脚本需在RF联通后单独补齐。在此之前，不能仅凭保存了GUTI就宣布path=2已验证。

停止：在UE前台终端按Ctrl+C；再按主机A文档停止gNB/核心网。关闭tail只结束查看。

## 6. 联调时需要回传的内容

操作系统/CPU、UHD版本、两台序列号、USB速度、RF参考和连线、CMake找到UHD的记录、构建错误或UE/gNB日志。不要把K/OPc、私钥、完整runtime.env或订阅配置粘贴到公开issue。

本仓库保留现有PQAKA协议代码；USRP时序参数、增益与实际RF联通仍需要联调。尚未将这套新构建/配置标记为USRP测试通过。
