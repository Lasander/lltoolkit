set(logicSources 
	StateMachine.hpp
)
install (FILES ${commonSources} DESTINATION include/logic)

set(logicTestSources 
    ${logicSources}
	unittest/Test_StateMachine.cpp
)

add_cxx_test(logicTests "${logicTestSources}")
