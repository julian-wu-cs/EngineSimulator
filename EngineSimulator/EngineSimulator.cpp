#include "EngineSimulator.h"
#include "DataGenerator.h"
#include "Logger.h"
#include "EngineState.h"
#include <QTimer>
#include <QDebug>

// 构造函数
EngineSimulator::EngineSimulator(QWidget* parent)
	: QMainWindow(parent), TheGlobalTime(0.0)
	, ui(new Ui::EngineSimulatorClass)
    , simulationTimer(new QTimer(this))
    , displayTimer(new QTimer(this)), anomalyState()
{
	// 设置UI
    ui->setupUi(this);

    // 5ms 定时器 - 数据生成
    simulationTimer->setInterval(5);
    connect(simulationTimer, &QTimer::timeout, this, &EngineSimulator::onSimulateTimerTimeout);

    // 1秒 定时器 - UI 显示
    displayTimer->setInterval(1000);
    connect(displayTimer, &QTimer::timeout, this, &EngineSimulator::onDisplayTimerTimeout);

    // 连接控制台按钮
    connect(ui->btnStart, &QPushButton::clicked, this, &EngineSimulator::onStartButtonClicked);
    connect(ui->btnStop, &QPushButton::clicked, this, &EngineSimulator::onStopButtonClicked);
    connect(ui->btnIncreaseThrust, &QPushButton::clicked, this, &EngineSimulator::onIncreaseThrustClicked);
    connect(ui->btnDecreaseThrust, &QPushButton::clicked, this, &EngineSimulator::onDecreaseThrustClicked);


    // 传感器异常情况按钮连接
    connect(ui->N1LS1Fail, &QPushButton::clicked, this, &EngineSimulator::onN1LS1FailClicked);
    connect(ui->N1RS1Fail, &QPushButton::clicked, this, &EngineSimulator::onN1RS1FailClicked);
    connect(ui->N1LS2Fail, &QPushButton::clicked, this, &EngineSimulator::onN1LS2FailClicked);
    connect(ui->N1RS2Fail, &QPushButton::clicked, this, &EngineSimulator::onN1RS2FailClicked);
    connect(ui->EGTLS1Fail, &QPushButton::clicked, this, &EngineSimulator::onEGTLS1FailClicked);
    connect(ui->EGTRS1Fail, &QPushButton::clicked, this, &EngineSimulator::onEGTRS1FailClicked);
    connect(ui->EGTLS2Fail, &QPushButton::clicked, this, &EngineSimulator::onEGTLS2FailClicked);
    connect(ui->EGTRS2Fail, &QPushButton::clicked, this, &EngineSimulator::onEGTRS2FailClicked);
    connect(ui->N1SFail, &QPushButton::clicked, this, &EngineSimulator::onN1SFailClicked);
    connect(ui->EGTSFail, &QPushButton::clicked, this, &EngineSimulator::onEGTSFailClicked);

	// 速度异常情况按钮连接
    connect(ui->N1OverSpeed1, &QPushButton::clicked, this, &EngineSimulator::onN1OverSpeed1Clicked);
    connect(ui->N1OverSpeed2, &QPushButton::clicked, this, &EngineSimulator::onN1OverSpeed2Clicked);
    connect(ui->EGTOverSpeed1, &QPushButton::clicked, this, &EngineSimulator::onEGTOverSpeed1Clicked);
    connect(ui->EGTOverSpeed2, &QPushButton::clicked, this, &EngineSimulator::onEGTOverSpeed2Clicked);
    connect(ui->EGTOverSpeed3, &QPushButton::clicked, this, &EngineSimulator::onEGTOverSpeed3Clicked);
    connect(ui->EGTOverSpeed4, &QPushButton::clicked, this, &EngineSimulator::onEGTOverSpeed4Clicked);

	// 燃油相关异常按钮连接
    connect(ui->FuelSFail, &QPushButton::clicked, this, &EngineSimulator::onFuelSFailClicked);
    connect(ui->LowFuel, &QPushButton::clicked, this, &EngineSimulator::onLowFuelClicked);
    connect(ui->FFOverSpeed, &QPushButton::clicked, this, &EngineSimulator::onFFOverSpeedClicked);

    // 初始状态
    ui->labelStartIndicator->setStyleSheet("background-color: gray;");
    ui->labelRunIndicator->setStyleSheet("background-color: gray;");

	// 仪表盘初始化
    n1LeftGauge = new GaugeWidget(GaugeWidget::N1, this);
    n1RightGauge = new GaugeWidget(GaugeWidget::N1, this);
    egtLeftGauge = new GaugeWidget(GaugeWidget::EGT, this);
    egtRightGauge = new GaugeWidget(GaugeWidget::EGT, this);
    fuelLevelGauge = new GaugeWidget(GaugeWidget::FUEL_LEVEL, this);

	// 将仪表盘添加到布局中
    ui->layoutN1Left->addWidget(n1LeftGauge);
    ui->layoutN1Right->addWidget(n1RightGauge);
    ui->layoutEGTLeft->addWidget(egtLeftGauge);
    ui->layoutEGTRight->addWidget(egtRightGauge);
    ui->layoutFuelLevel->addWidget(fuelLevelGauge);
}

// 析构函数
EngineSimulator::~EngineSimulator()
{
    delete ui;  // 释放内存
}

// 启动按钮响应函数
void EngineSimulator::onStartButtonClicked() {
    if (!simulationTimer->isActive() && !(anomalyState.N1LS1Fail && anomalyState.N1LS2Fail && anomalyState.N1RS1Fail && anomalyState.N1RS2Fail) &&
        !(anomalyState.EGTLS1Fail && anomalyState.EGTLS2Fail && anomalyState.EGTRS1Fail && anomalyState.EGTRS2Fail)) {
        generator.startSequence();
        TheGlobalTime = 0.0;
        simulationTimer->start();
        displayTimer->start();
        logger.initLogFile();
    }
}

// 停车按钮响应函数 - 最高优先级
void EngineSimulator::onStopButtonClicked() {
    generator.stopSequence();
}

// 异常情况按钮响应函数
void EngineSimulator::onN1LS1FailClicked() {
    if (anomalyState.N1LS1Fail) {
        anomalyState.N1LS1Fail = false;
    }
    else {
        anomalyState.N1LS1Fail = true;
    }
    updateSensor();
}
void EngineSimulator::onN1RS1FailClicked() {
    if (anomalyState.N1RS1Fail) {
        anomalyState.N1RS1Fail = false;
    }
    else {
        anomalyState.N1RS1Fail = true;
    }
    updateSensor();
}
void EngineSimulator::onN1LS2FailClicked() {
    if (anomalyState.N1LS2Fail) {
        anomalyState.N1LS2Fail = false;
    }
    else {
        anomalyState.N1LS2Fail = true;
    }
    updateSensor();
}
void EngineSimulator::onN1RS2FailClicked() {
    if (anomalyState.N1RS2Fail) {
        anomalyState.N1RS2Fail = false;
    }
    else {
        anomalyState.N1RS2Fail = true;
    }
    updateSensor();
}
void EngineSimulator::onEGTLS1FailClicked() {
    if (anomalyState.EGTLS1Fail) {
        anomalyState.EGTLS1Fail = false;
    }
    else {
        anomalyState.EGTLS1Fail = true;
    }
    updateSensor();
}
void EngineSimulator::onEGTRS1FailClicked() {
    if (anomalyState.EGTRS1Fail) {
        anomalyState.EGTRS1Fail = false;
    }
    else {
        anomalyState.EGTRS1Fail = true;
    }
    updateSensor();
}
void EngineSimulator::onEGTLS2FailClicked() {
    if (anomalyState.EGTLS2Fail) {
        anomalyState.EGTLS2Fail = false;
    }
    else {
        anomalyState.EGTLS2Fail = true;
    }
    updateSensor();
}
void EngineSimulator::onEGTRS2FailClicked() {
    if (anomalyState.EGTRS2Fail) {
        anomalyState.EGTRS2Fail = false;
    }
    else {
        anomalyState.EGTRS2Fail = true;
    }
    updateSensor();
}
void EngineSimulator::onN1SFailClicked() {
    if (anomalyState.N1LS1Fail && anomalyState.N1LS2Fail
        && anomalyState.N1RS1Fail && anomalyState.N1RS2Fail) {
        anomalyState.N1LS1Fail = false;
        anomalyState.N1RS1Fail = false;
        anomalyState.N1LS2Fail = false;
        anomalyState.N1RS2Fail = false;
    }
    else {
        anomalyState.N1LS1Fail = true;
        anomalyState.N1RS1Fail = true;
        anomalyState.N1LS2Fail = true;
        anomalyState.N1RS2Fail = true;
    }
    updateSensor();
}
void EngineSimulator::onEGTSFailClicked() {
    if (anomalyState.EGTLS1Fail && anomalyState.EGTRS1Fail
        && anomalyState.EGTLS2Fail && anomalyState.EGTRS2Fail) {
        anomalyState.EGTLS1Fail = false;
        anomalyState.EGTRS1Fail = false;
        anomalyState.EGTLS2Fail = false;
        anomalyState.EGTRS2Fail = false;
    }
    else {
        anomalyState.EGTLS1Fail = true;
        anomalyState.EGTRS1Fail = true;
        anomalyState.EGTLS2Fail = true;
        anomalyState.EGTRS2Fail = true;
    }
    updateSensor();
}
void EngineSimulator::onFuelSFailClicked() {
    if (anomalyState.FuelSFail) {
        anomalyState.FuelSFail = false;
    }
    else {
        anomalyState.FuelSFail = true;
    }
    updateSensor();
}

// N1超转1
void EngineSimulator::onN1OverSpeed1Clicked() {
    if (currentData.phase != EnginePhase::Stopping && currentData.phase != EnginePhase::Idle) {
        if (anomalyState.N1OverSpeedLevel == 1) {
            anomalyState.N1OverSpeedLevel = 0;
            currentData.n1LeftAverage = currentData.n1RightAverage = 100.0;
            currentData.phase = EnginePhase::Stable;
            generator.updateData(currentData);
        }
        else {
            anomalyState.N1OverSpeedLevel = 1;
            currentData.n1LeftAverage = currentData.n1RightAverage = 110.0;
            currentData.phase = EnginePhase::Stable;
            generator.updateData(currentData);
        }
    }
    updateSpeed();
}

// N1超转2
void EngineSimulator::onN1OverSpeed2Clicked() {
    if (currentData.phase != EnginePhase::Stopping && currentData.phase != EnginePhase::Idle) {
        if (anomalyState.N1OverSpeedLevel != 2) {
            anomalyState.N1OverSpeedLevel = 2;
            currentData.n1LeftAverage = currentData.n1RightAverage = 124.0;
            generator.updateData(currentData);
			onStopButtonClicked(); // 触发停车
        }
    }
    updateSpeed();

}

// EGT超温1和2只能在启动阶段触发
void EngineSimulator::onEGTOverSpeed1Clicked() {
    if (currentData.phase == EnginePhase::Starting) {
        if (anomalyState.EGTOverSpeedLevel == 1) {
            anomalyState.EGTOverSpeedLevel = 0;
            currentData.EGTOverSpeed1 = false;
            generator.updateData(currentData);
        }
        else {
            anomalyState.EGTOverSpeedLevel = 1;
            currentData.EGTOverSpeed1 = true;
            generator.updateData(currentData);
        }
    }
    updateSpeed();
}
void EngineSimulator::onEGTOverSpeed2Clicked() {
    if (currentData.phase == EnginePhase::Starting) {
        if (anomalyState.EGTOverSpeedLevel != 2) {
            anomalyState.EGTOverSpeedLevel = 2;
            currentData.egtLeftAverage = currentData.egtRightAverage = 1100.0;
            generator.updateData(currentData);
            onStopButtonClicked(); // 触发停车
        }
    }
    updateSpeed();
}

// EGT超温3和4只能在稳定阶段触发
void EngineSimulator::onEGTOverSpeed3Clicked() {
    if (currentData.phase == EnginePhase::Stable) {
        if (anomalyState.EGTOverSpeedLevel == 3) {
            anomalyState.EGTOverSpeedLevel = 0;
            currentData.egtLeftAverage = currentData.egtRightAverage = 720.0;
            generator.updateData(currentData);
        }
        else {
            anomalyState.EGTOverSpeedLevel = 3;
            currentData.egtLeftAverage = currentData.egtRightAverage = 1000.0;
            generator.updateData(currentData);
        }

    }
    updateSpeed();
}
void EngineSimulator::onEGTOverSpeed4Clicked() {
    if (currentData.phase == EnginePhase::Stable) {
        if (anomalyState.EGTOverSpeedLevel != 4) {
            anomalyState.EGTOverSpeedLevel = 4;
            currentData.egtLeftAverage = currentData.egtRightAverage = 1170.0;
            generator.updateData(currentData);
            onStopButtonClicked();
        }
    }
    updateSpeed();
}

// 更新燃油流量显示
void EngineSimulator::updateFuelFlowDisplay() {
    // 动态设置颜色：>50 为琥珀色，否则为白色
    if (currentData.fuelFlow > 50.0) {
        ui->lcdFuelFlow->setText("Over Speed!");
        ui->lcdFuelFlow->setStyleSheet(
            "color: #FFBF00; "                      // 琥珀色
            "font-family: \"Courier New\", monospace; "
            "font-size: 24pt; "
            "font-weight: bold; "
            "background-color: black; "
            "border: 1px solid #444; "
            "padding: 5px; "
            "text-align: center;"
        );
    }
    else {
        QString flowText = QString::number(currentData.fuelFlow, 'f', 1);
        ui->lcdFuelFlow->setText(flowText);
        ui->lcdFuelFlow->setStyleSheet(
            "color: white; "
            "font-family: \"Courier New\", monospace; "
            "font-size: 24pt; "
            "font-weight: bold; "
            "background-color: black; "
            "border: 1px solid #444; "
            "padding: 5px; "
            "text-align: center;"
        );
    }
}

// 低燃油状态切换
void EngineSimulator::onLowFuelClicked() {
    if (anomalyState.LowFuel) {
        anomalyState.LowFuel = false;
        currentData.fuelLevel += 19001.0; // 恢复正常燃油状态
        generator.updateData(currentData);
        fuelLevelGauge->updateValue(currentData.fuelLevel);
    }
    else {
        anomalyState.LowFuel = true;
        currentData.fuelLevel -= 19001.0; // 设置为低燃油状态
        generator.updateData(currentData);
        fuelLevelGauge->updateValue(currentData.fuelLevel);
    }
    updateSensor();
}

// 燃油流速超速切换
void EngineSimulator::onFFOverSpeedClicked() {
    if (currentData.phase == EnginePhase::Stable) {
        if (anomalyState.FFOverSpeed) {
            anomalyState.FFOverSpeed = false;
            currentData.fuelFlow = 40.0;
            generator.updateData(currentData);
        }
        else {
            anomalyState.FFOverSpeed = true;
            currentData.fuelFlow = 60.0; // 设置为高燃油流速状态
            generator.updateData(currentData);
        }
    }
    updateSpeed();
}

// 增大推力
void EngineSimulator::onIncreaseThrustClicked() {
    if (currentData.phase == EnginePhase::Stable) {
        generator.setIncreaseThrust(true);
    }
}

// 减小推力
void EngineSimulator::onDecreaseThrustClicked() {
    if (currentData.phase == EnginePhase::Stable) {
        generator.setDecreaseThrust(true);
    }
}

// 5ms定时器触发函数
void EngineSimulator::onSimulateTimerTimeout() {
    // 全局时间更新
	TheGlobalTime += 0.005; 

	// 数据更新
    generator.updateData();
    currentData = generator.getData();

	// 检查数据异常
    checkData();

	// UI显示更新
    updateDisplay();

	// 日志记录与告警处理
    logger.logDataAndAlerts(TheGlobalTime,generator.processData(currentData), anomalyState, ui->alertLogDisplay);

	// 停车完成后停止定时器
    if (currentData.phase == EnginePhase::Idle) {
        initAnomalyState();
        simulationTimer->stop();
        displayTimer->stop();
    }
}

// 1秒定时器触发函数
void EngineSimulator::onDisplayTimerTimeout() {
    // 定义各数据字段的有效性判断函数（内联函数，简洁易维护）
	bool isValidN1L = !(anomalyState.N1LS1Fail && anomalyState.N1LS2Fail); // N1是百分比，0-120
	bool isValidN1R = !(anomalyState.N1RS1Fail && anomalyState.N1RS2Fail); // N1是百分比，0-120
	bool isValidEGTL = !(anomalyState.EGTLS1Fail && anomalyState.EGTLS2Fail); // EGT合理范围20-1200℃
	bool isValidEGTR = !(anomalyState.EGTRS1Fail && anomalyState.EGTRS2Fail); // EGT合理范围20-1200℃
	bool isValidFuelS = !anomalyState.FuelSFail; // 燃油传感器故障
	bool isValidFuelFlow = !anomalyState.FFOverSpeed; // 燃油流速超速

	// 处理数据以获取UI显示值
	SensorData uiData = generator.processData(currentData);

    // 逐个字段处理：有效则格式化数值，无效则显示N/A
    QString timeStr = QString::number(TheGlobalTime, 'f', 2);
    QString n1LeftStr = isValidN1L ? QString::number(uiData.n1LeftAverage, 'f', 2) : "N/A"; 
    QString n1RightStr = isValidN1R ? QString::number(uiData.n1RightAverage, 'f', 2) : "N/A";
    QString egtLeftStr = isValidEGTL ? QString::number(uiData.egtLeftAverage, 'f', 1) : "N/A";
    QString egtRightStr = isValidEGTR ? QString::number(uiData.egtRightAverage, 'f', 1) : "N/A";
    QString fuelLevelStr = QString::number(uiData.fuelLevel, 'f', 1);
    QString fuelFlowStr = QString::number(uiData.fuelFlow, 'f', 1);
    QString phaseStr = QString::number(static_cast<int>(uiData.phase));

    // 拼接日志字符串（无效字段会显示N/A）
    QString log = QString("Time: %1s | N1 Left: %2% | N1 Right: %3% | EGT Left: %4 \u2103 | EGT Right: %5 \u2103 | Fuel Level: %6 lbs | Fuel Flow: %7 lbs/hr | Phase: %8")
        .arg(timeStr)
        .arg(n1LeftStr)
        .arg(n1RightStr)
        .arg(egtLeftStr)
        .arg(egtRightStr)
        .arg(fuelLevelStr)
        .arg(fuelFlowStr)
        .arg(phaseStr);

    ui->dataLogDisplay->append(log);
}

// UI显示更新函数
void EngineSimulator::updateDisplay() {
	// 更新启动和运行指示灯
    if (currentData.phase == EnginePhase::Starting) {
        ui->labelStartIndicator->setStyleSheet("background-color: green;");
        ui->labelRunIndicator->setStyleSheet("background-color: gray;");
    }
    else if (currentData.phase == EnginePhase::Stable) {
        ui->labelStartIndicator->setStyleSheet("background-color: gray;");
        ui->labelRunIndicator->setStyleSheet("background-color: green;");
    }
    else {
        ui->labelStartIndicator->setStyleSheet("background-color: gray;");
        ui->labelRunIndicator->setStyleSheet("background-color: gray;");
    }

    // 更新仪表数字显示
    n1LeftGauge->updateValue(currentData.n1LeftAverage);
    n1RightGauge->updateValue(currentData.n1RightAverage);
    egtLeftGauge->updateValue(currentData.egtLeftAverage);
    egtRightGauge->updateValue(currentData.egtRightAverage);
    fuelLevelGauge->updateValue(currentData.fuelLevel);

    // 更新燃油流速显示
    updateFuelFlowDisplay();

}

// 数据异常检查函数
void EngineSimulator::checkData() {
    bool updateSensorFlag = false;
    bool updateSpeedFlag = false;

    // N1 超转检测
    if ((currentData.n1LeftAverage > 120.0 || currentData.n1RightAverage > 120.0)
        && anomalyState.N1OverSpeedLevel != 2) {
        anomalyState.N1OverSpeedLevel = 2;
        updateSpeedFlag = true;
        onStopButtonClicked();
    }
    else if ((currentData.n1LeftAverage > 105.0 || currentData.n1RightAverage > 105.0)
        && anomalyState.N1OverSpeedLevel != 1) {
        if (anomalyState.N1OverSpeedLevel == 0) { // 避免重复触发
        }
        anomalyState.N1OverSpeedLevel = 1;
        updateSpeedFlag = true;
    }
    else if ((currentData.n1LeftAverage <= 105.0 && currentData.n1RightAverage <= 105.0)
        && anomalyState.N1OverSpeedLevel != 0) {
        anomalyState.N1OverSpeedLevel = 0;
        updateSpeedFlag = true;
    }

    // EGT 超温检测
    bool inStarting = (currentData.phase == EnginePhase::Starting) ||
        (currentData.lastPhase == EnginePhase::Starting && currentData.phase == EnginePhase::Stopping);
    bool inStable = (currentData.phase == EnginePhase::Stable) ||
        (currentData.lastPhase == EnginePhase::Stable && currentData.phase == EnginePhase::Stopping);

    if (inStarting) {
        if ((currentData.egtLeftAverage > 1000.0 || currentData.egtRightAverage > 1000.0)
            && anomalyState.EGTOverSpeedLevel != 2) {
            anomalyState.EGTOverSpeedLevel = 2;
            generator.updateData(currentData);
            updateSpeedFlag = true;
            onStopButtonClicked();
        }
        else if ((currentData.egtLeftAverage > 850.0 || currentData.egtRightAverage > 850.0)
            && anomalyState.EGTOverSpeedLevel != 1) {
            if (anomalyState.EGTOverSpeedLevel == 0) {
            }
            anomalyState.EGTOverSpeedLevel = 1;
            updateSpeedFlag = true;
        }
        else if ((currentData.egtLeftAverage <= 850.0 && currentData.egtRightAverage <= 850.0)
            && anomalyState.EGTOverSpeedLevel != 0) {
            anomalyState.EGTOverSpeedLevel = 0;
            updateSpeedFlag = true;
        }
    }
    else if (inStable) {
        if ((currentData.egtLeftAverage > 1100.0 || currentData.egtRightAverage > 1100.0)
            && anomalyState.EGTOverSpeedLevel != 4) {
            anomalyState.EGTOverSpeedLevel = 4;
            generator.updateData(currentData);
            updateSpeedFlag = true;
            onStopButtonClicked();
        }
        else if ((currentData.egtLeftAverage > 950.0 || currentData.egtRightAverage > 950.0)
            && anomalyState.EGTOverSpeedLevel != 3) {
            if (anomalyState.EGTOverSpeedLevel == 0) {
            }
            anomalyState.EGTOverSpeedLevel = 3;
            updateSpeedFlag = true;
        }
        else if ((currentData.egtLeftAverage <= 950.0 && currentData.egtRightAverage <= 950.0)
            && anomalyState.EGTOverSpeedLevel != 0) {
            anomalyState.EGTOverSpeedLevel = 0;
            updateSpeedFlag = true;
        }
    }

    // 燃油流速超限检测
    if (currentData.fuelFlow > 50.0 && !anomalyState.FFOverSpeed) {
        anomalyState.FFOverSpeed = true;
        updateSpeedFlag = true;
    }
    else if (currentData.fuelFlow <= 50.0 && anomalyState.FFOverSpeed) {
        anomalyState.FFOverSpeed = false;
        updateSpeedFlag = true;
    }

    // 燃油余量过低检测
    if (currentData.fuelLevel < 1000.0 && !anomalyState.LowFuel) {
        anomalyState.LowFuel = true;
        updateSensorFlag = true;
    }
    else if (currentData.fuelLevel >= 1000.0 && anomalyState.LowFuel) {
        anomalyState.LowFuel = false;
        updateSensorFlag = true;
    }

	// 注意：燃油流速检测被集成在了 updateFuelFlowDisplay() 中

    // 更新 UI
    if (updateSensorFlag) updateSensor();
    if (updateSpeedFlag) updateSpeed();
}

// 传感器故障状态更新函数
void EngineSimulator::updateSensor() {
    // 处理 N1 系列按钮
    // 判断 N1 总故障
    if (anomalyState.N1LS1Fail && anomalyState.N1LS2Fail &&
        anomalyState.N1RS1Fail && anomalyState.N1RS2Fail) {
        // N1 总故障按钮 + 4个传感器按钮 全红
        ui->N1SFail->setChecked(true);
        ui->N1SFail->setStyleSheet(CHECKED_BTN_STYLE_RED);

        ui->N1LS1Fail->setChecked(true);
        ui->N1LS1Fail->setStyleSheet(CHECKED_BTN_STYLE_RED);

        ui->N1LS2Fail->setChecked(true);
        ui->N1LS2Fail->setStyleSheet(CHECKED_BTN_STYLE_RED);

        ui->N1RS1Fail->setChecked(true);
        ui->N1RS1Fail->setStyleSheet(CHECKED_BTN_STYLE_RED);

        ui->N1RS2Fail->setChecked(true);
        ui->N1RS2Fail->setStyleSheet(CHECKED_BTN_STYLE_RED);

        // N1 左右仪表设为红色报警
        n1LeftGauge->updateLevel(AlertLevel::redWarning);
        n1RightGauge->updateLevel(AlertLevel::redWarning);

        // 红色警告直接停车
        onStopButtonClicked();

        // 记录日志并显示警报
        onStopButtonClicked();
    }
    else {
        // 分别处理左/右传感器
        ui->N1SFail->setChecked(false);
        ui->N1SFail->setStyleSheet(NORMAL_BTN_STYLE);

        // 处理左 N1 传感器（N1LS1/N1LS2）
        if (anomalyState.N1LS1Fail && anomalyState.N1LS2Fail) {
            ui->N1LS1Fail->setChecked(true);
            ui->N1LS1Fail->setStyleSheet(CHECKED_BTN_STYLE_AMBER);
            ui->N1LS2Fail->setChecked(true);
            ui->N1LS2Fail->setStyleSheet(CHECKED_BTN_STYLE_AMBER);
            n1LeftGauge->updateLevel(AlertLevel::amberWarning);
        }
        else if (anomalyState.N1LS1Fail) {
            ui->N1LS1Fail->setChecked(true);
            ui->N1LS1Fail->setStyleSheet(CHECKED_BTN_STYLE_WHITE);
            ui->N1LS2Fail->setChecked(false);
            ui->N1LS2Fail->setStyleSheet(NORMAL_BTN_STYLE);
            n1LeftGauge->updateLevel(AlertLevel::whiteWarning);
        }
        else if (anomalyState.N1LS2Fail) {
            ui->N1LS2Fail->setChecked(true);
            ui->N1LS2Fail->setStyleSheet(CHECKED_BTN_STYLE_WHITE);
            ui->N1LS1Fail->setChecked(false);
            ui->N1LS1Fail->setStyleSheet(NORMAL_BTN_STYLE);
            n1LeftGauge->updateLevel(AlertLevel::whiteWarning);
        }
        else {
            ui->N1LS1Fail->setChecked(false);
            ui->N1LS1Fail->setStyleSheet(NORMAL_BTN_STYLE);
            ui->N1LS2Fail->setChecked(false);
            ui->N1LS2Fail->setStyleSheet(NORMAL_BTN_STYLE);
            n1LeftGauge->updateLevel(AlertLevel::normal);
        }

        // 处理右 N1 传感器（N1RS1/N1RS2）
        if (anomalyState.N1RS1Fail && anomalyState.N1RS2Fail) {
            ui->N1RS1Fail->setChecked(true);
            ui->N1RS1Fail->setStyleSheet(CHECKED_BTN_STYLE_AMBER);
            ui->N1RS2Fail->setChecked(true);
            ui->N1RS2Fail->setStyleSheet(CHECKED_BTN_STYLE_AMBER);
            n1RightGauge->updateLevel(AlertLevel::amberWarning);
        }
        else if (anomalyState.N1RS1Fail) {
            ui->N1RS1Fail->setChecked(true);
            ui->N1RS1Fail->setStyleSheet(CHECKED_BTN_STYLE_WHITE);
            ui->N1RS2Fail->setChecked(false);
            ui->N1RS2Fail->setStyleSheet(NORMAL_BTN_STYLE);
            n1RightGauge->updateLevel(AlertLevel::whiteWarning);
        }
        else if (anomalyState.N1RS2Fail) {
            ui->N1RS2Fail->setChecked(true);
            ui->N1RS2Fail->setStyleSheet(CHECKED_BTN_STYLE_WHITE);
            ui->N1RS1Fail->setChecked(false);
            ui->N1RS1Fail->setStyleSheet(NORMAL_BTN_STYLE);
            n1RightGauge->updateLevel(AlertLevel::whiteWarning);
        }
        else {
            ui->N1RS1Fail->setChecked(false);
            ui->N1RS1Fail->setStyleSheet(NORMAL_BTN_STYLE);
            ui->N1RS2Fail->setChecked(false);
            ui->N1RS2Fail->setStyleSheet(NORMAL_BTN_STYLE);
            n1RightGauge->updateLevel(AlertLevel::normal);
        }
    }

    // 处理 EGT 系列按钮
    if (anomalyState.EGTLS1Fail && anomalyState.EGTLS2Fail &&
        anomalyState.EGTRS1Fail && anomalyState.EGTRS2Fail) {
        // EGT 总故障按钮 + 4个传感器按钮 全红
        ui->EGTSFail->setChecked(true);
        ui->EGTSFail->setStyleSheet(CHECKED_BTN_STYLE_RED);
        ui->EGTLS1Fail->setChecked(true);
        ui->EGTLS1Fail->setStyleSheet(CHECKED_BTN_STYLE_RED);
        ui->EGTLS2Fail->setChecked(true);
        ui->EGTLS2Fail->setStyleSheet(CHECKED_BTN_STYLE_RED);
        ui->EGTRS1Fail->setChecked(true);
        ui->EGTRS1Fail->setStyleSheet(CHECKED_BTN_STYLE_RED);
        ui->EGTRS2Fail->setChecked(true);
        ui->EGTRS2Fail->setStyleSheet(CHECKED_BTN_STYLE_RED);
        // EGT 左右仪表设为红色报警
        egtLeftGauge->updateLevel(AlertLevel::redWarning);
        egtRightGauge->updateLevel(AlertLevel::redWarning);

        // 红色警告直接停车
        onStopButtonClicked();

        // 记录日志并显示警报
        onStopButtonClicked();
    }
    else {
        // 分别处理左/右传感器
        ui->EGTSFail->setChecked(false);
        ui->EGTSFail->setStyleSheet(NORMAL_BTN_STYLE);
        // 处理左 EGT 传感器（EGTLS1/EGTLS2）
        if (anomalyState.EGTLS1Fail && anomalyState.EGTLS2Fail) {
            ui->EGTLS1Fail->setChecked(true);
            ui->EGTLS1Fail->setStyleSheet(CHECKED_BTN_STYLE_AMBER);
            ui->EGTLS2Fail->setChecked(true);
            ui->EGTLS2Fail->setStyleSheet(CHECKED_BTN_STYLE_AMBER);
            egtLeftGauge->updateLevel(AlertLevel::amberWarning);
        }
        else if (anomalyState.EGTLS1Fail) {
            ui->EGTLS1Fail->setChecked(true);
            ui->EGTLS1Fail->setStyleSheet(CHECKED_BTN_STYLE_WHITE);
            ui->EGTLS2Fail->setChecked(false);
            ui->EGTLS2Fail->setStyleSheet(NORMAL_BTN_STYLE);
            egtLeftGauge->updateLevel(AlertLevel::whiteWarning);
        }
        else if (anomalyState.EGTLS2Fail) {
            ui->EGTLS2Fail->setChecked(true);
            ui->EGTLS2Fail->setStyleSheet(CHECKED_BTN_STYLE_WHITE);
            ui->EGTLS1Fail->setChecked(false);
            ui->EGTLS1Fail->setStyleSheet(NORMAL_BTN_STYLE);
            egtLeftGauge->updateLevel(AlertLevel::whiteWarning);
        }
        else {
            ui->EGTLS1Fail->setChecked(false);
            ui->EGTLS1Fail->setStyleSheet(NORMAL_BTN_STYLE);
            ui->EGTLS2Fail->setChecked(false);
            ui->EGTLS2Fail->setStyleSheet(NORMAL_BTN_STYLE);
            egtLeftGauge->updateLevel(AlertLevel::normal);
        }
        // 处理右 EGT 传感器（EGTRS1/EGTRS2）
        if (anomalyState.EGTRS1Fail && anomalyState.EGTRS2Fail) {
            ui->EGTRS1Fail->setChecked(true);
            ui->EGTRS1Fail->setStyleSheet(CHECKED_BTN_STYLE_AMBER);
            ui->EGTRS2Fail->setChecked(true);
            ui->EGTRS2Fail->setStyleSheet(CHECKED_BTN_STYLE_AMBER);
            egtRightGauge->updateLevel(AlertLevel::amberWarning);
        }
        else if (anomalyState.EGTRS1Fail) {
            ui->EGTRS1Fail->setChecked(true);
            ui->EGTRS1Fail->setStyleSheet(CHECKED_BTN_STYLE_WHITE);
            ui->EGTRS2Fail->setChecked(false);
            ui->EGTRS2Fail->setStyleSheet(NORMAL_BTN_STYLE);
            egtRightGauge->updateLevel(AlertLevel::whiteWarning);
        }
        else if (anomalyState.EGTRS2Fail) {
            ui->EGTRS2Fail->setChecked(true);
            ui->EGTRS2Fail->setStyleSheet(CHECKED_BTN_STYLE_WHITE);
            ui->EGTRS1Fail->setChecked(false);
            ui->EGTRS1Fail->setStyleSheet(NORMAL_BTN_STYLE);
            egtRightGauge->updateLevel(AlertLevel::whiteWarning);
        }
        else {
            ui->EGTRS1Fail->setChecked(false);
            ui->EGTRS1Fail->setStyleSheet(NORMAL_BTN_STYLE);
            ui->EGTRS2Fail->setChecked(false);
            ui->EGTRS2Fail->setStyleSheet(NORMAL_BTN_STYLE);
            egtRightGauge->updateLevel(AlertLevel::normal);
        }
    }

    // 处理燃油余量传感器按钮
    if (anomalyState.FuelSFail) {
        ui->FuelSFail->setChecked(true);
        ui->FuelSFail->setStyleSheet(CHECKED_BTN_STYLE_RED);
        fuelLevelGauge->updateLevel(AlertLevel::redWarning);
    }
    else {
        ui->FuelSFail->setChecked(false);
        ui->FuelSFail->setStyleSheet(NORMAL_BTN_STYLE);
        fuelLevelGauge->updateLevel(AlertLevel::normal);
    }

    // 处理低燃油按钮
    if (anomalyState.LowFuel) {
        ui->LowFuel->setChecked(true);
        ui->LowFuel->setStyleSheet(CHECKED_BTN_STYLE_AMBER);
    }
    else {
        ui->LowFuel->setChecked(false);
        ui->LowFuel->setStyleSheet(NORMAL_BTN_STYLE);
    }
}

// 重置超速状态
void EngineSimulator::resetSpeed() {
    ui->N1OverSpeed1->setChecked(false);
    ui->N1OverSpeed1->setStyleSheet(NORMAL_BTN_STYLE);
    ui->N1OverSpeed2->setChecked(false);
    ui->N1OverSpeed2->setStyleSheet(NORMAL_BTN_STYLE);
    ui->EGTOverSpeed1->setChecked(false);
    ui->EGTOverSpeed1->setStyleSheet(NORMAL_BTN_STYLE);
    ui->EGTOverSpeed2->setChecked(false);
    ui->EGTOverSpeed2->setStyleSheet(NORMAL_BTN_STYLE);
    ui->EGTOverSpeed3->setChecked(false);
    ui->EGTOverSpeed3->setStyleSheet(NORMAL_BTN_STYLE);
    ui->EGTOverSpeed4->setChecked(false);
    ui->EGTOverSpeed4->setStyleSheet(NORMAL_BTN_STYLE);
    ui->FFOverSpeed->setChecked(false);
    ui->FFOverSpeed->setStyleSheet(NORMAL_BTN_STYLE);
}

// 更新超速状态显示
void EngineSimulator::updateSpeed() {
	// 先重置超速状态
    resetSpeed();
    // 更新 N1 超速按钮
    switch (anomalyState.N1OverSpeedLevel) {
    case 0:
        break;
    case 1:
        ui->N1OverSpeed1->setChecked(true);
        ui->N1OverSpeed1->setStyleSheet(CHECKED_BTN_STYLE_AMBER);
        break;

    case 2:
        ui->N1OverSpeed2->setChecked(true);
        ui->N1OverSpeed2->setStyleSheet(CHECKED_BTN_STYLE_RED);
        break;
    }

	// 更新 EGT 超温按钮
    switch (anomalyState.EGTOverSpeedLevel) {
    case 0:
        break;
    case 1:
        ui->EGTOverSpeed1->setChecked(true);
        ui->EGTOverSpeed1->setStyleSheet(CHECKED_BTN_STYLE_AMBER);
        break;
    case 2:
        ui->EGTOverSpeed2->setChecked(true);
        ui->EGTOverSpeed2->setStyleSheet(CHECKED_BTN_STYLE_RED);
        break;
    case 3:
        ui->EGTOverSpeed3->setChecked(true);
        ui->EGTOverSpeed3->setStyleSheet(CHECKED_BTN_STYLE_AMBER);
        break;
    case 4:
        ui->EGTOverSpeed4->setChecked(true);
        ui->EGTOverSpeed4->setStyleSheet(CHECKED_BTN_STYLE_RED);
        break;
    }

	// 更新低油显示按钮
    if (anomalyState.LowFuel) {
        ui->LowFuel->setChecked(true);
        ui->LowFuel->setStyleSheet(CHECKED_BTN_STYLE_AMBER);
    }

	// 更新燃油流速超速按钮
    if (anomalyState.FFOverSpeed) {
        ui->FFOverSpeed->setChecked(true);
        ui->FFOverSpeed->setStyleSheet(CHECKED_BTN_STYLE_AMBER);
    }
}

// 初始化异常状态
// 注意：这里不会重置传感器故障状态，因为停车后传感器故障并不会消失
void EngineSimulator::initAnomalyState() {
    resetSpeed();
    anomalyState.EGTOverSpeedLevel = 0;
    anomalyState.N1OverSpeedLevel = 0;
    anomalyState.FFOverSpeed = false;
}

