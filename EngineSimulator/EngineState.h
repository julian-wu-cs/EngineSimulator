#pragma once
#ifndef ENGINESTATE_H
#define ENGINESTATE_H
#include <QString>

// 发动机状态
enum class EnginePhase {
    Idle,        // 未启动
    Starting,    // 启动阶段（0~2s线性，之后对数增长）
    Stable,      // 稳态
    Stopping     // 停车阶段
};

// 告警级别
enum class AlertLevel {
	normal,
    whiteWarning,
	amberWarning,
    redWarning
};

// 传感器数据结构
struct SensorData {
    double n1LeftAverage;           // 左发 N1%
    double n1RightAverage;          // 右发 N1%
    double egtLeftAverage;          // 左发 EGT
    double egtRightAverage;         // 右发 EGT
    double fuelLevel;        // 燃油余量
    double fuelFlow;         // 燃油流速
    EnginePhase phase;       // 当前阶段
    EnginePhase lastPhase;   // 上一阶段
    double elapsedTime;      // 经过时间
    bool EGTOverSpeed1;    // 是否处于超温1状态

    // 构造函数初始化默认值
    SensorData() : n1LeftAverage(0.0), n1RightAverage(0.0), egtLeftAverage(20.0),
        egtRightAverage(20.0), fuelLevel(20000.0), fuelFlow(0.0),
        phase(EnginePhase::Idle), elapsedTime(0.0), lastPhase(EnginePhase::Idle),
        EGTOverSpeed1(false) {
    }
};

struct AnomalyState {
    // 当前传感器数据
	bool N1LS1Fail;        // 左发 N1 传感器1 故障
	bool N1RS1Fail;        // 右发 N1 传感器1 故障
	bool N1LS2Fail;        // 左发 N1 传感器2 故障
	bool N1RS2Fail;        // 右发 N1 传感器2 故障
	bool EGTLS1Fail;       // 左发 EGT 传感器1 故障
	bool EGTRS1Fail;       // 右发 EGT 传感器1 故障
	bool EGTLS2Fail;       // 左发 EGT 传感器2 故障
	bool EGTRS2Fail;       // 右发 EGT 传感器2 故障
	bool FuelSFail;        // 燃油传感器故障
    int N1OverSpeedLevel;  // 0-无超转，1-超转1，2-超转2
    int EGTOverSpeedLevel; // 0-无超速，1-超温1，2-超温2，3-超温3，4-超温4
    bool LowFuel;
    bool FFOverSpeed;

    AnomalyState()
        : N1LS1Fail(false), N1RS1Fail(false), N1LS2Fail(false), N1RS2Fail(false),
          EGTLS1Fail(false), EGTRS1Fail(false), EGTLS2Fail(false), EGTRS2Fail(false),
          FuelSFail(false), N1OverSpeedLevel(0), EGTOverSpeedLevel(0),
		LowFuel(false), FFOverSpeed(false) {
	}
};

// 常量
constexpr double RATED_RPM = 40000.0;      // 额定转速
constexpr double MAX_FUEL = 20000.0;       // 满油
constexpr double TIME_STEP = 0.005;        // 5ms 时间步长
constexpr double TIME_UI_STEP = 0.5;       // 500ms UI 更新步长
constexpr double TEMPRATURE = 20.0;        // 环境温度
constexpr double STOP_DURATION = 10.0; // 10秒停车时间


// 按钮状态样式定义
// 普通状态（非报警）- 灰色基底
const QString NORMAL_BTN_STYLE = R"(
    QPushButton {
        background-color: #333333;
        color: white;
        border: 1px solid #666666;
        border-radius: 5px;
    }
    QPushButton:hover {
        background-color: #444444;
    }
    QPushButton:pressed {
        background-color: #555555;
        border: 2px solid #FF9800;
    }
)";

// 报警状态1 - 白色（高对比度白，适配深色基底）
const QString CHECKED_BTN_STYLE_WHITE = R"(
    QPushButton {
        background-color: #FFFFFF;
        color: #222222; /* 黑色文字保证可读性 */
        border: 2px solid #EEEEEE;
        border-radius: 5px;
        font-weight: bold;
    }
    QPushButton:hover {
        background-color: #F5F5F5;
    }
)";

// 报警状态2 - 琥珀色/黄色（预警级别）
const QString CHECKED_BTN_STYLE_AMBER = R"(
    QPushButton {
        background-color: #FFC107;
        color: #222222; /* 黑色文字保证可读性 */
        border: 2px solid #FFA000;
        border-radius: 5px;
        font-weight: bold;
    }
    QPushButton:hover {
        background-color: #FFD54F;
    }
)";

// 报警状态3 - 红色（紧急级别，保留你原有样式）
const QString CHECKED_BTN_STYLE_RED = R"(
    QPushButton {
        background-color: #F44336;
        color: white;
        border: 2px solid #FF5252;
        border-radius: 5px;
        font-weight: bold;
    }
    QPushButton:hover {
        background-color: #EF5350;
    }
)";


#endif // ENGINESTATE_H