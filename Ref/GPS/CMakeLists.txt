####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
#
####
if (${CMAKE_SYSTEM_NAME} STREQUAL "arm-linux-gnueabihf")
	set(SOURCE_FILES
		"${CMAKE_CURRENT_LIST_DIR}/GPSComponentAi.xml"
		"${CMAKE_CURRENT_LIST_DIR}/GPSComponentImpl.cpp"
	)
else()
	set(SOURCE_FILES
		"${CMAKE_CURRENT_LIST_DIR}/GPSComponentAi.xml"
		"${CMAKE_CURRENT_LIST_DIR}/GPSComponentImplStub.cpp"
	)
endif()

register_fprime_module()
