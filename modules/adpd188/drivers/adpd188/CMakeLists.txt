# drivers/sensor/adpd188/CMakeLists.txt

zephyr_library()
zephyr_library_sources(adpd188bi.c adpd188.c)

# 2. Add include paths for internal headers (like adpd188.h)
zephyr_library_include_directories(include)

