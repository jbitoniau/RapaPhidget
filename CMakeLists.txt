CMAKE_MINIMUM_REQUIRED( VERSION 3.0 )

PROJECT( "RapaPhidget" )

SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake" )

IF( MSVC )
	INCLUDE( VisualStudioSettings )
ENDIF()
	
INCLUDE( FindPhidget21 )
IF( PHIDGET21_FOUND )
	
	INCLUDE_DIRECTORIES( ${Phidget21_INCLUDE_DIR} )
	INCLUDE_DIRECTORIES( include )

	SET	(	HEADERS
			include/RPhiVector3.h
			include/RPhiDevice.h
			include/RPhiSpatial.h
			include/RPhiTemperatureSensor.h
			include/RPhiDeviceManager.h
			include/RPhiLocalDeviceManager.h
			include/RPhiRemoteDeviceManager.h
		)			

	SET	(	SOURCES
			src/RPhiDevice.cpp
			src/RPhiSpatial.cpp
			src/RPhiTemperatureSensor.cpp
			src/RPhiDeviceManager.cpp
			src/RPhiLocalDeviceManager.cpp
			src/RPhiRemoteDeviceManager.cpp
		)	
	
	SOURCE_GROUP("" FILES ${HEADERS} ${SOURCES} )		# Avoid "Header Files" and "Source Files" virtual folders in VisualStudio

	SET(CMAKE_DEBUG_POSTFIX "d")
	ADD_LIBRARY( ${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES} )
	TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${Phidget21_LIBRARY} ) 
	
	#
	# Install
	#
	INSTALL(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
			LIBRARY DESTINATION lib
			ARCHIVE DESTINATION lib
			RUNTIME DESTINATION bin )
			#INCLUDES DESTINATION include )		# If uncommented, the ${PROJECT_NAME} target contains INCLUDE_DIRECTORIES information. Importing the target automatically adds this directory to the INCLUDE_DIRECTORIES.
	SET( TARGET_NAMESPACE Rapa:: )
	INSTALL( FILES ${HEADERS} DESTINATION include COMPONENT Devel )		
	EXPORT( EXPORT ${PROJECT_NAME}Targets FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake" NAMESPACE ${TARGET_NAMESPACE} )
	CONFIGURE_FILE( cmake/${PROJECT_NAME}Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake" @ONLY )
	SET( ConfigPackageLocation lib/cmake/${PROJECT_NAME} )
	INSTALL(EXPORT ${PROJECT_NAME}Targets
			FILE ${PROJECT_NAME}Targets.cmake
			NAMESPACE ${TARGET_NAMESPACE}
			DESTINATION ${ConfigPackageLocation} )
	INSTALL( FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake" DESTINATION ${ConfigPackageLocation} COMPONENT Devel )
	
	ADD_SUBDIRECTORY( samples )

ELSE()
	MESSAGE("Phidget21 not found")
ENDIF()

