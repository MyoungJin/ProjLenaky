cmake_minimum_required(VERSION 3.3)

add_definitions("-std=c++11")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -W -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall")
#file(GLOB COLL_FILES ${COMMON_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/collector/*)

add_executable(hwserver zmq_rep_server.cpp)
target_link_libraries(hwserver zmq)
# build REP-REQ (Server:REP) vs (Client:REQ)

add_executable(hwserver_2 zmq_req_server.cpp)
target_link_libraries(hwserver_2 zmq)
# build REP-REQ (Server:REQ) vs (Client:REP)

add_executable(hwserver_3 zmq_rep_server_twisted.cpp)
target_link_libraries(hwserver_3 zmq)
# build REP-REQ (Server:REP twice) vs (Client:REQ and recv twice)
# 이건 죽는다. REQ-REP는 한번씩 왔다리 갔다리 해야 한다.

add_executable(hwserver_4 zmq_rep_server_twisted2.cpp)
target_link_libraries(hwserver_4 zmq)
# build REP-REQ (Server:REP twice with ZMQ_SNDMORE) vs (Client:REQ and recv twice)
# 받는 쪽에서는 recv 한번만 하면 되는 줄 알았는데 아니었다.
# 이를 멀티 파트 메시지라고 하는데
# while (1) {
#    zmq_msg_t message;
#    zmq_msg_init (&message);
#    zmq_msg_recv (&message, socket, 0);
#    // 메시지 프레임을 처리한다.
#    …
#    zmq_msg_close (&message);
#    if (!zmq_msg_more (&message))
#        break;      // 마지막 메시지 프레임
#}
# 이런식으로 핸들링 한다고..

add_executable(hwserver_5 zmq_rep_server_reverse.cpp)
target_link_libraries(hwserver_5 zmq)
# 바보짓했다. REQ-REP는 서버/클라이언트 개념으로 접근할 문제가 아니었다.
# 단지 순서의 문제이다.

add_executable(hwserver_6 zmq_req_rep_conclusion.cpp)
target_link_libraries(hwserver_6 zmq)

# Publisher-Subscriber 패턴
add_executable(pubsub zmq_pub_sub.cpp)
target_link_libraries(pubsub zmq)

# Publisher-Subscriber 패턴 2 서버 클라이언트는 의미가 없다를 확인하기 위한.
add_executable(pubsub2 zmq_pub_sub_sc_switched.cpp)
target_link_libraries(pubsub2 zmq)
