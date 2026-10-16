file(GLOB COFFEE_COMMON_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/common/application/*.c")
file(GLOB COFFEE_SOURCES CONFIGURE_DEPENDS Src/*.c)
file(GLOB COFFEE_HEADERS CONFIGURE_DEPENDS Hdr/*.h)

add_executable(DemoApp_CoffeeMachine ${COFFEE_COMMON_SOURCES} ${COFFEE_SOURCES} ${COFFEE_HEADERS})
target_include_directories(DemoApp_CoffeeMachine PRIVATE Hdr "${CMAKE_SOURCE_DIR}/common/application")
target_link_libraries(DemoApp_CoffeeMachine PRIVATE eve_hal)
set_property(TARGET DemoApp_CoffeeMachine PROPERTY C_STANDARD 11)

option(COFFEE_MACHINE_DEMO "Enable the simulated coffee controller" ON)
if(COFFEE_MACHINE_DEMO)
  target_compile_definitions(DemoApp_CoffeeMachine PRIVATE COFFEE_MACHINE_DEMO=1)
endif()
if(MSVC)
  target_compile_options(DemoApp_CoffeeMachine PRIVATE /utf-8)
  target_link_libraries(DemoApp_CoffeeMachine PRIVATE user32 gdi32)
endif()
if(IS_PLATFORM_RP2040)
  pico_enable_stdio_uart(DemoApp_CoffeeMachine 1)
  pico_enable_stdio_usb(DemoApp_CoffeeMachine 1)
  pico_add_extra_outputs(DemoApp_CoffeeMachine)
endif()

set(COFFEE_DEPLOY "${CMAKE_BINARY_DIR}/deploy/DemoApp_CoffeeMachine")
add_custom_command(TARGET DemoApp_CoffeeMachine POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${COFFEE_DEPLOY}"
  COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:DemoApp_CoffeeMachine>" "${COFFEE_DEPLOY}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/Test/Flash/BT81X_Flash.bin" "${COFFEE_DEPLOY}/__Flash.bin")
if(IS_PLATFORM_RP2040)
  add_custom_command(TARGET DemoApp_CoffeeMachine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE_DIR:DemoApp_CoffeeMachine>/DemoApp_CoffeeMachine.uf2"
      "${COFFEE_DEPLOY}/DemoApp_CoffeeMachine.uf2")
endif()
if(WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(COFFEE_ARCH x64)
  else()
    set(COFFEE_ARCH x86)
  endif()
  file(GLOB COFFEE_DLLS
    "${CMAKE_SOURCE_DIR}/common/eve_hal/Bin/MSVC/${COFFEE_ARCH}/*.dll"
    "${CMAKE_SOURCE_DIR}/common/eve_hal/Bin/Simulation/${COFFEE_ARCH}/*.dll")
  foreach(COFFEE_DLL IN LISTS COFFEE_DLLS)
    add_custom_command(TARGET DemoApp_CoffeeMachine POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${COFFEE_DLL}" "${COFFEE_DEPLOY}")
  endforeach()
  set_property(TARGET DemoApp_CoffeeMachine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${COFFEE_DEPLOY}")
endif()
