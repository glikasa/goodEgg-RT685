# drivers/sensor/adpd188/CMakeLists.txt

zephyr_library()
zephyr_library_sources(tas2563_driver.c tasdevice.c tas2563_ftc.c)

# 2. Add include paths for internal headers (like adpd188.h)
zephyr_library_include_directories(include)

# If the user selects the TAS2563_I2C_MODE Kconfig option (which defines
# the C symbol CONFIG_TAS2563_I2C_MODE), define the macro that the library expects: I2C_ENABLE.
# if(CONFIG_TAS2563_I2C_MODE)
#     target_compile_definitions(tas2563 PRIVATE I2C_ENABLE)
# endif()
#
# if(CONFIG_TAS2563_RX_16BIT)
#     target_compile_definitions(tas2563 PRIVATE RX_16BIT)
# endif()
#
# if(CONFIG_TAS2563_PDM_ENABLE)
# 	target_compile_definitions(tas2563 PRIVATE PDM_ENABLE)
# endif()

zephyr_compile_definitions_ifdef(CONFIG_TAS2563_I2C_ENABLE I2C_ENABLE)
