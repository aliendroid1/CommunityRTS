set(COMPRESSION_SRC
    Compression.h
    CompressionManager.cpp
    EAC/btreeabout.cpp
    EAC/btreecodex.h
    EAC/btreedecode.cpp
    EAC/btreeencode.cpp
    EAC/codex.h
    EAC/gimex.h
    EAC/huffabout.cpp
    EAC/huffcodex.h
    EAC/huffdecode.cpp
    EAC/huffencode.cpp
    EAC/refabout.cpp
    EAC/refcodex.h
    EAC/refdecode.cpp
    EAC/refencode.cpp
    
    
)

add_library(core_compression STATIC)
set_target_properties(core_compression PROPERTIES OUTPUT_NAME compression)

target_sources(core_compression PRIVATE ${COMPRESSION_SRC})

target_include_directories(core_compression INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(core_compression PRIVATE
    core_config
    corei_libraries_include
)



target_link_libraries(core_compression PUBLIC libzlib)

