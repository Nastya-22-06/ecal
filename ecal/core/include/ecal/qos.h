/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2026 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/
/**
 * @file   qos.h
 * @brief  eCAL Quality of Service (QoS) policies
**/

#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace eCAL
{
  namespace QoS
  {
    enum class Reliability : std::uint8_t
    {
      BEST_EFFORT = 0,
      RELIABLE    = 1,
    };

    enum class Durability : std::uint8_t
    {
      VOLATILE        = 0,
      TRANSIENT_LOCAL = 1,
    };

    enum class Priority : std::uint32_t
    {
      BACKGROUND = 10,
      LOW        = 25,
      NORMAL     = 50,
      HIGH       = 75,
      CRITICAL   = 100,
    };

    /**
     * @brief Набор QoS-политик для публикации и приёма сообщений.
     *
     * Базовые политики (reliability/priority/deadline_ms/durability) могут задаваться:
     * - на уровне конфигурации publisher/subscriber;
     * - на уровне конкретного сообщения (переопределение при отправке).
     *
     * Дополнительные поля (min_priority/deadline_callback) относятся к поведению подписчика:
     * - min_priority: фильтрация входящих сообщений по минимальному приоритету;
     * - deadline_callback: уведомление о нарушении deadline (сообщение пришло слишком поздно).
     */
    struct Policies
    {
      Reliability reliability  { Reliability::BEST_EFFORT };
      Priority    priority     { Priority::NORMAL };
      std::uint32_t deadline_ms{ 0U };                    //!< 0 == без ограничения на время доставки
      Durability  durability   { Durability::VOLATILE };

      // Дополнительные настройки поведения подписчика
      Priority min_priority{ Priority::BACKGROUND };      //!< По умолчанию принимать все сообщения
      std::function<void(const std::string& topic_name)> deadline_callback{};
    };
  }
}

