# Author: Marguerite Su <i@marguerite.su>
# License: GPL
# Description: find Rime schema collection package (brise).
# BRISE_FOUND - System has brise package
# BRISE_DIR - Brise absolute path

set(BRISE_FIND_DIR "${CMAKE_INSTALL_PREFIX}/share/brise"
                   "${CMAKE_INSTALL_PREFIX}/share/rime-data"
                   "${CMAKE_INSTALL_PREFIX}/share/rime/data"
                   "/usr/share/brise"
                   "/usr/share/rime-data"
                   "/usr/share/rime/data")

set(BRISE_FOUND FALSE)

foreach(_BRISE_DIR ${BRISE_FIND_DIR})
    if (IS_DIRECTORY ${_BRISE_DIR})
        set(BRISE_FOUND True)
        set(BRISE_DIR ${_BRISE_DIR})
    endif (IS_DIRECTORY ${_BRISE_DIR})
endforeach(_BRISE_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Brise DEFAULT_MSG BRISE_DIR)
mark_as_advanced(BRISE_DIR)
