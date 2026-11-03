# 3. 修改后的srsUE如何通过Git交付

目标仓库：`https://github.com/huahuaLover/6g-pqaka-usrp.git`，分支`main`，由用户明确指定。

## 为什么不能只上传srsRAN_4G

当前NAS/USIM代码会引用外置的`modules/pqaka/cpp`及固定liboqs。只clone官方srsRAN_4G，或只推送它的修改，会缺失密码模块/链接依赖。另外，当前PQAKA修改尚有未提交文件，不能仅用`git archive HEAD`导出，否则会丢失实际实现。

采用独立交付仓库：复制当前独立UE工作树的源码快照、保留上游许可证；外置PQ模块一并纳入。交付副本将CMake中的本机默认绝对根目录改为仓库相对根目录，构建脚本显式指定PQAKA_ROOT；不修改正在运行的原开发副本。

```text
6g-pqaka-usrp/
  sources/srsRAN_4G/       修改后的完整源码快照，不嵌套.git
  modules/pqaka/cpp/       六组运行模块的.h/.cc
  scripts/build-ue.sh      固定liboqs并启用UHD构建
  scripts/run-ue.sh        读取主机B私有配置，前台运行UE
  config/*.example        无真实订阅参数的模板
  docs/                   双机操作说明
  VERSIONS.md             上游基准与依赖版本
  README.md
  .gitignore
```

liboqs由构建脚本从官方仓库下载0.12.0并核对提交`f4b96220e4bd208895172acc4fedb5a191d9f5b1`，不把本机编译产物或另一份嵌套Git工作树上传。

## 交付范围

包含UE的PQAKA密码、NAS/SUCI/GUTI、响应/恢复、消息编解码和调试打点。不包含整个free5GC、不包含Go核心网模块，也不需要主机B运行Docker核心网。

不上传：

- 当前测试用户真实K/OPc、私有UE配置、实际GUTI文件。
- HN KEM私钥、NF服务凭据、数据库/备份、cache/network.env。
- 运行日志、packet capture、artifacts及CMake缓存。

模板只有占位符，不能直接运行。主机B需要私下获得与主机A订阅一致的K/OPc、IMSI和匹配HN公钥及可信指纹。UE不需要HN私钥。公钥虽不属于秘密，也必须通过可信配置确认其来源，不能只对未知下载公钥自行算指纹就当作可信。

## 本机交付副本与推送

独立副本位于：

```text
/home/huahua/pqaka-dev/delivery/6g-pqaka-usrp
```

先完成源文件对照、模板秘密检查和脚本语法检查，再在该目录提交和推送；不能在原free5GC仓库直接`git add .`。

```bash
cd /home/huahua/pqaka-dev/delivery/6g-pqaka-usrp
git status --short
git diff --cached --stat
git push -u origin main
```

实际提交/推送结果单独记录在本目录的`delivery-status.md`，文档中的命令不代表已经成功推送。远端写权限或认证失败时，先保留可审阅的本地提交；不要更换成别人的账号或覆盖远端分支。

主机B克隆：

```bash
git clone --branch main https://github.com/huahuaLover/6g-pqaka-usrp.git
cd 6g-pqaka-usrp
```

私有仓库需用户自己的GitHub认证。克隆后按[主机B操作](04-host-b-ue.md)构建，不直接复制主机A的二进制或运行环境。

源码审查说明：保留了上游未修改的公开USIM测试向量文件，其中存在与既有实验订阅重合的固定测试值；这不是私有部署配置，不能将这些公开测试值当作生产秘密。本仓库不交付实际运行配置、HN私钥或NF凭据。
