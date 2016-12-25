set(commonSources 
	TypeHelpers.hpp
	Semaphore.hpp
	Synchronized.hpp
)
install (FILES ${commonSources} DESTINATION include/common)

set(commonTestSources 
    ${commonSources}
	unittest/LogHelpers.hpp
	unittest/Test_Synchronized.cpp
	unittest/Test_TypeHelpers.cpp
)

add_cxx_test(commonTests "${commonTestSources}")
