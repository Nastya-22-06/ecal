#pragma once

#include <QWidget>
#include <array>

// Конкретные заголовки Qt Charts (вместо #include <QtCharts>)
#include <QtCharts/QChart>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChartView>

#include <ecal/types/monitoring.h>

class PriorityDistributionWidget : public QWidget
{
  Q_OBJECT

public:
  explicit PriorityDistributionWidget(QWidget* parent = Q_NULLPTR);

public slots:
  void updateFromMonitoring(const eCAL::Monitoring::SMonitoring& monitoring);

private:
  enum class PriorityBucket : int
  {
    BACKGROUND = 0,
    LOW,
    NORMAL,
    HIGH,
    CRITICAL,
    COUNT
  };

  QChart*           chart_;
  QChartView*       chart_view_;
  QBarSeries*       bar_series_;
  QBarCategoryAxis* axis_x_;
  QValueAxis*       axis_y_;

  QBarSet* bar_background_;
  QBarSet* bar_low_;
  QBarSet* bar_normal_;
  QBarSet* bar_high_;
  QBarSet* bar_critical_;

  static PriorityBucket mapPriority(uint32_t qos_priority);
  static void accumulateTopicPriorities(const std::vector<eCAL::Monitoring::STopic>& topics, std::array<int, static_cast<int>(PriorityBucket::COUNT)>& counters);
  void applyCountersToChart(const std::array<int, static_cast<int>(PriorityBucket::COUNT)>& counters);
};

