set(logicSources 
	StateMachine.hpp
	StateMachine2.hpp
	StateMachine3.hpp
)
install (FILES ${commonSources} DESTINATION include/logic)

set(logicTestSources 
    ${logicSources}
	unittest/Test_StateMachine.cpp
	unittest/Test_StateMachine2.cpp
	unittest/Test_StateMachine3.cpp
)

add_cxx_test(logicTests "${logicTestSources}")
