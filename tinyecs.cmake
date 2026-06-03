function(init_target name)
	if(MSVC)
		target_compile_options(${name} PRIVATE /w) # i dont care about what microsoft has to say
	endif()

	if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		target_compile_options(${name} PRIVATE
			-Weverything
			-Wno-c++98-c++11-c++14-c++17-compat
			-Wno-c++98-c++11-c++14-c++17-compat-pedantic
			-Wno-c++98-compat-pedantic
			-Wno-c++11-compat-pedantic
			-Wno-c++14-compat-pedantic
			-Wno-c++17-compat-pedantic
			-Wno-c++98-compat
			-Wno-c++11-compat
			-Wno-c++14-compat
			-Wno-c++17-compat
			-Wno-c++20-compat
			-Wno-c99-compat

			-Wno-unsafe-buffer-usage # it's stupid
			-Wno-padded              # not super useful

			-fdiagnostics-show-template-tree
			-fdiagnostics-show-option
			-fdiagnostics-show-category=name
			-fvisibility=hidden
		)
	endif()
	target_compile_definitions(${name} PRIVATE $<$<NOT:$<CONFIG:Debug>>:NDEBUG>)
	set_property(TARGET ${name} PROPERTY CXX_STANDARD 20)
endfunction()
