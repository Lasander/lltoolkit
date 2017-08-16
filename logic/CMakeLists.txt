set(logicSources 
	StateMachine.hpp
	StateMachine2.hpp
)
install (FILES ${commonSources} DESTINATION include/logic)

set(logicTestSources 
    ${logicSources}
	unittest/Test_StateMachine.cpp
	unittest/Test_StateMachine2.cpp
)

add_cxx_test(logicTests "${logicTestSources}")
