# Author: Marguerite Su <i@marguerite.su>
# License: GPL
# Description: find Rime schema collection package.
# RIME_DATA_FOUND - System has rime-data package
# RIME_DATA_DIR - rime-data absolute path

set(RIME_DATA_FIND_DIR "${CMAKE_INSTALL_PREFIX}/share/rime-data"
                       "${CMAKE_INSTALL_PREFIX}/share/rime/data"
                       "/usr/share/rime-data"
                       "/usr/share/rime/data")

set(RIME_DATA_FOUND FALSE)

foreach(_RIME_DATA_DIR ${RIME_DATA_FIND_DIR})
    if (IS_DIRECTORY ${_RIME_DATA_DIR})
        set(RIME_DATA_FOUND True)
        set(RIME_DATA_DIR ${_RIME_DATA_DIR})
    endif (IS_DIRECTORY ${_RIME_DATA_DIR})
endforeach(_RIME_DATA_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RimeData DEFAULT_MSG RIME_DATA_DIR)
mark_as_advanced(RIME_DATA_DIR)
