# main list
cmake_minimum_required(VERSION 2.8.4)

PROJECT(GCD)

# set GNU compiler parameters
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion -std=c++0x -DQPU_MODE")

# for buidling object file by source file from Lib directory
set(QPULIB_DIR "/home/pi/Downloads/QPULib/Lib")

# output binary directory (only 1 directory for a project)
#set(BIN_OBJ_DIR "/home/pi/Downloads/QPULib/Tests/build/OBJ_DIR")

include_directories( ${QPULIB_DIR}  )

#add_subdirectory( ${QPULIB_DIR} 		${BIN_OBJ_DIR} )
#add_subdirectory( ${QPULIB_DIR}/Source 		${BIN_OBJ_DIR}/Source )
#add_subdirectory( ${QPULIB_DIR}/Target 		${BIN_OBJ_DIR}/Target )
#add_subdirectory( ${QPULIB_DIR}/VideoCore 	${BIN_OBJ_DIR}/VideoCore )

# add QPU OBJECT lib
add_library(qpuObjLib OBJECT 
${QPULIB_DIR}/Kernel.cpp
${QPULIB_DIR}/Source/Syntax.cpp
${QPULIB_DIR}/Source/Int.cpp
${QPULIB_DIR}/Source/Float.cpp
${QPULIB_DIR}/Source/Stmt.cpp
${QPULIB_DIR}/Source/Pretty.cpp
${QPULIB_DIR}/Source/Translate.cpp
${QPULIB_DIR}/Source/Interpreter.cpp
${QPULIB_DIR}/Source/Gen.cpp
${QPULIB_DIR}/Target/Syntax.cpp
${QPULIB_DIR}/Target/SmallLiteral.cpp
${QPULIB_DIR}/Target/Pretty.cpp
${QPULIB_DIR}/Target/RemoveLabels.cpp
${QPULIB_DIR}/Target/CFG.cpp
${QPULIB_DIR}/Target/Liveness.cpp
${QPULIB_DIR}/Target/RegAlloc.cpp
${QPULIB_DIR}/Target/ReachingDefs.cpp
${QPULIB_DIR}/Target/Subst.cpp
${QPULIB_DIR}/Target/LiveRangeSplit.cpp
${QPULIB_DIR}/Target/Satisfy.cpp
${QPULIB_DIR}/Target/LoadStore.cpp
${QPULIB_DIR}/Target/Emulator.cpp
${QPULIB_DIR}/Target/Encode.cpp
${QPULIB_DIR}/VideoCore/Mailbox.cpp
${QPULIB_DIR}/VideoCore/Invoke.cpp
${QPULIB_DIR}/VideoCore/VideoCore.cpp)

# add exe
add_executable( myGcd GCD.cpp $<TARGET_OBJECTS:qpuObjLib> )

#add_library(gcd GCD.cpp 	$<TARGET_OBJECTS:${BIN_OBJ_DIR}/Kernel> )
#				$<TARGET_OBJECTS:${BIN_OBJ_DIR}/Source> 
#				$<TARGET_OBJECTS:${BIN_OBJ_DIR}/Target>
#				$<TARGET_OBJECTS:${BIN_OBJ_DIR}/VideoCore>)
#