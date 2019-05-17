####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding diles
# MOD_DEPS: (optional) module dependencies
#
####
if (${PLATFORM} STREQUAL "RASPIAN")
	set(SOURCE_FILES
		"${CMAKE_CURRENT_LIST_DIR}/BME280ComponentAi.xml"
		"${CMAKE_CURRENT_LIST_DIR}/BME280ComponentImpl.cpp"
	)
else()
	set(SOURCE_FILES
		"${CMAKE_CURRENT_LIST_DIR}/BME280ComponentAi.xml"
		"${CMAKE_CURRENT_LIST_DIR}/BME280ComponentImpl_DEFAULT.cpp"
	)
endif()

register_fprime_module()
