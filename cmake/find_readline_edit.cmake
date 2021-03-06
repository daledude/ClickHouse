set (READLINE_PATHS "/usr/local/opt/readline/lib")
# First try find custom lib for macos users (default lib without history support)
find_library (READLINE_LIB NAMES readline PATHS ${READLINE_PATHS} NO_DEFAULT_PATH)
if (NOT READLINE_LIB)
    find_library (READLINE_LIB NAMES readline PATHS ${READLINE_PATHS})
endif ()

list(APPEND CMAKE_FIND_LIBRARY_SUFFIXES .so.2)

find_library (TERMCAP_LIB NAMES termcap)
find_library (EDIT_LIB NAMES edit)

set(READLINE_INCLUDE_PATHS "/usr/local/opt/readline/include")
if (READLINE_LIB AND TERMCAP_LIB)
    find_path (READLINE_INCLUDE_DIR NAMES readline/readline.h PATHS ${READLINE_INCLUDE_PATHS} NO_DEFAULT_PATH)
    if (NOT READLINE_INCLUDE_DIR)
        find_path (READLINE_INCLUDE_DIR NAMES readline/readline.h PATHS ${READLINE_INCLUDE_PATHS})
    endif ()
    set (USE_READLINE 1)
    set (LINE_EDITING_LIBS ${READLINE_LIB} ${TERMCAP_LIB})
    message (STATUS "Using line editing libraries (readline): ${READLINE_INCLUDE_DIR} : ${LINE_EDITING_LIBS}")
elseif (EDIT_LIB)
    find_library (CURSES_LIB NAMES curses)
    set (USE_LIBEDIT 1)
    find_path (READLINE_INCLUDE_DIR NAMES editline/readline.h PATHS ${READLINE_INCLUDE_PATHS})
    set (LINE_EDITING_LIBS ${EDIT_LIB} ${CURSES_LIB} ${TERMCAP_LIB})
    message (STATUS "Using line editing libraries (edit): ${READLINE_INCLUDE_DIR} : ${LINE_EDITING_LIBS}")
else ()
    message (STATUS "Not using any library for line editing.")
endif ()
if (READLINE_INCLUDE_DIR)
    include_directories (${READLINE_INCLUDE_DIR})
endif ()

include (CheckCXXSourceRuns)

set (CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${LINE_EDITING_LIBS})
check_cxx_source_runs ("
    #include <stdio.h>
    #include <readline/readline.h>
    #include <readline/history.h>
    int main() {
        add_history(nullptr);
        append_history(1,nullptr);
        return 0;
    }
" HAVE_READLINE_HISTORY)
