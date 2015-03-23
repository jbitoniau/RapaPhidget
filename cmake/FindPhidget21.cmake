# Determine the Phidget21 API include and library paths. 
# Once done, the following is defined
#  PHIDGET21_FOUND - system has Phidget21
#  Phidget21_INCLUDE_DIR - the Phidget21 include directories
#  Phidget21_LIBRARY - link these to use Phidget21
IF( CMAKE_SYSTEM_NAME MATCHES "Windows" )

	# On Windows, the standard install path of for Phidget is the following
	SET( Phidget21_DEFAULT_INSTALL_PATH "C:/Program Files/Phidgets" )
	
	# Try to find the main header and determine the include path
	FIND_PATH( Phidget21_INCLUDE_DIR NAMES phidget21.h PATHS ${Phidget21_DEFAULT_INSTALL_PATH} )
	
	# If we've found the header, deduce the library path for the current architecture (32 or 64 bits)
	SET( Phidget21_LIB_SEARCH_PATHS ${Phidget21_DEFAULT_INSTALL_PATH} )
	
	IF ( Phidget21_INCLUDE_DIR )
		SET( Phidget21_LIB_SEARCH_PATHS ${Phidget21_INCLUDE_DIR} )
	ENDIF()
	
	IF ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		SET( Phidget21_LIB_SEARCH_PATHS 
			 ${Phidget21_LIB_SEARCH_PATHS}
			 ${Phidget21_LIB_SEARCH_PATHS}/x64 )
	ELSE()
		SET( Phidget21_LIB_SEARCH_PATHS 
			 ${Phidget21_LIB_SEARCH_PATHS}/x86} )
	ENDIF()

	FIND_LIBRARY( Phidget21_LIBRARY NAMES phidget21 PATHS ${Phidget21_LIB_SEARCH_PATHS} )
	
ELSEIF( CMAKE_SYSTEM_NAME MATCHES "Darwin" )

	# On MacOS, if the Phidget21 framework has been properly installed, CMake should find it automatically
	FIND_PATH( Phidget21_INCLUDE_DIR NAMES phidget21.h )
	FIND_LIBRARY( Phidget21_LIBRARY NAMES Phidget21 )
	
ELSEIF( CMAKE_SYSTEM_NAME MATCHES "Linux" )

	FIND_PATH( Phidget21_INCLUDE_DIR NAMES phidget21.h  )
	FIND_LIBRARY( Phidget21_LIBRARY NAMES phidget21 )

ENDIF()

# Handle the QUIETLY and REQUIRED arguments and set PHIDGET21_FOUND to TRUE if all listed variables are TRUE
# Note that this necessarily generates a upper-case variable name! 
INCLUDE( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( Phidget21 DEFAULT_MSG Phidget21_LIBRARY Phidget21_INCLUDE_DIR )

#MESSAGE( "PH#DGET21_FOUND: ${PHIDGET21_FOUND})
#MESSAGE( "Phidget21_INCLUDE_DIR: ${Phidget21_INCLUDE_DIR}")
#MESSAGE( "Phidget21_LIBRARY: ${Phidget21_LIBRARY}")