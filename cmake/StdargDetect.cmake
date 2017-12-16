function (plibsys_detect_va_copy result)
  #
  # On platforms that supports va_start() and al.
  # there is sometimes the need to do va_copy() when
  # calling another va_list aware function.
  #
  # This mean that va_copy() is not needed (thus not available)
  # everywhere.
  #
  # We depend on stdarg in any case, stdio in some cases.
  #
  # Know implementations vary from va_copy to __va_copy (old proposed name).
  # We check the _va_copy eventually.
  #
  set (P_VA_COPY FALSE)
  foreach (KEYWORD "va_copy" "_va_copy" "__va_copy")
    try_compile (P_OK_${KEYWORD} ${CMAKE_CURRENT_BINARY_DIR}
      ${PROJECT_SOURCE_DIR}/cmake/va_copy.c
      COMPILE_DEFINITIONS "-Dp_va_copy=${KEYWORD}"
      )
    if (P_OK_${KEYWORD})
      message (STATUS "Looking for ${KEYWORD} - yes")
      set (P_VA_COPY ${KEYWORD})
      break ()
    else ()
      message (STATUS "Looking for ${KEYWORD} - no")
    endif ()
  endforeach ()

  # Assign result
  set (${result} ${P_VA_COPY} PARENT_SCOPE)

endfunction (plibsys_detect_va_copy)
