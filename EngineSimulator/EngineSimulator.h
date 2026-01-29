#ifndef ENGINESIMULATOR_H
#define ENGINESIMULATOR_H

#include <QtWidgets/QMainWindow>
#include "ui_EngineSimulator.h"
#include "DataGenerator.h"
#include "Logger.h"
#include "EngineState.h"
#include "GaugeWidget.h"
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class EngineSimulatorClass; }
QT_END_NAMESPACE

class EngineSimulator : public QMainWindow
{
    Q_OBJECT

public:
	// 构造与析构函数
    explicit EngineSimulator(QWidget* parent = nullptr);
    ~EngineSimulator();


// 在Qt中，加slots关键字表示该函数是一个槽函数，可以响应信号
private slots:
    // 按钮响应槽函数
    void onStartButtonClicked();
    void onStopButtonClicked();


    // 5ms定时器触发
    void onSimulateTimerTimeout(); 

	// 1秒定时器触发，用于更新UI显示
	void onDisplayTimerTimeout();

	// UI显示更新函数
    void updateDisplay();

	// 检查传入数据函数
	void checkData();

	// 推力控制按钮响应槽函数
    void onIncreaseThrustClicked();
    void onDecreaseThrustClicked();

	// 传感器异常情况按钮响应槽函数
	void onN1LS1FailClicked();
	void onN1RS1FailClicked();
	void onN1LS2FailClicked();
	void onN1RS2FailClicked();
	void onEGTLS1FailClicked();
	void onEGTRS1FailClicked();
	void onEGTLS2FailClicked();
	void onEGTRS2FailClicked();
	void onN1SFailClicked();
	void onEGTSFailClicked();
	// 传感器故障状态更新
	void updateSensor();

	// 速度异常情况按钮响应槽函数
	void onN1OverSpeed1Clicked();
	void onN1OverSpeed2Clicked();
	void onEGTOverSpeed1Clicked();
	void onEGTOverSpeed2Clicked();
	void onEGTOverSpeed3Clicked();
	void onEGTOverSpeed4Clicked();
	void onFFOverSpeedClicked();
	// 速度相关的显示更新
	void updateSpeed();
	void resetSpeed();

	// 燃油异常情况按钮响应槽函数
	void onFuelSFailClicked();
	void updateFuelFlowDisplay();
	void onLowFuelClicked();

	// 初始化异常按钮状态
	void initAnomalyState();
	

private:
	// --- 成员变量 ---
    Ui::EngineSimulatorClass* ui;

	// 全局时间
	double TheGlobalTime;
	// 用于在后台模拟数据生成，5ms更新一次
    QTimer* simulationTimer;
	// 用于在UI显示，1秒更新一次更合理
	QTimer* displayTimer;

	// 数据生成器
    DataGenerator generator;

	// 日志记录器
	Logger logger;

	// 当前数据
	SensorData currentData;

	// 仪表盘控件
    GaugeWidget* n1LeftGauge;
    GaugeWidget* n1RightGauge;
    GaugeWidget* egtLeftGauge;
    GaugeWidget* egtRightGauge;
    GaugeWidget* fuelLevelGauge;

	// 异常状态
	AnomalyState anomalyState;
};

#endif

