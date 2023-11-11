# NOTE: Only best practices articles and posts were used to determine this set
# of compiler warnings. A comprehensive analysis of all warnings provided by
# all compiler vendors was not conducted.

# References
#   * GCC
#       * https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
#       * https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Dialect-Options.html
#   * Clang
#       * https://clang.llvm.org/docs/DiagnosticsReference.html
#   * Recommended compiler warnings
#       * https://github.com/aminya/project_options/blob/main/src/CompilerWarnings.cmake
#   * Compiler flags available for specific versions of compilers
#       * https://github.com/Barro/compiler-warnings/blob/master/gcc/warnings-gcc-8.txt
option(BUILD_ENABLE_COMPILER_WARNINGS "Enable compiler warnings" ON)

# Enable compiler warnings for the directory from which the function was called
function(enable_compiler_warnings)
    set(warnings
        "-Werror"
        "-Wall"
        "-Wextra"

        # Warn on non-standard C++
        "-Wpedantic"

        # Warn whenever a pointer is cast such that the required alignment of the
        # target is increased. For example, warn if a char* is cast to an int* on
        # machines where integers can only be accessed at two- or four-bytes boundaries.
        "-Wcast-align"

        # Warn whenever a pointer is cast so as to remove a type qualifier from the
        # target type. Also warn when making a cast that introduces a type qualifier
        # in an unsafe way.
        "-Wcast-qual"

        # Warn for implicit conversions that may alter a value. Does not warn for
        # explicit casts or if the value is not changed by the conversion. For C++,
        # also warn for confusing overload resolution for user-defined conversions;
        # and conversions that never use a type conversion operator.
        "-Wconversion"

        # Warn when a value of type float is implicitly promoted to double.
        "-Wdouble-promotion"

        # Warn about redundant semicolons after in-class function definitions.
        "-Wextra-semi"

        # Warn if floating-point values are used in equality comparisons.
        "-Wfloat-equal"

        # Check calls to printf and scanf, etc., to make sure that the arguments
        # supplied have types appropriate to the format string specified, and that
        # the conversions specified in the format string make sense.
        #
        # -Wformat=2 enables -Wformat plus additional format checks. Currently
        # equivalent to -Wformat -Wformat-nonliteral -Wformat-security -Wformat-y2k.
        "-Wformat=2"

        # Warn when a class has virtual functions and an accessible non-virtual
        # destructor itself or in an accessbile polymorphic base class, in which
        # case it is possible but unsafe to delete an instance of a derived class
        # through a pointer to the class itself or base class.
        "-Wnon-virtual-dtor"

        # Warn if the compiler detects paths that trigger erroneous or undefined
        # behavior due to dereferencing a null pointer.
        "-Wnull-dereference"

        # Warn if an old-style (C-style) cast to a non-void type is used within a
        # C++ program. The new-style casts (dynamic_cast, static_cast,
        # reinterpret_cast, and const_cast) are less vulnerable to unintended
        # effects and much easier to search for.
        "-Wold-style-cast"

        # Warn when a function declaration hides virtual functions from a base class.
        #
        # "Level 1 is included in -Wall" in later compiler versions.
        "-Woverloaded-virtual"

        # Warn whenever a local variable or type declaration shadows another variable,
        # parameter, type, class member or whenever a built-in function is shadowed.
        "-Wshadow"

        # Warn if an undefined identifier is evaluated in an #if directive. Such
        # identifiers are replaced with zero.
        "-Wundef"

        # Warn when a literal '0' is used as null pointer constant.
        "-Wzero-as-null-pointer-constant"
    )

    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        list(APPEND warnings
        # Warn about overriding virtual functions that are not marked with the
        # override keyword.
        "-Wsuggest-override"

        # Warn when an if-else has identical branches.
        "-Wduplicated-branches"

        # Warn about duplicated conditions in an if-else-if chain.
        "-Wduplicated-cond"

        # Warn about suspicious uses of logical operators in expressions. This
        # includes using logical operators in contexts where a bit-wise operator
        # is likely to be expected. Also warns when the operands of a logical
        # operator are the same.
        "-Wlogical-op"

        # Warn when an expression is cast to its own type.
        "-Wuseless-cast"
        )
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # NOTE: An effort was only made to find the GCC-specific flags in
        # Clang. There was no effort to comprehensively find the optimal
        # set of compiler flags we should use, neither in GCC nor Clang.
        list(APPEND warnings
            "-Wbitwise-instead-of-logical"
        )
    endif()

    add_compile_options(${warnings})
    message(STATUS "Enabled compiler warnings: ${warnings}")
    unset(warnings)
endfunction()
