cmake_minimum_required(VERSION 3.5)

project(HikvionProjectV2)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


MESSAGE(STATUS "operation system is ${CMAKE_SYSTEM}")


IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    MESSAGE(STATUS "current platform: Linux ")
    set(HKVS_ROOT_DIR Hikvison)
    set(OpenCV_ROOT_DIR /home/siky/Applications/opencv/build/install)

    if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
        set(OpenCV_LIBS opencv_core opencv_imgproc opencv_highgui opencv_imgcodecs opencv_objdetect)

    else()
        message("BUILD DEBUG")
        set(OpenCV_LIBS opencv_core opencv_imgproc opencv_highgui opencv_imgcodecs opencv_objdetect)
    endif()



ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    MESSAGE(STATUS "current platform: Windows")

    add_definitions("/DGOOGLE_GLOG_DLL_DECL=")
    set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} /bigobj /MTd")
    set(CMAKE_C_FLAGS_RELEASE  "${CMAKE_C_FLAGS_RELEASE} /bigobj /MT")
    set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} /bigobj /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE   "${CMAKE_CXX_FLAGS_RELEASE} /bigobj /MT")


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

    SET(PADDLE_DIR D:\\programs\\PP\\paddle_inference)
    SET(PADDLE_LIB_NAME "libpaddle_inference")
    #SET(OPENCV_DIR D:\\programs\\PP\\opencv)
    SET(CUDA_LIB C:\\Program\ Files\\NVIDIA\ GPU\ Computing\ Toolkit\\CUDA\\v10.1\\lib\\x64 )
    SET(CUDNN_LIB C:\\Program\ Files\\NVIDIA\ GPU\ Computing\ Toolkit\\CUDA\\v10.1\\cudnn-7.6.5.32\\lib\\x64)
    SET(TENSORRT_INC_DIR C:\\Program\ Files\\NVIDIA\ GPU\ Computing\ Toolkit\\CUDA\\v10.1\\TensorRT-6.0.1.5\\include)
    SET(TENSORRT_LIB_DIR C:\\Program\ Files\\NVIDIA\ GPU\ Computing\ Toolkit\\CUDA\\v10.1\\TensorRT-6.0.1.5\\lib)

ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    MESSAGE(STATUS "current platform: FreeBSD")
ELSE ()
    MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

set(Hikvison_LIB_DIRS ${HKVS_ROOT_DIR}/lib ${HKVS_ROOT_DIR}/lib/HCNetSDKCom)
set(Hikvison_INCLUDE_DIRS ${HKVS_ROOT_DIR}/include)
set(OpenCV_LIB_DIRS ${OpenCV_ROOT_DIR}/lib)
set(OpenCV_INCLUDE_DIRS ${OpenCV_ROOT_DIR}/include)
set(ONNX_DIR onnxruntime-linux-x64-1.10.0)
set(EIGEN_DIR "eigen-3.4.0")
link_directories("${ONNX_DIR}/lib")

#if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
#    message("Build Release")
#    set(MEG_VERSION host_with_cuda_release)

#else()
#    set(MEG_VERSION host)
#endif()








#include(PPDET_BASE/cmake/yaml-cpp.cmake)

SET(SRCS
#        ppdet_base.cpp
        PPDET_BASE/utils.cc
        PPDET_BASE/preprocess_op.cc
        PPDET_BASE/picodet_postprocess.cc
        PPDET_BASE/object_detector.cc
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




find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets QuickWidgets Multimedia REQUIRED)


include_directories(
    "./include",
    "include",
    "./${ONNX_DIR}/include",
    "./${EIGEN_DIR}"
    )
include_directories(${Hikvison_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

link_directories(${OpenCV_LIB_DIRS} ${Hikvison_LIB_DIRS})

set(PROJECT_SOURCES
        qml.qrc
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        hikvisonhandlercstyelfuncs.cpp
        hikvisonhandler.cpp
        hikvisonviewer.cpp
        imageviewer.cpp
        audiodevice.h
        audiodevice.cpp
        CsvPraser.cpp
        CsvPraser.h
        transforms.cpp
        transforms.h
        crnn.cpp
        models.h models.cpp
        audio_handler.h audio_handler.cpp
        imagehandler.h imagehandler.cpp

)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(HikvionProjectV2
        ${PROJECT_SOURCES}
#        ${SRCS}
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



target_link_libraries(HikvionProjectV2 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::QuickWidgets Qt${QT_VERSION_MAJOR}::Multimedia
    ${OpenCV_LIBS} hcnetsdk PlayCtrl onnxruntime soxr)

#target_link_libraries(HikvionProjectV2 PRIVATE paddle_inference.lib libyaml-cppmt.lib glog.lib)
#target_link_libraries(HikisonProjectV2 PRIVATE megengine);
#target_compile_definitions(HikvionProjectV2 PRIVATE MGE_DLL_IMPORT_DATA)








#link_directories(
#        "${PADDLE_DIR}/paddle/lib",
#        "${CMAKE_BINARY_DIR}/ext/yaml-cpp/lib"
#        "${OPENCV_DIR}/build/x64/vc15/lib"
#        "${PADDLE_DIR}/third_party/install/glog/lib"
#        "${PADDLE_DIR}/third_party/install/protobuf/lib"
#)
#link_directories("${PADDLE_DIR}/third_party/install/gflags/lib")
#link_directories("${PADDLE_DIR}/third_party/install/xxhash/lib")




#link_directories("${CMAKE_CURRENT_BINARY_DIR}")

#include_directories(
#        "PPDET_BASE",
#        "${PADDLE_DIR}/paddle/include",
#        "${CMAKE_BINARY_DIR}/ext/yaml-cpp/src/ext-yaml-cpp/include",
#        "${OPENCV_DIR}/build/include",
#        "${PADDLE_DIR}/third_party/install/glog/include"
#)
