// test_durability.cpp — Проверка TRANSIENT_LOCAL
#include <ecal/ecal.h>
#include <ecal/qos.h>
#include <iostream>
#include <thread>
#include <chrono>

int main() {
  eCAL::Initialize("durability_test");
  
  // 🎯 Publisher с TRANSIENT_LOCAL
  eCAL::Publisher::Configuration pub_config;
  pub_config.qos.durability = eCAL::QoS::Durability::TRANSIENT_LOCAL;
  eCAL::CPublisher pub("test_durability", eCAL::SDataTypeInformation{}, pub_config);
  
  // Отправляем одно сообщение ДО подключения подписчика
  std::string first_msg = "FIRST_MESSAGE_BEFORE_SUBSCRIBER";
  pub.Send(first_msg.c_str(), first_msg.size(), pub_config.qos);
  std::cout << "Publisher: sent '" << first_msg << "' (before subscriber)\n";
  
  std::this_thread::sleep_for(std::chrono::seconds(5)); // Даём время на "полёт"
  
  // 🎯 Subscriber подключается ПОЗЖЕ
  eCAL::CSubscriber sub("test_durability");
  sub.SetReceiveCallback([](const eCAL::STopicId&, const eCAL::SDataTypeInformation&, 
                            const eCAL::SReceiveCallbackData& data) {
    if (data.buffer && data.buffer_size > 0) {
      std::string msg(static_cast<const char*>(data.buffer), data.buffer_size);
      std::cout << "Subscriber: received '" << msg << "'\n";
    }
  });
  
  std::cout << "Subscriber: connected, waiting for messages...\n";
  
  // Ждём 3 секунды — если LVC работает, подписчик получит "FIRST_MESSAGE..."
  std::this_thread::sleep_for(std::chrono::seconds(5));
  
  // Отправляем ещё одно сообщение (уже после подключения)
  std::string second_msg = "SECOND_MESSAGE_AFTER_CONNECT";
  pub.Send(second_msg.c_str(), second_msg.size());
  std::cout << "Publisher: sent '" << second_msg << "' (after subscriber)\n";
  
  std::this_thread::sleep_for(std::chrono::seconds(5));
  
  eCAL::Finalize();
  std::cout << "Test completed.\n";
  return 0;
}