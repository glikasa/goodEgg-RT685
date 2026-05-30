#include the required sub directories
add_subdirectory(drivers)

#add subdirectories to compiler's include search path
zephyr_include_directories(include)
