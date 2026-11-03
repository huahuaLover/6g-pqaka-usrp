# 6G-PQAKA srsUE — USRP delivery

当前官方srsRAN_4G独立开发副本的PQAKA源码交付。用于软件UE/B210功能联调，尚未完成USRP RF验证。主机A运行既有free5GC/gNB，主机B使用本仓库构建UE。

- [双机操作总览](docs/README.md)
- [主机B下载、构建、配置和运行](docs/04-host-b-ue.md)
- [源码范围、秘密隔离和Git交付](docs/03-git-delivery.md)
- [基础版本](VERSIONS.md)

```bash
./scripts/build-ue.sh
```

先安装依赖。脚本会下载并核对liboqs固定版本，随后构建UHD版UE。填好config与secrets中的本地主机配置后运行：

```bash
sudo ./scripts/run-ue.sh
```

config下example为占位模板；没有内置实际K/OPc或HN公钥。私有参数须与主机A订阅一致。原上游COPYRIGHT和LICENSE保留在sources/srsRAN_4G中。

当前已验证的ZeroMQ功能与本仓库新增UHD构建/RF配置的验证范围不同，请查看交付状态；成功编译不等于射频接入成功。

源码审查说明：保留了上游未修改的公开USIM测试向量文件，其中存在与既有实验订阅重合的固定测试值；这不是私有部署配置，不能将这些公开测试值当作生产秘密。本仓库不交付实际运行配置、HN私钥或NF凭据。
