# 基础版本

srsRAN_4G baseline: eea87b1d893ae58e0b08bc381730c502024ae71f (release_23_11).
Source snapshot includes current uncommitted PQAKA changes from the independent development copy.
CMake defaults in this delivery resolve PQAKA_ROOT relative to the repository; crypto/NAS code is copied unchanged.
liboqs: 0.12.0, commit f4b96220e4bd208895172acc4fedb5a191d9f5b1, ML-KEM-512 only, OpenSSL disabled.
Original core/gNB host: Ubuntu 20.04.6, UHD 3.15.0.HEAD-0-gaea0e2de.
USRP RF operation and a fresh remote-host build are not yet validated.
