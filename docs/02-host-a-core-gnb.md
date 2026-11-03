# 2. 主机A：只运行free5GC和B210 gNB

前置：完成[设备检查](01-vmware-uhd-check.md)，RF连线/衰减与增益已经确认，另一台主机准备运行UE。以下步骤尚未实机RF验证，不修改当前ZeroMQ配置。

## A. 停止本机旧无线模式

```bash
cd /home/huahua/pqaka-dev
sudo ./scripts/run-pqaka.sh stop
```

若原标准环境也在运行，先使用其原脚本停止。之后不要执行`run-pqaka.sh start`启动USRP：它仍会启动本机ZeroMQ gNB和UE。

## B. 生成独立gNB配置

先确认主机A序列号：

```bash
export GNB_USRP_SERIAL='替换为主机A的B210序列号'
```

在已安装Python PyYAML的环境执行以下转换；缺少时先安装`python3-yaml`。只改副本的RF后端，其余小区、切片、QoS及核心网地址沿用已验证配置：

```bash
python3 - <<'PY'
from pathlib import Path
import os
import yaml
root = Path('/home/huahua/pqaka-dev')
target = root / 'config/srsran/gnb-b210.local.yml'
if target.exists():
    raise SystemExit('目标已存在，请检查原文件，脚本不会覆盖。')
serial = os.environ['GNB_USRP_SERIAL']
if not serial.isalnum():
    raise SystemExit('请填写实际B210序列号。')
cfg = yaml.safe_load((root / 'config/srsran/gnb-zmq.yml').read_text())
cfg['ru_sdr'].update(device_driver='uhd',
    device_args='type=b200,serial=' + serial,
    clock='internal', tx_gain=0, rx_gain=40)
target.write_text(yaml.safe_dump(cfg, sort_keys=False))
print(target)
PY
```

这里tx_gain=0只是低发射增益起点，不保证任意衰减链路均能接入。实际增益按连线/功率预算调整；没有共同外部参考时使用internal，不照抄external。

继续沿用n3/FDD、20MHz、15kHz、23.04Msps、PLMN 20893、TAC 1、SST 1/SD 010203。若后续调整带宽/采样率，UE和gNB必须一起调整。

## C. 只启动独立PQAKA核心网

```bash
cd /home/huahua/pqaka-dev
sudo env PQAKA_PROTOCOL_TRACE=1 docker-compose --ansi never \
  -p pqaka-dev -f compose.yaml -f compose.pqaka.yaml up -d
```

该Compose文件仅包含核心网服务，不包含UE或gNB。按实际角色挂载既有PQAKA秘密；不重新生成HN密钥，不将主机B配置混入核心网。

查看状态与各SBI监听：

```bash
sudo docker-compose --ansi never -p pqaka-dev \
  -f compose.yaml -f compose.pqaka.yaml ps

for nf in nrf amf ausf udm udr nssf pcf smf; do
  sudo docker exec "pqaka-$nf" sh -c \
    'grep -q ":1F40 .* 0A " /proc/net/tcp /proc/net/tcp6' \
    && echo "$nf: SBI listening" || echo "$nf: NOT READY"
done
```

若仍有NOT READY，先看对应容器日志，等待或处理问题，不直接启动UE。监听检查不替代后续NG Setup/实际认证。

核心网日志单独开终端：

```bash
cd /home/huahua/pqaka-dev
sudo docker-compose --ansi never -p pqaka-dev \
  -f compose.yaml -f compose.pqaka.yaml logs --no-color --tail=100 -f
```

这里直接跟随Compose日志，不依赖旧ZeroMQ启动脚本创建的core.log采集进程。

## D. 前台启动gNB

另开终端执行：

```bash
cd /home/huahua/pqaka-dev
sudo /home/huahua/Documents/srsRAN_Project/srsRAN_Project/build/apps/gnb/gnb \
  -c config/srsran/gnb-b210.local.yml
```

应看到B210初始化、实际USB 3连接、采样率配置和NG Setup成功。再启动主机B的UE。

AMF仍位于10.101.200.16；gNB本地N2/N3地址仍为10.101.200.1，UPF为10.101.200.2。因为gNB和核心网都在主机A，不能把AMF地址改成主机B的管理IP。

主机A不启动srsUE、不创建UE namespace；主机B通过RF接入，不需要直接访问AMF容器的Docker私网。

## E. 停止与验收

- gNB前台终端按Ctrl+C停止实际gNB。
- 核心网停止命令：

```bash
cd /home/huahua/pqaka-dev
sudo docker-compose --ansi never -p pqaka-dev \
  -f compose.yaml -f compose.pqaka.yaml stop
```

手工启动的gNB没有写入旧run.sh的PID文件，因此不要以为旧`run-pqaka.sh stop`必然管理它。停止日志跟随也不会停止核心网。

最终验收须覆盖主机B的path=1、有效path=2、失效恢复、NAS/AS安全、PDU及经UPF的数据连接。保留ZeroMQ配置以便回退；USRP未完成时不标记RF验证通过。
