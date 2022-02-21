cmake_minimum_required(VERSION 3.5)

project(HikvionProjectV2)

#add_subdirectory ("PPDET_BASE")


set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    message("Build Release")
    set(MEG_VERSION host_with_cuda_release)

else()
    set(MEG_VERSION host)
endif()



set(MEGE_INCLUDE_DIR D:/d/programs/MegEngine/build_dir/${MEG_VERSION}/install/include)
set(MEGE_LIB_DIR D:/d/programs/MegEngine/build_dir/${MEG_VERSION}/install/lib)
set(OPENCV_INCLUDE_DIRS D:/Applications/opencv/opencv_4.3.0/opencv/build/include)
set(OPENCV_LIB_DIR D:/Applications/opencv/opencv_4.3.0/opencv/build/x64/vc15/lib)
set(CUDA_ROOT_PATH C:/Program\ Files/NVIDIA\ GPU\ Computing\ Toolkit/CUDA/v10.1)
set(CUDA_LIB_DIR ${CUDA_ROOT_PATH}/lib/x64)
set(CUDA_INCLUDE_DIR ${CUDA_ROOT_PATH}/include)
set(CUDNN_ROOT_PATH ${CUDA_ROOT_PATH}/cudnn-7.6.5.32)
set(CUDNN_LIB_DIR ${CUDNN_ROOT_PATH}/lib/x64)
set(CUDNN_INCLUDE_DIR ${CUDNN_ROOT_PATH}/include)
set(TRT_ROOT_PATH ${CUDA_ROOT_PATH}/TensorRT-6.0.1.5)
set(TRT_LIB_DIR ${TRT_ROOT_PATH}/lib)
set(TRT_INCLUDE_DIR ${TRT_ROOT_PATH}/include)

set(HKVS_LIBS_PATH D:\\programs\\HKVSlib\\lib)
set(HKVS_INCLUDE_PATH D:\\programs\\HKVSlib\\include)


if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(OpenCV_LIBS opencv_world452)

else()
    message("BUILD DEBUG")
    set(OpenCV_LIBS opencv_world452d)
endif()



include(PPDET_BASE/cmake/yaml-cpp.cmake)

SET(PADDLE_DIR D:\\programs\\PP\\paddle_inference)
SET(PADDLE_LIB_NAME "libpaddle_inference")
#SET(OPENCV_DIR D:\\programs\\PP\\opencv)
SET(CUDA_LIB C:\\Program\ Files\\NVIDIA\ GPU\ Computing\ Toolkit\\CUDA\\v10.1\\lib\\x64 )
SET(CUDNN_LIB C:\\Program\ Files\\NVIDIA\ GPU\ Computing\ Toolkit\\CUDA\\v10.1\\cudnn-7.6.5.32\\lib\\x64)
SET(TENSORRT_INC_DIR C:\\Program\ Files\\NVIDIA\ GPU\ Computing\ Toolkit\\CUDA\\v10.1\\TensorRT-6.0.1.5\\include)
SET(TENSORRT_LIB_DIR C:\\Program\ Files\\NVIDIA\ GPU\ Computing\ Toolkit\\CUDA\\v10.1\\TensorRT-6.0.1.5\\lib)

#SET(SRCS
##        ppdet_base.cpp
#        PPDET_BASE/utils.cc
#        PPDET_BASE/preprocess_op.cc
#        PPDET_BASE/picodet_postprocess.cc
#        PPDET_BASE/object_detector.cc
#)

message("++++++++++",${CMAKE_BINARY_DIR})

link_directories(
#        "${PADDLE_DIR}/paddle/lib",
#        "${CMAKE_BINARY_DIR}/ext/yaml-cpp/lib"
#        "${OPENCV_DIR}/build/x64/vc15/lib"
#        "${PADDLE_DIR}/third_party/install/glog/lib"
#        "${PADDLE_DIR}/third_party/install/protobuf/lib"
)
#link_directories("${PADDLE_DIR}/third_party/install/gflags/lib")
#link_directories("${PADDLE_DIR}/third_party/install/xxhash/lib")
link_directories("${CMAKE_CURRENT_BINARY_DIR}")

include_directories(
#        "PPDET_BASE",
#        "${PADDLE_DIR}/paddle/include",
#        "${CMAKE_BINARY_DIR}/ext/yaml-cpp/src/ext-yaml-cpp/include",
#        "${OPENCV_DIR}/build/include",
#        "${PADDLE_DIR}/third_party/install/glog/include"
)
macro(safe_set_static_flag)
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)
endmacro()

add_definitions("/DGOOGLE_GLOG_DLL_DECL=")
set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} /bigobj /MTd")
set(CMAKE_C_FLAGS_RELEASE  "${CMAKE_C_FLAGS_RELEASE} /bigobj /MT")
set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} /bigobj /MTd")
set(CMAKE_CXX_FLAGS_RELEASE   "${CMAKE_CXX_FLAGS_RELEASE} /bigobj /MT")



find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets QuickWidgets REQUIRED)


include_directories("./include")
include_directories(${CUDA_INCLUDE_DIR} ${CUDNN_INCLUDE_DIR} ${TRT_INCLUDE_DIR} ${MEGE_INCLUDE_DIR} ${OPENCV_INCLUDE_DIRS})

link_directories(${MEGE_LIB_DIR} ${OPENCV_LIB_DIR} ${HKVS_LIBS_PATH})

set(PROJECT_SOURCES
        qml.qrc
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
#        detector.cpp
#        yolox_c.cpp
        hikvisonhandlercstyelfuncs.cpp
        hikvisonhandler.cpp
        hikvisonviewer.cpp
        imageviewer.cpp
#        inferencethread.cpp
        imageviwerv2.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(HikvionProjectV2
        ${PROJECT_SOURCES}
        ${SRCS}
    )
else()
    if(ANDROID)
        add_library(HikvionProjectV2 SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(HikvionProjectV2
            ${PROJECT_SOURCES}

        )
    endif()
endif()



target_link_libraries(HikvionProjectV2 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::QuickWidgets opencv_world452d.lib HCNetSDK.lib PlayCtrl.lib )
#target_link_libraries(HikvionProjectV2 PRIVATE paddle_inference.lib libyaml-cppmt.lib glog.lib)
#target_link_libraries(HikisonProjectV2 PRIVATE megengine);
#target_compile_definitions(HikvionProjectV2 PRIVATE MGE_DLL_IMPORT_DATA)
