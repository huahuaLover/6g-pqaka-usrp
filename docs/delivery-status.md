# UE Git交付状态

日期：2026-11-03。目标为 https://github.com/huahuaLover/6g-pqaka-usrp.git 的main分支。

## 已完成

- 独立交付副本：`/home/huahua/pqaka-dev/delivery/6g-pqaka-usrp`。
- 修改后的完整UE源码、12个PQ运行模块文件、可移植CMake根目录、UHD构建脚本、前台运行脚本和无部署凭据模板。
- 源码提交已创建；首个提交为`5e20a39`，后续文档状态提交以本地`git log -1`为准。
- 交付源码审查通过；与当前实验值重合的固定测试向量只位于逐字节未修改的上游usim_test.cc，作为公开测试向量保留。未加入部署secrets、NF凭据、运行日志或私有配置。
- Shell语法、UHD CMake配置、固定liboqs和完整UHD UE构建通过。
- 新二进制`--help`运行通过，CMake确认ENABLE_UHD=ON且找到libuhd；不等于实际射频接入通过。
- 22个修改/PQ运行源文件的SHA-256清单可通过仓库根目录的`sha256sum -c SOURCE-MANIFEST.sha256`核验。

本机构建产物位于`artifacts/srsue-uhd-build`，被Git忽略；主机B必须自行编译。构建日志保存在原开发目录的`artifacts/usrp-delivery-build.log`。

## 尚未完成

- GitHub远端推送：本机HTTPS写入检查报告`could not read Username for https://github.com`，没有可用登录凭据。
- B210实际探测/USB直通/收发、主机B构建及双机PQAKA RF接入。

请在本机配置GitHub写入认证后执行：

```bash
cd /home/huahua/pqaka-dev/delivery/6g-pqaka-usrp
git push -u origin main
```

不要在聊天、文档或远端URL中填写Token。提交用户名和邮箱不等于GitHub登录授权；当前尚不能让主机B从远端下载这些修改。

现有ZeroMQ代码、配置、服务和原标准基线本次未更改。Git源码位于独立交付目录，后续开发修改需要同步并重新验证，不会自动与原开发副本双向同步。
