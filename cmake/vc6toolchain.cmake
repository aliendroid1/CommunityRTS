#Defining convenience vars

set(VS_X86 "C:/PROGRA~2/MICROS~2")
set(VC6_ROOT "${VS_X86}/VC98")
set(VS_X86_COMMON "${VS_X86}/Common")

set(VC6_INCLUDES  "${VC6_ROOT}/INCLUDE;${VC6_ROOT}/MFC/INCLUDE;${VC6_ROOT}/ATL/INCLUDE")
set(VC6_LIBRARIES "${VC6_ROOT}/LIB;${VC6_ROOT}/MFC/LIB")
set(VC6_PATH "${VC6_ROOT}/BIN;${VS_X86_COMMON}/Tools;${VS_X86_COMMON}/Tools/WinNT;${VS_X86_COMMON}/MSDev98/Bin")
set(VC6_DEV "${VS_X86_COMMON}/MSDev98")

set(STANDARD_LIBRARIES "kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib")

#Setting toolchain vars to configure ninja for vc6

set(ENV{INCLUDE}   "${VC6_INCLUDES};$ENV{INCLUDE}")
set(ENV{LIB}       "${VC6_LIBRARIES};$ENV{LIB}")
set(ENV{PATH}      "${VC6_PATH};$ENV{PATH}")
set(ENV{MSDEV_DIR} "${VC6_DEV}")

set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES  "$ENV{INCLUDE}")
set(CMAKE_CXX_STANDARD_LIBRARIES "${STANDARD_LIBRARIES}")
set(CMAKE_CXX_STANDARD_LINK_DIRECTORIES  "$ENV{LIB}")

set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES  "$ENV{INCLUDE}")
set(CMAKE_C_STANDARD_LIBRARIES "${STANDARD_LIBRARIES}")
set(CMAKE_C_STANDARD_LINK_DIRECTORIES  "$ENV{LIB}")

# Setting C++ default compiler flags for VC6 

string(JOIN " " VC6_CXX_FLAGS
    "/DWIN32"          # Define WIN32 macro
    "/D_WINDOWS"       # Define _WINDOWS macro
    "/Zm800"           # Cap precompiled header memory allocation to prevent running out of heap space
    "/Gd"              # __cdecl calling convention for all functions (C/C++ standard behaviour)
    "/GR"              # Enable RTTI (Run-Time Type Information, needed for things like dynamic_cast)
    "/Op"              # Improve floating point consistency (More ISO-compliant and closer to 64bit behaviour)
    "/EHs"             # Synchronous C++ exception handling (ISO-standard C++ exception handling)
#    "/Za"              # Disable Microsoft language extensions (enforce standard)
)
set(CMAKE_CXX_FLAGS "${VC6_CXX_FLAGS}" CACHE STRING "" FORCE)

string(JOIN " " VC6_CXX_FLAGS_DEBUG
    "/Od"              # Disable optimizations
    "/Ob0"             # Disable inline expansion
    "/EHc-"            # Do NOT assume extern "C" functions never throw (less strict, useful for debugging)
    "/Oy-"             # Disable frame pointer omission (helps debugging)
    "/GZ"              # Helps Catch Release-Build Errors in Debug Build
    "/Yd"              # Store complete debug info in .obj (needed for embedded debug info from precompiled header to be accessible)
)
set(CMAKE_CXX_FLAGS_DEBUG "${VC6_CXX_FLAGS_DEBUG}" CACHE STRING "" FORCE)

string(JOIN " " VC6_CXX_FLAGS_RELEASE
    "/O2"              # Optimize for speed
    "/Ob2"             # Inline any suitable function
    "/EHc"             # Assume extern "C" functions never throw (safe for release)
    "/Oy"              # Omit frame pointer (smaller/faster code)
    "/nologo"          # Suppress copyright message
)
set(CMAKE_CXX_FLAGS_RELEASE "${VC6_CXX_FLAGS_RELEASE}" CACHE STRING "" FORCE)

# Setting C++ default linker flags for VC6

string(JOIN " " VC6_EXE_LINKER_FLAGS
    "/machine:IX86"        # Target x86 architecture
    "/LARGEADDRESSAWARE"   # Allow >2GB RAM on x64 OS (useful for large address space on 32-bit binaries)
)
set(CMAKE_EXE_LINKER_FLAGS "${VC6_EXE_LINKER_FLAGS}" CACHE STRING "" FORCE)

string(JOIN " " VC6_EXE_LINKER_FLAGS_RELEASE
    "/RELEASE"             # Set the checksum in the header and mark as release
    "/DEBUG:NONE"          # Do not generate debug info
    "/OPT:REF,ICF"         # Remove unreferenced code/data, COMDAT folding
    "/NOLOGO"              # Suppress linker startup banner
)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${VC6_EXE_LINKER_FLAGS_RELEASE}" CACHE STRING "" FORCE)

string(JOIN " " VC6_EXE_LINKER_FLAGS_DEBUG
    #"/DEBUG"               # Generate debug info
    #"/PDB:NONE"            # Do not generate a PDB file
    "/DEBUGTYPE:CV"        # Use CodeView format for debug info (VC6 default)
    "/OPT:NOREF,NOICF"     # Do not remove unreferenced code/data, COMDAT folding
    "/PDBTYPE:SEPT"        # Separate debug symbols (one .pdb per .obj, easier for incremental linking)
    "/INCREMENTAL:YES"     # Enable incremental linking
    "/VERBOSE"             # Enable verbose output from the linker
)
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${VC6_EXE_LINKER_FLAGS_DEBUG}" CACHE STRING "" FORCE)

# @todo: Set default flags for C