cmake_minimum_required(VERSION 3.4)
project(attiny85_joycon C)

set(CMAKE_C_STANDARD 99)

set(AVR_INCLUDE /usr/lib/avr/include)
set(DIGI_AVR_ROOT ../DigistumpArduino/digistump-avr)
set(DIGI_AVR_CORE ${DIGI_AVR_ROOT}/cores/tiny)
include_directories(${AVR_INCLUDE} ${DIGI_AVR_CORE} usb)

enable_language(ASM)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})

add_compile_options(-mmcu=attiny85)
add_compile_options(-DF_CPU=16500000)
add_compile_options(-Os)

aux_source_directory(${DIGI_AVR_CORE} AVR_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR}/usb USB_SRCS)
aux_source_directory(${CMAKE_SOURCE_DIR} SRCS)
list(APPEND SRCS ${AVR_SRCS} ${USB_SRCS} usb/usbdrvasm.S)
#set(SRCS ${USB_SRCS} ${AVR_SRCS} usb/usbdrvasm.S)

add_executable(${PROJECT_NAME} ${SRCS})