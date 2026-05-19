set(CMAKE_BUILD_RPATH ${CMAKE_BUILD_RPATH}:/home/kweronsx/dpcpp/build/lib)
set(gKernTimelineOutDir ${CMAKE_BINARY_DIR}/kernTimelines)

add_custom_target(cleanTimelines COMMAND find ${gKernTimelineOutDir}
		-name \"*.json\" -type f | xargs rm -fv)

file(MAKE_DIRECTORY ${gKernTimelineOutDir})

add_custom_target(ztKernTimelines) # ze_tracer timelines
add_custom_target(utKernTimelines) # unitrace timelines

function(addKernelTest binaryFile sourceFile timelinePrefix)
	set(kernTimelineOutDir ${gKernTimelineOutDir}/${binaryFile})
	file(MAKE_DIRECTORY ${kernTimelineOutDir})

	add_executable(${binaryFile} ${sourceFile})

	if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		target_compile_options(${binaryFile} PRIVATE -fsycl -O0 -g)
	else()
		target_compile_options(${binaryFile} PRIVATE -fsycl -O3)
	endif()

	target_link_options(${binaryFile} PRIVATE -fsycl)

	add_custom_target(${binaryFile}TimelineZT
										 COMMAND LD_LIBRARY_PATH=${CMAKE_BUILD_RPATH}
										 UR_L0_V2_DISABLE_ZE_LAUNCH_KERNEL_WITH_ARGS=1 ze_tracer
										 --chrome-kernel-timeline ${CMAKE_CURRENT_BINARY_DIR}/${binaryFile}
										 WORKING_DIRECTORY ${kernTimelineOutDir}
										 DEPENDS ${binaryFile}
										 COMMENT "Generating ze_tracer timeline file for ${binaryFile}")

	add_dependencies(ztKernTimelines "${binaryFile}TimelineZT")

	add_custom_target(${binaryFile}TimelineUT
										 COMMAND LD_LIBRARY_PATH=${CMAKE_BUILD_RPATH} unitrace
										 --chrome-kernel-logging ${CMAKE_CURRENT_BINARY_DIR}/${binaryFile}
										 WORKING_DIRECTORY ${kernTimelineOutDir}
										 DEPENDS ${binaryFile}
										 COMMENT "Generating unitrace timeline file for ${binaryFile}")

	add_dependencies(utKernTimelines "${binaryFile}TimelineUT")
endfunction()

add_custom_target(reKernTimelines
		DEPENDS cleanTimelines
		DEPENDS utKernTimelines
		DEPENDS ztKernTimelines)

addKernelTest(k2q2b k2q2b.cpp k2q2b) # 2 kernels, 2 queues, uses sycl buffers
addKernelTest(k2q2p k2q2p.cpp k2q2p) # 2 kernels, 2 queues, uses USM pointers
addKernelTest(k3q3b k3q3b.cpp k3q3b) # 3 kernels, 3 queues, uses sycl buffers
addKernelTest(k3q3p k3q3p.cpp k3q3p) # 3 kernels, 3 queues, uses USM pointers
