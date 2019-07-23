####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
#
####
if (${CMAKE_SYSTEM_NAME} STREQUAL "arm-linux-gnueabihf")
	set(SOURCE_FILES
		"${CMAKE_CURRENT_LIST_DIR}/BME280ComponentAi.xml"
		"${CMAKE_CURRENT_LIST_DIR}/BME280ComponentImpl.cpp"
	)
else()
	set(SOURCE_FILES
		"${CMAKE_CURRENT_LIST_DIR}/BME280ComponentAi.xml"
		"${CMAKE_CURRENT_LIST_DIR}/BME280ComponentImplStub.cpp"
	)
endif()

register_fprime_module()
