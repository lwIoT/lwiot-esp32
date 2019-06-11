#
# ESP32 platform build file.
#

SET(PORT_SOURCE_FILES
	esp32adcchip.cpp
	esp32dacchip.cpp
	esp32gpiochip.cpp
	esp32i2calgorithm.cpp
	esp32primaryadc.cpp
	esp32pwmchannel.cpp
	esp32pwmtimer.cpp
	esp32secondaryadc.cpp
	esp32uart.cpp
	esp32watchdog.cpp
	esp32_ap.cpp
	esp32_sta.cpp
	esp32_wifi.c
	spiffs.cpp
	spibus.cpp
	esp32spi.c
	hardwarei2calgorithm.cpp

	soc/esp32.c
	soc/esp32system.cpp
	soc/i2chal.c

	${PROJECT_SOURCE_DIR}/freertos/rtos.c
	${PROJECT_SOURCE_DIR}/lwip/lwip.c
	${PROJECT_SOURCE_DIR}/mbedtls/mbedtls.c
)

add_library(lwiot-platform ${PORT_SOURCE_FILES})

INSTALL(TARGETS lwiot-platform
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/esp32/include/ DESTINATION include)
INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/esp32/config/${CMAKE_BUILD_TYPE}/ DESTINATION config)

include_directories(lwiot-platform PUBLIC ${PROJECT_SOURCE_DIR}/lwiot-core/include ${CMAKE_BINARY_DIR} ${PORT_INCLUDE_DIR})
