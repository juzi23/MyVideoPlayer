cmake_minimum_required(VERSION 3.4.1)

#将ffmpeg相关头文件导入
include_directories(src/main/jni/ffmpeg/include/)

#将videoplayer相关头文件导入
include_directories(src/main/jni/videoplayer/include/)

#新建文件夹引用wlxvideoplayer
file(GLOB wlxvideoplayer "src/main/jni/videoplayer/cpp/*.cpp")

#将文件夹引用wlxvideoplayer下的所有.cpp文件加入到wlxvideoplayer动态库中
add_library( wlxvideoplayer

             SHARED

             ${wlxvideoplayer} )

add_library( avcodec-57 SHARED IMPORTED)
set_target_properties( avcodec-57
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavcodec-57.so)


add_library( avdevice-57 SHARED IMPORTED)
set_target_properties( avdevice-57
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavdevice-57.so)


add_library( avfilter-6 SHARED IMPORTED)
set_target_properties( avfilter-6
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavfilter-6.so)


add_library( avformat-57 SHARED IMPORTED)
set_target_properties( avformat-57
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavformat-57.so)


add_library( avutil-55 SHARED IMPORTED)
set_target_properties( avutil-55
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libavutil-55.so)

add_library( postproc-54 SHARED IMPORTED)
set_target_properties( postproc-54
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libpostproc-54.so)

add_library( swresample-2 SHARED IMPORTED)
set_target_properties( swresample-2
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libswresample-2.so)


add_library( swscale-4 SHARED IMPORTED)
set_target_properties( swscale-4
                       PROPERTIES IMPORTED_LOCATION
                       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libswscale-4.so)

target_link_libraries( wlxvideoplayer

                       avcodec-57
                       avdevice-57
                       avfilter-6
                       avformat-57
                       avutil-55
                       postproc-54
                       swresample-2
                       swscale-4

                       OpenSLES

                       GLESv2
                       EGL
                       android

                       log )