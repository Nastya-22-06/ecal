# PROGRESS.md — Журнал разработки QoS для eCAL

> Этот файл содержит полное ТЗ, утверждённую стратегию и текущий прогресс.

---

## ЗАДАЧА

Проект:
- Репозиторий: eclipse-ecal/ecal (форк: github.com/Nastya-22-06/ecal)
- Задача: Реализация Quality of Service (QoS) для сообщений в eCAL middleware

Описание:
Реализовать систему QoS, которая позволит разработчикам:
- Задавать приоритеты сообщений
- Гарантировать доставку критичных данных
- Устанавливать временные ограничения на доставку

---

## ТРЕБОВАНИЯ К РЕАЛИЗАЦИИ

1. Перечисления QoS уровней (eCAL::QoS::Policies):

   Reliability:
   - BEST_EFFORT (0, по умолчанию)
   - RELIABLE (1)

   Priority:
   - BACKGROUND (10)
   - LOW (25)
   - NORMAL (50, по умолчанию)
   - HIGH (75)
   - CRITICAL (100)

   Deadline:
   - uint32_t ms (0 = без ограничений, по умолчанию)

   Durability:
   - VOLATILE (0, по умолчанию)
   - TRANSIENT_LOCAL (1)

2. API Publisher:
   - [ ] Расширить Publisher::Configuration (ecal/config/publisher.h) полем qos типа QoS::Policies
   - [ ] Добавить метод CPublisher::SetQoS()
   - [ ] Добавить перегрузку CPublisher::Send(const void*, size_t, const QoS::Policies&)

3. API Subscriber:
   - [ ] Расширить Subscriber::Configuration (ecal/config/subscriber.h) полем qos
   - [ ] Добавить фильтрацию по min_priority
   - [ ] Добавить deadline_callback для уведомлений о нарушениях

4. Приоритизация в ядре:
   - [ ] Внедрить приоритетные очереди вместо FIFO
   - [ ] Реализовать RELIABLE: ACK+retry для TCP
   - [ ] Проверка deadline + генерация событий
   - [ ] Durability: кэш последнего сообщения для новых подписчиков

5. Транспортные уровни (SHM/UDP/TCP):
   - [ ] SHM: добавить QoS-метаданные в SMemFileHeader
   - [ ] UDP: раздельные очереди по приоритету, отправка CRITICAL с меньшими задержками
   - [ ] TCP: надёжная доставка с подтверждениями и повторными отправками

6. Мониторинг (eCAL Monitor):
   - [ ] Колонка "QoS Priority" с цветовой индикацией:
     * Красный = CRITICAL
     * Оранжевый = HIGH
     * Зеленый = NORMAL
     * Серый = LOW/BACKGROUND
   - [ ] Счётчики нарушений deadline для каждого топика
   - [ ] Индикатор режима надежности (BEST_EFFORT/RELIABLE)
   - [ ] График распределения сообщений по приоритетам

7. Тесты:
   - [ ] Приоритетная отправка (порядок доставки)
   - [ ] Надёжная доставка (потеря пакетов + восстановление)
   - [ ] Deadline violations (генерация событий)
   - [ ] Durability (новый подписчик получает последнее сообщение)

---

## ОЖИДАЕМЫЙ РЕЗУЛЬТАТ — ПРИМЕР ИСПОЛЬЗОВАНИЯ

```cpp
#include <ecal/ecal.h>
#include <ecal/qos.h>

int main() {
    eCAL::Initialize(argc, argv, "QoS Example");
    
    eCAL::Publisher::Configuration pub_config;
    pub_config.qos.reliability = eCAL::QoS::Reliability::RELIABLE;
    pub_config.qos.priority = eCAL::QoS::Priority::CRITICAL;
    pub_config.qos.deadline_ms = 10;
    
    eCAL::CPublisher pub("critical_commands", data_type_info, pub_config);
    
    eCAL::QoS::Policies msg_qos;
    msg_qos.priority = eCAL::QoS::Priority::HIGH;
    pub.Send(data, length, msg_qos);
    
    eCAL::Subscriber::Configuration sub_config;
    sub_config.qos.min_priority = eCAL::QoS::Priority::NORMAL;
    sub_config.qos.deadline_callback = [](const std::string& topic) {
        std::cerr << "Deadline violated for topic: " << topic << std::endl;
    };
    
    eCAL::CSubscriber sub("critical_commands", sub_config);
    eCAL::Finalize();
    return 0;
}


КЛЮЧЕВЫЕ ФАЙЛЫ ДЛЯ МОДИФИКАЦИИ
Готовые файлы:
ecal/core/include/ecal/qos.h — Enums + QoS::Policies [ГОТОВ]
ecal/core/include/ecal/config/publisher.h — Publisher::Configuration::qos [ГОТОВ]
ecal/core/include/ecal/pubsub/publisher.h — CPublisher::Send(..., QoS) [ГОТОВ]
ecal/core/include/ecal/config/subscriber.h — Subscriber::Configuration::qos [ГОТОВ]
Следующие файлы:
ecal/core/include/ecal/pubsub/subscriber.h — API подписчика [ОЖИДАЕТ]
ecal/core/src/readwrite/ecal_writer_data.h — SWriterAttr::qos [СЛЕДУЮЩИЙ]
Позже:
ecal/core/src/pubsub/ecal_publisher_impl.cpp — Логика приоритетных очередей
ecal/core/src/pubsub/ecal_subscriber_impl.cpp — Проверка deadline, фильтрация
ecal/core/src/io/shm/ecal_memfile_header.h — QoS в заголовке SHM
ecal/core_pb/src/ecal/core/pb/ecal.proto — QoS-поля в protobuf
app/mon/mon_gui/src/widgets/models/topic_tree_model.* — Отображение QoS в GUI


АРХИТЕКТУРА РЕШЕНИЯ
QoS-метаданные передаются в заголовках транспорта, не меняя payload.
Структура заголовка:
struct SMessageQoSHeader {
    uint32_t priority;           // Приоритет сообщения
    uint64_t deadline_timestamp; // Время доставки
    uint8_t  reliability;        // Режим надежности
    uint8_t  durability;         // Режим сохранения
};


ОБРАТНАЯ СОВМЕСТИМОСТЬ
Все изменения должны быть обратно совместимыми:

QoS параметры должны иметь разумные значения по умолчанию (BEST_EFFORT, NORMAL priority, no deadline)
Существующий код без QoS должен продолжать работать без изменений
Сообщения без QoS заголовка должны обрабатываться с дефолтными QoS параметрами




КРИТЕРИИ ПРИЕМКИ
Все существующие тесты проходят
Новые тесты QoS проходят
Пример кода из ТЗ компилируется и работает
eCAL Monitor отображает QoS-параметры (цветовая индикация)
Производительность без QoS не ухудшается >5%
Документация обновлена (комментарии в заголовочных файлах)
История коммитов показывает поэтапную разработку


УТВЕРЖДЁННЫЙ ПЛАН (Variant 1: Protocol-first)
Стратегия:
"QoS-метаданные передаются в заголовках транспорта, не меняя payload; приоритизация и deadline реализуются в ядре, durability — last-value cache; reliability — отдельной фазой."
Почему выбран этот вариант:
Полнота: Покрывает все транспорты (SHM/UDP/TCP)
Совместимость: Использует встроенные механизмы eCAL (SMemFileHeader::hdr_size, protobuf unknown-fields)
Гибкость: Позволяет итеративно развивать: сначала MVP, потом RELIABLE
Сложность: Средняя (без RELIABLE) / Высокая (с RELIABLE)
Файлы для изменения:
Создать:
ecal/core/include/ecal/qos.h [ГОТОВ]
Изменить (готово):
ecal/core/include/ecal/config/publisher.h
ecal/core/include/ecal/pubsub/publisher.h
ecal/core/include/ecal/config/subscriber.h
Изменить (следующие):
ecal/core/src/readwrite/ecal_writer_data.h (SWriterAttr::qos)
ecal/core/src/pubsub/ecal_publisher_impl.cpp (передача qos в writer)
ecal/core/src/pubsub/ecal_subscriber_impl.cpp (проверка deadline, фильтрация)
Изменить (транспорт):
ecal/core/src/io/shm/ecal_memfile_header.h (QoS в заголовке)
ecal/core/src/serialization/ecal_serialize_sample_payload.cpp
ecal/core_pb/src/ecal/core/pb/ecal.proto (protobuf-поля)
Изменить (мониторинг):
app/mon/mon_gui/src/widgets/models/topic_tree_model.*
app/mon/mon_gui/src/widgets/models/topic_tree_item.*
Что тестируем в первую очередь:
[ГОТОВО] "QoS metadata end-to-end" (publish -> transport -> subscriber extracts) для SHM/UDP/TCP
[ОЖИДАЕТ] Приоритизация: доставка CRITICAL раньше LOW при конкуренции
[ОЖИДАЕТ] Deadline violation: задержка > deadline -> callback срабатывает
[ОЖИДАЕТ] Durability: новый subscriber получает last sample при подключении
ТЕКУЩИЙ СТАТУС
[2026-04-26] Этап 1: Публичные заголовки (API) — ЗАВЕРШЁН
Выполнено:
ecal/core/include/ecal/qos.h — создан: enums + struct Policies с русскими комментариями
ecal/core/include/ecal/config/publisher.h — добавлено поле qos в конец Configuration
ecal/core/include/ecal/pubsub/publisher.h — добавлена перегрузка Send(..., const QoS::Policies&)
ecal/core/include/ecal/config/subscriber.h — добавлено поле qos в конец Configuration

Коммиты:
c0c2f8b917f31e87b5526d93de0859b1601478c6  QoS: add qos field to Subscriber config
9aa6171ea0a7ec2189d12b2f46dbd6e6ee53c09d  QoS: add qos field to Publisher config and overloaded Send()
bd2ff69bcae23aa3f5a59895b35678ff17d7a488  QoS: add qos.h with enums and Policies struct

Важные решения:
Все поля qos добавлены в конец структур для ABI-совместимости
Комментарии на русском для удобства защиты
min_priority и deadline_callback не дублируются — они внутри QoS::Policies
[2026-04-27] Этап 2: Внутренняя передача QoS (ядро) — В РАБОТЕ
Текущая задача:
ecal/core/src/readwrite/ecal_writer_data.h — расширить SWriterAttr полем qos
ecal/core/src/pubsub/ecal_publisher_impl.cpp — передача qos из Configuration в SWriterAttr
Пример кода (черновик):
ecal_writer_data.h:
struct SWriterAttr
{
  // ... старые поля ...
  eCAL::QoS::Policies qos{};  // Политики QoS для этого сообщения
};

ecal_publisher_impl.cpp:
bool CPublisherImpl::Write(..., const eCAL::QoS::Policies* msg_qos_ = nullptr)
{
  SWriterAttr attr;
  // ...
  attr.qos = (msg_qos_ != nullptr) ? *msg_qos_ : m_config.qos;
  // ...
}

Следующие задачи:
Реализация приоритетных очередей (замена FIFO)
Проверка deadline в ecal_subscriber_impl.cpp
Механизм durability (кэш последнего сообщения)
[Планируется] Этап 3: Транспортные слои
SHM: добавить QoS в SMemFileHeader (в конец, увеличить hdr_size)
UDP: раздельные очереди по приоритету
TCP: надёжная доставка с подтверждениями (RELIABLE — отдельная фаза)
Протокол: добавить QoS-поля в ecal.proto (protobuf unknown-fields для совместимости)
[Планируется] Этап 4: Мониторинг и тесты
app/mon/mon_gui/ — колонка "QoS Priority" с цветами
Счётчики deadline violations
Тесты: приоритет, deadline, durability, reliability
