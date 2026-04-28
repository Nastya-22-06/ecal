#include "priority_distribution_widget.h"

#include <QColor>
#include <QVBoxLayout>

#include <ecal/qos.h>

namespace
{
  constexpr int kCategoryCount = 5;
}

PriorityDistributionWidget::PriorityDistributionWidget(QWidget* parent)
  : QWidget(parent)
  , chart_(new QChart())
  , chart_view_(new QChartView(chart_, this))
  , bar_series_(new QBarSeries())
  , axis_x_(new QBarCategoryAxis())
  , axis_y_(new QValueAxis())
  , bar_background_(new QBarSet("BACKGROUND"))
  , bar_low_(new QBarSet("LOW"))
  , bar_normal_(new QBarSet("NORMAL"))
  , bar_high_(new QBarSet("HIGH"))
  , bar_critical_(new QBarSet("CRITICAL"))
{
  // Задаем цвета столбцов по согласованной QoS-палитре.
  bar_critical_->setColor(QColor(255, 0, 0));
  bar_high_->setColor(QColor(255, 165, 0));
  bar_normal_->setColor(QColor(0, 200, 0));
  bar_low_->setColor(QColor(128, 128, 128));
  bar_background_->setColor(QColor(128, 128, 128));

  // Для каждой категории заполняем только свой индекс.
  *bar_background_ << 0 << 0 << 0 << 0 << 0;
  *bar_low_        << 0 << 0 << 0 << 0 << 0;
  *bar_normal_     << 0 << 0 << 0 << 0 << 0;
  *bar_high_       << 0 << 0 << 0 << 0 << 0;
  *bar_critical_   << 0 << 0 << 0 << 0 << 0;

  bar_series_->append(bar_background_);
  bar_series_->append(bar_low_);
  bar_series_->append(bar_normal_);
  bar_series_->append(bar_high_);
  bar_series_->append(bar_critical_);

  chart_->addSeries(bar_series_);
  chart_->setTitle("QoS Priority Distribution");
  chart_->legend()->hide();

  axis_x_->append({ "BACKGROUND", "LOW", "NORMAL", "HIGH", "CRITICAL" });
  chart_->addAxis(axis_x_, Qt::AlignBottom);
  bar_series_->attachAxis(axis_x_);

  axis_y_->setLabelFormat("%d");
  axis_y_->setTitleText("Количество сообщений");
  axis_y_->setRange(0, 1);
  chart_->addAxis(axis_y_, Qt::AlignLeft);
  bar_series_->attachAxis(axis_y_);

  chart_view_->setRenderHint(QPainter::Antialiasing);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(chart_view_);
  setLayout(layout);
}

void PriorityDistributionWidget::updateFromMonitoring(const eCAL::Monitoring::SMonitoring& monitoring)
{
  std::array<int, static_cast<int>(PriorityBucket::COUNT)> counters{ 0, 0, 0, 0, 0 };

  accumulateTopicPriorities(monitoring.publishers, counters);
  accumulateTopicPriorities(monitoring.subscribers, counters);

  applyCountersToChart(counters);
}

PriorityDistributionWidget::PriorityBucket PriorityDistributionWidget::mapPriority(uint32_t qos_priority)
{
  const auto priority = static_cast<eCAL::QoS::Priority>(qos_priority);
  switch (priority)
  {
  case eCAL::QoS::Priority::CRITICAL:
    return PriorityBucket::CRITICAL;
  case eCAL::QoS::Priority::HIGH:
    return PriorityBucket::HIGH;
  case eCAL::QoS::Priority::LOW:
    return PriorityBucket::LOW;
  case eCAL::QoS::Priority::BACKGROUND:
    return PriorityBucket::BACKGROUND;
  case eCAL::QoS::Priority::NORMAL:
  default:
    return PriorityBucket::NORMAL;
  }
}

void PriorityDistributionWidget::accumulateTopicPriorities(const std::vector<eCAL::Monitoring::STopic>& topics, std::array<int, static_cast<int>(PriorityBucket::COUNT)>& counters)
{
  for (const auto& topic : topics)
  {
    // При qos_priority == 0 считаем, что данных QoS нет, и пропускаем запись.
    if (topic.qos_priority == 0)
    {
      continue;
    }

    const auto bucket = mapPriority(topic.qos_priority);
    counters[static_cast<int>(bucket)]++;
  }
}

void PriorityDistributionWidget::applyCountersToChart(const std::array<int, static_cast<int>(PriorityBucket::COUNT)>& counters)
{
  for (int i = 0; i < kCategoryCount; ++i)
  {
    bar_background_->replace(i, 0.0);
    bar_low_->replace(i, 0.0);
    bar_normal_->replace(i, 0.0);
    bar_high_->replace(i, 0.0);
    bar_critical_->replace(i, 0.0);
  }

  bar_background_->replace(static_cast<int>(PriorityBucket::BACKGROUND), static_cast<qreal>(counters[static_cast<int>(PriorityBucket::BACKGROUND)]));
  bar_low_->replace(static_cast<int>(PriorityBucket::LOW), static_cast<qreal>(counters[static_cast<int>(PriorityBucket::LOW)]));
  bar_normal_->replace(static_cast<int>(PriorityBucket::NORMAL), static_cast<qreal>(counters[static_cast<int>(PriorityBucket::NORMAL)]));
  bar_high_->replace(static_cast<int>(PriorityBucket::HIGH), static_cast<qreal>(counters[static_cast<int>(PriorityBucket::HIGH)]));
  bar_critical_->replace(static_cast<int>(PriorityBucket::CRITICAL), static_cast<qreal>(counters[static_cast<int>(PriorityBucket::CRITICAL)]));

  int max_value = 1;
  for (const int value : counters)
  {
    if (value > max_value)
    {
      max_value = value;
    }
  }
  axis_y_->setRange(0, max_value);
}

