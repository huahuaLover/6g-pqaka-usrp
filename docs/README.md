# 双主机B210：环境检查、Git交付和启动指南

日期：2026-11-03。目标仅为6G-PQAKA在USRP上完成认证和数据连接，性能测试暂缓。

## 四个问题的直接回答

| 问题 | 结论／入口 |
| --- | --- |
| VMware是否已有USRP环境？ | Ubuntu内UHD、B210镜像和gNB的UHD链接已具备，但当前未识别B210，不能判定实际收发可用。见[环境检查](01-vmware-uhd-check.md)。 |
| 本机怎样启动free5GC和gNB？ | 使用独立核心网Compose，只启动核心网；gNB加载新UHD配置并在终端前台运行，不再使用自动拉起本地ZeroMQ UE的start命令。见[主机A操作](02-host-a-core-gnb.md)。 |
| srsUE如何上传Git？ | 用独立交付仓库包含修改后的srsUE、外置PQ模块、固定liboqs来源和构建说明；密钥/订阅参数通过私有渠道交付，不上传现有开发目录。见[Git交付方案](03-git-delivery.md)。 |
| UE主机下载后怎么做？ | 安装UHD/编译依赖，构建固定liboqs及UHD版PQ srsUE，配置公钥与软件USIM，建立namespace后前台启动UE。见[主机B操作](04-host-b-ue.md)。 |

## 拓扑和顺序

```text
主机A：当前VMware中的Ubuntu
  free5GC ←→ srsRAN Project gNB ←UHD/USB3→ B210-A
                                               ↕ RF
主机B：Linux + PQAKA srsUE       ←UHD/USB3→ B210-B
```

按“主机A设备识别→主机B设备识别→私有配置交付→构建→核心网→gNB→UE→数据检查”的顺序执行。管理以太网只用于Git/配置/日志，不替代RF认证或传输I/Q。

两个RF端口间的有线连接必须包含合适衰减，B210端口、双向路径和增益尚需按实际连线确认；不能把ZeroMQ的增益直接当成有线RF安全设置。B210官方手册规定输入不超过-15dBm、loopback至少30dB衰减；还要结合实际功率预算。[官方B2x0手册](https://files.ettus.com/manual/page_usrp_b200.html)

## 当前完成与未完成

- 已完成：本机只读检查、四项操作文档，核对当前源码和构建依赖。
- 未完成：B210实际探测/收发、Git仓库导出/提交/推送、主机B构建、双机USRP接入。
- 文中仓库地址、序列号及私有订阅占位符必须替换；命令是操作步骤，不代表已经执行。
- 已指定远端`https://github.com/huahuaLover/6g-pqaka-usrp.git`和`main`分支，提交/推送及构建结果见[交付状态](delivery-status.md)。原ZeroMQ源码、配置及服务本次未更改。

官方srsRAN文档提供srsUE/gNB双B210接入示例，但示例核心网为Open5GS，当前使用free5GC；本机gNB为24.04，不能直接照抄最新版的AMF配置层级。[官方接入说明](https://docs.srsran.com/projects/project/en/latest/tutorials/source/srsUE/source/index.html)
