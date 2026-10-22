# SparkInterop shared library + optional CoreCLR host.

find_program(SPARK_PYTHON3 python3)
set(SPARK_SHARP_ROOT "${CMAKE_SOURCE_DIR}")
set(SPARK_ENGINE_ROOT "${SPARK_ENGINE_ROOT}")

if(SPARK_PYTHON3)
    set(SPARK_INTEROP_BINDGEN_H "${CMAKE_CURRENT_SOURCE_DIR}/include/spark/scripting/SparkInteropComponentBindings.generated.h")
    set(SPARK_INTEROP_BINDGEN_CPP "${CMAKE_CURRENT_SOURCE_DIR}/src/spark/scripting/SparkInteropComponentBindings.generated.cpp")
    add_custom_command(
            OUTPUT "${SPARK_INTEROP_BINDGEN_H}" "${SPARK_INTEROP_BINDGEN_CPP}"
            COMMAND ${CMAKE_COMMAND} -E env
                    SPARK_ENGINE_ROOT=${SPARK_ENGINE_ROOT}
                    SPARK_SHARP_ROOT=${SPARK_SHARP_ROOT}
                    ${SPARK_PYTHON3} "${SPARK_SHARP_ROOT}/tools/spark_script_bindgen.py"
            DEPENDS "${SPARK_SHARP_ROOT}/tools/spark_script_bindgen.py"
            COMMENT "Generating SPARK_SCRIPT_BIND interop thunks (engine headers + sharp outputs)"
            VERBATIM
    )
    add_custom_target(SparkInteropBindingsCodegen DEPENDS
            "${SPARK_INTEROP_BINDGEN_H}"
            "${SPARK_INTEROP_BINDGEN_CPP}"
    )
endif()

set(SPARK_INTEROP_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/spark/scripting/SparkInterop.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/spark/scripting/SparkInteropInternal.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/spark/scripting/SparkInteropMath.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/spark/scripting/SparkInteropScene.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/spark/scripting/SparkInteropPlatformer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/spark/scripting/SparkInteropPhysics2D.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/spark/scripting/SparkInteropComponents.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/spark/scripting/SparkInteropFoliage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/spark/scripting/SparkInteropComponentFactory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/spark/scripting/SparkInteropGameplay.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/spark/scripting/SparkInteropComponentBindings.generated.cpp
)

add_library(SparkInterop SHARED ${SPARK_INTEROP_SOURCES})
target_compile_features(SparkInterop PUBLIC cxx_std_23)
if(TARGET SparkInteropBindingsCodegen)
    add_dependencies(SparkInterop SparkInteropBindingsCodegen)
endif()
target_compile_definitions(SparkInterop PRIVATE SPARK_INTEROP_EXPORTS)
target_include_directories(SparkInterop
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${SPARK_ENGINE_ROOT}/include
        ${SPARK_ENGINE_ROOT}/build/include
)
target_link_libraries(SparkInterop PUBLIC SparkEngine SparkGameplay)

if(APPLE)
    set_target_properties(SparkInterop PROPERTIES INSTALL_RPATH "@loader_path")
endif()

find_program(SPARK_DOTNET dotnet)
if(SPARK_DOTNET)
    set(SPARK_BINDINGS_PROJECT "${SPARK_SHARP_ROOT}/scripting/bindings/generated/Spark.Bindings/Spark.Bindings.csproj")
    set(SPARK_SCRIPTING_PROJECT "${SPARK_SHARP_ROOT}/scripting/Spark.Scripting/Spark.Scripting.csproj")
    set(SPARK_HELLO_CS_GAME_PROJECT "${SPARK_SHARP_ROOT}/scripting/samples/HelloCsGame/HelloCsGame.csproj")
    add_custom_target(SparkBindingsBuild
            COMMAND ${SPARK_DOTNET} build "${SPARK_BINDINGS_PROJECT}" -c Release -p:NuGetAudit=false
            COMMAND ${SPARK_DOTNET} build "${SPARK_SCRIPTING_PROJECT}" -c Release -p:NuGetAudit=false
            WORKING_DIRECTORY ${SPARK_SHARP_ROOT}
            COMMENT "Building Spark.Bindings + Spark.Scripting (Release)"
    )
    add_dependencies(SparkBindingsBuild SparkInterop)

    add_custom_target(SparkHelloCsGameDebug
            COMMAND ${SPARK_DOTNET} build "${SPARK_HELLO_CS_GAME_PROJECT}" -c Debug -p:NuGetAudit=false
            WORKING_DIRECTORY ${SPARK_SHARP_ROOT}
            COMMENT "Building HelloCsGame (Debug, for script host debugging)"
    )
    add_dependencies(SparkHelloCsGameDebug SparkBindingsBuild)
endif()

if(SPARK_SHARP_BUILD_SCRIPT_HOST)
    include(${SPARK_SHARP_ROOT}/cmake/DotNetHost.cmake)
    spark_find_dotnet_host(SPARK_DOTNET_HOST_INCLUDE SPARK_NETHOST_LIBRARY)

    add_executable(SparkScriptHost
            ${CMAKE_CURRENT_SOURCE_DIR}/src/spark/scripting/SparkScriptHost.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/spark/scripting/ManagedGameBridge.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/spark/scripting/CoreClrHost.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/spark/scripting/HostFxrLoader.cpp
    )
    target_include_directories(SparkScriptHost PRIVATE ${SPARK_DOTNET_HOST_INCLUDE})
    target_link_libraries(SparkScriptHost PRIVATE SparkEngine SparkInterop ${SPARK_NETHOST_LIBRARY})
    if(APPLE)
        set_target_properties(SparkScriptHost PROPERTIES BUILD_RPATH "@loader_path")
    endif()
    if(TARGET SparkBindingsBuild)
        add_dependencies(SparkScriptHost SparkBindingsBuild)
    endif()

    set(SPARK_CLION_RUN_SCRIPT "${SPARK_SHARP_ROOT}/tools/clion-run-hello-cs-game-debug.sh")
    add_custom_target(SparkRunHelloCsGameDebug
            COMMAND ${CMAKE_COMMAND} -E env
                    SPARK_ENGINE_ROOT=${SPARK_ENGINE_ROOT}
                    SPARK_SHARP_BUILD_DIR=${CMAKE_BINARY_DIR}
                    ${SPARK_CLION_RUN_SCRIPT}
            USES_TERMINAL
            COMMENT "Run SparkScriptHost with HelloCsGame (Debug); cwd=SPARK_ENGINE_ROOT"
    )
    add_dependencies(SparkRunHelloCsGameDebug SparkScriptHost SparkHelloCsGameDebug)
endif()
