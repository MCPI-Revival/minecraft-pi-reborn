project(openal)

# Silence Warnings
add_compile_options(-w)

## OpenAL

# Build
add_library(openal SHARED
    # https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L721
    src/common/alcomplex.cpp
    src/common/almalloc.cpp
    src/common/alstring.cpp
    src/common/althrd_setname.cpp
    src/common/dynload.cpp
    src/common/filesystem.cpp
    src/common/pffft.cpp
    src/common/polyphase_resampler.cpp
    src/common/strutils.cpp
    # https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L769
    src/core/ambdec.cpp
    src/core/ambidefs.cpp
    src/core/bformatdec.cpp
    src/core/bs2b.cpp
    src/core/bsinc_tables.cpp
    src/core/context.cpp
    src/core/converter.cpp
    src/core/cpu_caps.cpp
    src/core/cubic_tables.cpp
    src/core/devformat.cpp
    src/core/device.cpp
    src/core/effectslot.cpp
    src/core/except.cpp
    src/core/filters/biquad.cpp
    src/core/filters/nfc.cpp
    src/core/filters/splitter.cpp
    src/core/fpu_ctrl.cpp
    src/core/helpers.cpp
    src/core/hrtf.cpp
    src/core/hrtf_loader.cpp
    src/core/hrtf_resource.cpp
    src/core/logging.cpp
    src/core/mastering.cpp
    src/core/mixer.cpp
    src/core/storage_formats.cpp
    src/core/uhjfilter.cpp
    src/core/uiddefs.cpp
    src/core/voice.cpp
    # https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L879
    src/core/mixer/mixer_c.cpp
    # https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L886
    src/al/auxeffectslot.cpp
    src/al/buffer.cpp
    src/al/debug.cpp
    src/al/effect.cpp
    src/al/effects/autowah.cpp
    src/al/effects/chorus.cpp
    src/al/effects/compressor.cpp
    src/al/effects/convolution.cpp
    src/al/effects/dedicated.cpp
    src/al/effects/distortion.cpp
    src/al/effects/echo.cpp
    src/al/effects/effects.cpp
    src/al/effects/equalizer.cpp
    src/al/effects/fshifter.cpp
    src/al/effects/modulator.cpp
    src/al/effects/null.cpp
    src/al/effects/pshifter.cpp
    src/al/effects/reverb.cpp
    src/al/effects/vmorpher.cpp
    src/al/error.cpp
    src/al/event.cpp
    src/al/extension.cpp
    src/al/filter.cpp
    src/al/listener.cpp
    src/al/source.cpp
    src/al/state.cpp
    # https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L925
    src/alc/alc.cpp
    src/alc/alu.cpp
    src/alc/alconfig.cpp
    src/alc/context.cpp
    src/alc/device.cpp
    src/alc/effects/autowah.cpp
    src/alc/effects/chorus.cpp
    src/alc/effects/compressor.cpp
    src/alc/effects/convolution.cpp
    src/alc/effects/dedicated.cpp
    src/alc/effects/distortion.cpp
    src/alc/effects/echo.cpp
    src/alc/effects/equalizer.cpp
    src/alc/effects/fshifter.cpp
    src/alc/effects/modulator.cpp
    src/alc/effects/null.cpp
    src/alc/effects/pshifter.cpp
    src/alc/effects/reverb.cpp
    src/alc/effects/vmorpher.cpp
    src/alc/events.cpp
    src/alc/panning.cpp
    # https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L1031
    src/alc/backends/base.cpp
    src/alc/backends/loopback.cpp
    src/alc/backends/null.cpp
    # https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/fmt-11.2.0/CMakeLists.txt#L122
    src/fmt-11.2.0/src/format.cc
    src/fmt-11.2.0/src/os.cc
)

# Set C/C++ Standard
# https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L58
set_target_properties(openal PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE
    C_STANDARD 17
)

# https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L149-L156
if(MCPI_WIN32 AND CMAKE_SIZEOF_VOID_P MATCHES "4")
    add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-faligned-new=8>")
endif()

# Compile Flags
if(MCPI_WIN32)
    # https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L271
    target_compile_definitions(openal PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        __USE_MINGW_ANSI_STDIO
    )
    # https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L483
    set(EXPORT_DECL "__declspec(dllexport)")
else()
    # https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L492
    set(EXPORT_DECL "__attribute__((visibility(\"protected\")))")
endif()
target_compile_definitions(openal PRIVATE
    # https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L1779C13-L1779C49
    "ALC_API=${EXPORT_DECL}"
    "AL_API=${EXPORT_DECL}"
    AL_BUILD_LIBRARY
    AL_ALEXT_PROTOTYPES
)
set_target_properties(openal PROPERTIES
    # https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L501
    C_VISIBILITY_PRESET hidden
    CXX_VISIBILITY_PRESET hidden
)

# Optimizations
# https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L978
include(CheckIncludeFile)
function(al_check_header header)
    check_include_file("${header}" "AL_HAVE_${header}")
endfunction()
function(al_add_optimization var header source)
    set(CMAKE_REQUIRED_QUIET TRUE)
    al_check_header("${header}")
    unset(CMAKE_REQUIRED_QUIET)
    if("${AL_HAVE_${header}}")
        target_sources(openal PRIVATE "${source}")
        set("${var}" TRUE)
    endif()
endfunction()
al_add_optimization(HAVE_SSE xmmintrin.h src/core/mixer/mixer_sse.cpp)
al_add_optimization(HAVE_SSE2 emmintrin.h src/core/mixer/mixer_sse2.cpp)
if(HAVE_SSE2)
    set(HAVE_SSE_INTRINSICS TRUE)
endif()
al_add_optimization(HAVE_SSE3 pmmintrin.h src/core/mixer/mixer_sse3.cpp)
al_add_optimization(HAVE_SSE4_1 smmintrin.h src/core/mixer/mixer_sse41.cpp)
al_add_optimization(HAVE_NEON arm_neon.h src/core/mixer/mixer_neon.cpp)
if(HAVE_NEON)
    set(HAVE_NEON_INTRINSICS TRUE)
endif()

# Libraries
target_link_libraries(openal
    m
    Threads::Threads
)
if(NOT MCPI_WIN32)
    target_link_libraries(openal dl)
    set(HAVE_DLFCN_H TRUE)
endif()

# Audio Backends
if(NOT MCPI_WIN32)
    # https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L1058
    set(HAVE_PULSEAUDIO TRUE)
    find_path(PULSEAUDIO_INCLUDE_DIR
        NAMES pulse/pulseaudio.h
        REQUIRED
    )
    find_library(PULSEAUDIO_LIBRARY
        NAMES pulse
        REQUIRED
    )
    target_sources(openal PRIVATE src/alc/backends/pulseaudio.cpp)
    target_include_directories(openal PRIVATE "${PULSEAUDIO_INCLUDE_DIR}")
    target_link_libraries(openal "${PULSEAUDIO_LIBRARY}")
else()
    # https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L1152
    set(HAVE_WASAPI TRUE)
    target_sources(openal PRIVATE src/alc/backends/wasapi.cpp)
    target_link_libraries(openal avrt)
endif()

# Check For CPUID Intrinsic
include(CheckCSourceCompiles)
function(al_intrinsic_check header source_var source)
    # Check Header
    set(CMAKE_REQUIRED_QUIET TRUE)
    al_check_header("${header}")
    if("${AL_HAVE_${header}}")
        string(REPLACE "." "_" header_var "${header}")
        string(TOUPPER "${header_var}" header_var)
        set("HAVE_${header_var}" TRUE PARENT_SCOPE)
        # Check Source
        check_c_source_compiles("#include <${header}>\n${source}" "AL_${source_var}")
        if("${AL_${source_var}}")
            set("${source_var}" TRUE PARENT_SCOPE)
        endif()
    endif()
    unset(CMAKE_REQUIRED_QUIET)
endfunction()
al_intrinsic_check(cpuid.h HAVE_GCC_GET_CPUID "
    int main() {
        unsigned int eax, ebx, ecx, edx;
        return __get_cpuid(0, &eax, &ebx, &ecx, &edx);
    }
")
al_intrinsic_check(intrin.h HAVE_CPUID_INTRINSIC "
    int main() {
        int regs[4];
        __cpuid(regs, 0);
        return regs[0];
    }
")

# Create Empty Version File
# https://github.com/kcat/openal-soft/blob/78a2ddb791163ae0d603937c88613523d27fa22d/CMakeLists.txt#L1412
set(GIT_BRANCH "")
set(GIT_COMMIT_HASH "")
function(al_configure_header file)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/src/${file}.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${file}"
    )
endfunction()
al_configure_header(version.h)

# Generate Configuration Headers
set(HAVE_PTHREAD_SETSCHEDPARAM TRUE)
set(HAVE_PTHREAD_SETNAME_NP TRUE)
if(MCPI_WIN32)
    set(HAVE_GUIDDEF_H TRUE)
endif()
al_configure_header(config.h)
al_configure_header(config_backends.h)
al_configure_header(config_simd.h)

# Headers
target_include_directories(openal
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>"
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include/AL>"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_BINARY_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/common"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/gsl/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/fmt-11.2.0/include"
)

# Install
setup_library(openal TRUE TRUE)

# License
install(
    FILES
        src/COPYING
        src/LICENSE-pffft
        src/BSD-3Clause
    DESTINATION "${MCPI_LEGAL_DIR}/OpenAL"
)