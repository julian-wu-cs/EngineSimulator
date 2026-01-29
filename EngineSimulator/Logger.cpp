#include "Logger.h"
#include <QDir>
#include <QDateTime>
#include <QCoreApplication>
#include <QDebug>
#include <QMetaEnum>

// 辅助：判断当前是否处于“启动阶段”（含刚转入停车）
static bool isInStartingPhase(const SensorData& data) {
    return (data.phase == EnginePhase::Starting) ||
        (data.lastPhase == EnginePhase::Starting && data.phase == EnginePhase::Stopping);
}

// 辅助：判断当前是否处于“稳态”（含刚转入停车）
static bool isInStablePhase(const SensorData& data) {
    return (data.phase == EnginePhase::Stable) ||
        (data.lastPhase == EnginePhase::Stable && data.phase == EnginePhase::Stopping);
}

// 构造与析构
Logger::Logger() = default;
Logger::~Logger() {
    if (dataFile.isOpen()) dataFile.close();
    if (alertFile.isOpen()) alertFile.close();
}

// 初始化日志文件
void Logger::initLogFile() {
	// 重置告警时间记录
    lastAlertTime.clear();

	// 创建 DataLogging 目录
    QString exePath = QCoreApplication::applicationDirPath();
    QString loggingDir = exePath + "/DataLogging";

    QDir dir;
    if (!dir.exists(loggingDir)) {
        if (!dir.mkpath(loggingDir)) {
            qWarning() << "Failed to create DataLogging directory!";
            return;
        }
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    currentBaseName = loggingDir + "/" + timestamp;

    // 初始化数据日志 (.csv)
    dataFile.setFileName(currentBaseName + ".csv");
    if (!dataFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open data log file:" << currentBaseName + ".csv";
        return;
    }
    dataStream.setDevice(&dataFile);
    dataStream << "Timestamp(s),N1LeftAverage,N1RightAverage,EGTLeftAverage,EGTRightAverage,FuelLevel,FuelFlow,Phase\n";
    dataStream.flush();

    // 初始化告警日志 (.log)
    alertFile.setFileName(currentBaseName + ".log");
    if (!alertFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open alert log file:" << currentBaseName + ".log";
    }
    else {
        alertStream.setDevice(&alertFile);
        alertStream << "=== Engine Simulator Alert Log ===\n";
        alertStream << "Start Time: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n\n";
        alertStream.flush();
    }

    isLogging = true;
    qDebug() << "Log files created:" << currentBaseName + ".*";
}

// 统一记录数据 + 检测告警函数
void Logger::logDataAndAlerts(double t, const SensorData& data,
    const AnomalyState& anomaly,
    QTextEdit* alertDisplay) {
    if (!isLogging) return;

    // 记录传感器数据到 CSV
    if (dataFile.isOpen()) {
        // 定义各数据字段的有效性判断函数（内联函数，简洁易维护）
        bool isValidN1L = !(anomaly.N1LS1Fail && anomaly.N1LS2Fail); // N1是百分比，0-120
        bool isValidN1R = !(anomaly.N1RS1Fail && anomaly.N1RS2Fail); // N1是百分比，0-120
        bool isValidEGTL = !(anomaly.EGTLS1Fail && anomaly.EGTLS2Fail); // EGT合理范围20-1200℃
        bool isValidEGTR = !(anomaly.EGTRS1Fail && anomaly.EGTRS2Fail); // EGT合理范围20-1200℃
        bool isValidFuelS = !anomaly.FuelSFail; // 燃油传感器故障
        bool isValidFuelFlow = !anomaly.FFOverSpeed; // 燃油流速超速

        // 逐个字段处理：有效则格式化数值，无效则显示N/A
        QString timeStr = QString::number(t, 'f', 2);
        QString n1LeftStr = isValidN1L ? QString::number(data.n1LeftAverage, 'f', 2) : "N/A";
        QString n1RightStr = isValidN1R ? QString::number(data.n1RightAverage, 'f', 2) : "N/A";
        QString egtLeftStr = isValidEGTL ? QString::number(data.egtLeftAverage, 'f', 1) : "N/A";
        QString egtRightStr = isValidEGTR ? QString::number(data.egtRightAverage, 'f', 1) : "N/A";
        QString fuelLevelStr = QString::number(data.fuelLevel, 'f', 1);
        QString fuelFlowStr = QString::number(data.fuelFlow, 'f', 1);
        QString phaseStr = QString::number(static_cast<int>(data.phase));

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

		dataStream << log << "\n";
    }

    // 告警检测与记录

    // N1 超转
    if ((data.n1LeftAverage > 120.0 || data.n1RightAverage > 120.0) && anomaly.N1OverSpeedLevel == 2) {
        triggerAlert(t, AlertLevel::redWarning, "[Red Warning] N1 overspeed level 2: Exceeds 120% N1", alertDisplay);
    }
    else if ((data.n1LeftAverage > 105.0 || data.n1RightAverage > 105.0) && anomaly.N1OverSpeedLevel == 1) {
        triggerAlert(t, AlertLevel::amberWarning, "[Amber Warning] N1 overspeed level 1: Exceeds 105% N1", alertDisplay);
    }

    // EGT 超温
    if (isInStartingPhase(data)) {
        if (anomaly.EGTOverSpeedLevel == 2) {
            triggerAlert(t, AlertLevel::redWarning, "[Red Warning] EGT overtemp level 2: Exceeds 100\u2103 during engine start", alertDisplay);
        }
        else if (anomaly.EGTOverSpeedLevel == 1) {
            triggerAlert(t, AlertLevel::amberWarning, "[Amber Warning] EGT overtemp level 1: Exceeds 850 \u2103 during engine start", alertDisplay);
        }
    }
    else if (isInStablePhase(data)) {
        if (anomaly.EGTOverSpeedLevel == 4) {
            triggerAlert(t, AlertLevel::redWarning, "[Red Warning] EGT overtemp level 4: Exceeds 1100 \u2103 in stable operation", alertDisplay);
        }
        else if (anomaly.EGTOverSpeedLevel == 3) {
            triggerAlert(t, AlertLevel::amberWarning, "[Amber Warning] EGT overtemp level 3: Exceeds 950 \u2103 in stable operation", alertDisplay);
        }
    }

    // 燃油异常
    if (anomaly.LowFuel) {
        triggerAlert(t, AlertLevel::amberWarning, "[Amber Warning] Fuel level: Below 1000 lbs", alertDisplay);
    }
    if (anomaly.FFOverSpeed) {
        triggerAlert(t, AlertLevel::amberWarning, "[Amber Warning] Fuel flow: Exceeds 50 lbs per second", alertDisplay);
    }
    if (anomaly.FuelSFail) {
        triggerAlert(t, AlertLevel::redWarning, "[Red Warning] Fuel system: Sensor failure", alertDisplay);
    }

    // N1 传感器故障
    if (anomaly.N1LS1Fail && anomaly.N1LS2Fail &&
        anomaly.N1RS1Fail && anomaly.N1RS2Fail) {
        triggerAlert(t, AlertLevel::redWarning, "[Red Warning] N1 system: Critical failure all sensors failed", alertDisplay);
    }
    else {
        if (anomaly.N1LS1Fail && anomaly.N1LS2Fail) {
            triggerAlert(t, AlertLevel::amberWarning, "[Amber Warning] N1 system: Left engine sensors failed", alertDisplay);
        }
        else if (anomaly.N1LS1Fail || anomaly.N1LS2Fail) {
            triggerAlert(t, AlertLevel::whiteWarning, "[White Warning] N1 system: Left engine single sensor failure", alertDisplay);
        }

        if (anomaly.N1RS1Fail && anomaly.N1RS2Fail) {
            triggerAlert(t, AlertLevel::amberWarning, "[Amber Warning] N1 system: Right engine sensors failed", alertDisplay);
        }
        else if (anomaly.N1RS1Fail || anomaly.N1RS2Fail) {
            triggerAlert(t, AlertLevel::whiteWarning, "[White Warning] N1 system: Right engine single sensor failure", alertDisplay);
        }
    }

    // EGT 传感器故障
    if (anomaly.EGTLS1Fail && anomaly.EGTLS2Fail &&
        anomaly.EGTRS1Fail && anomaly.EGTRS2Fail) {
        triggerAlert(t, AlertLevel::redWarning, "[Red Warning] EGT system: Critical failure all sensors failed", alertDisplay);
    }
    else {
        if (anomaly.EGTLS1Fail && anomaly.EGTLS2Fail) {
            triggerAlert(t, AlertLevel::amberWarning, "[Amber Warning] EGT system: Left engine sensors failed", alertDisplay);
        }
        else if (anomaly.EGTLS1Fail || anomaly.EGTLS2Fail) {
            triggerAlert(t, AlertLevel::whiteWarning, "[White Warning] EGT system: Left engine single sensor failure", alertDisplay);
        }

        if (anomaly.EGTRS1Fail && anomaly.EGTRS2Fail) {
            triggerAlert(t, AlertLevel::amberWarning, "[Amber Warning] EGT system: Right engine sensors failed", alertDisplay);
        }
        else if (anomaly.EGTRS1Fail || anomaly.EGTRS2Fail) {
            triggerAlert(t, AlertLevel::whiteWarning, "[White Warning] EGT system: Right engine single sensor failure", alertDisplay);
        }
    }
}

// 触发单条告警（含去重 + 文件+UI）
void Logger::triggerAlert(double elapsedTime, AlertLevel level, const QString & alertMessage, QTextEdit * alertDisplay) {
    QString msg = alertMessage.trimmed();
    if (msg.isEmpty()) return;

    // 5秒内同消息不重复
    if (lastAlertTime.contains(msg)) {
        if (elapsedTime - lastAlertTime[msg] < 5.0) {
            return;
        }
    }

    lastAlertTime[msg] = elapsedTime;

    // 生成统一文本
    QString fullText = QString("[%1s] %2").arg(elapsedTime, 0, 'f', 3).arg(msg);

    // 写入文件
    if (alertFile.isOpen()) {
        alertStream << fullText << "\n";
        alertStream.flush();
    }

    if(alertDisplay){
        QColor color;
        switch(level){
            // 白色警告
            case AlertLevel::whiteWarning:
                color = QColor("white");
                break;
            // 琥珀色警告
            case AlertLevel::amberWarning:
                color = QColor("#FFBF00");
                break;
			// 红色警告
            case AlertLevel::redWarning:
                color = QColor("red");
                break;
        }
        alertDisplay->setTextColor(color);
	}

    // 显示到 UI
    if (alertDisplay) {
        alertDisplay->append(fullText);
    }

    qDebug() << "[ALERT]" << fullText;
}