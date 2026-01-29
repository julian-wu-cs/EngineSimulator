#ifndef LOGGER_H
#define LOGGER_H

#include "EngineState.h" 
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QHash>
#include <QTextEdit> 

class Logger {
public:
    explicit Logger();
    ~Logger();

    // 初始化日志文件（创建 DataLogging/ 目录和文件）
    void initLogFile(); 

    // 统一处理数据记录 + 告警
    void logDataAndAlerts(double t,const SensorData& data,
        const AnomalyState& anomaly,
        QTextEdit* alertDisplay = nullptr);

private:
	// 数据日志文件和告警日志文件
    QFile dataFile;
    QFile alertFile; 

	// 数据流对象
    QTextStream dataStream;
    QTextStream alertStream;

	// 当前日志文件基础名称
    QString currentBaseName;

	// 是否正在记录日志
    bool isLogging = false;

    // 5秒告警去重
    QHash<QString, double> lastAlertTime; 

    // 辅助函数：生成不同颜色的告警并记录
    void triggerAlert(double elapsedTime, AlertLevel level, const QString& alertMessage, QTextEdit* alertDisplay);
};

#endif 