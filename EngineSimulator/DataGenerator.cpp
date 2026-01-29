#include "DataGenerator.h"
#include <QRandomGenerator>
#include <cmath>
#include <qdebug.h>

// 构造函数初始化
DataGenerator::DataGenerator() {
    data = SensorData();
    auxData = SensorData();
    increaseThrust = false;
    decreaseThrust = false;
}

// 控制接口实现
void DataGenerator::startSequence() {
    if (data.phase == EnginePhase::Idle) {
        data.phase = EnginePhase::Starting;
        data.elapsedTime = 0.0; // 重置时间
    }
}
void DataGenerator::stopSequence() {
    if (data.phase != EnginePhase::Idle && data.phase != EnginePhase::Stopping) {
        data.lastPhase = data.phase;
        data.phase = EnginePhase::Stopping;
        data.fuelFlow = 0.0;
        auxData = data; // 保存停车瞬间的状态作为衰减起点
    }
}
void DataGenerator::setIncreaseThrust(bool val) {
    increaseThrust = val;
}
void DataGenerator::setDecreaseThrust(bool val) {
    decreaseThrust = val;
}

// 内部更新数据函数
void DataGenerator::updateData() {
    // 燃油余量更新
    auxData.fuelLevel = data.fuelLevel -= data.fuelFlow * TIME_STEP;
    // 更新时间
    data.elapsedTime += TIME_STEP;

	// 根据当前阶段更新数据
    switch (data.phase) {
        // 保持不变
    case EnginePhase::Idle:
        break;
        // 启动阶段
    case EnginePhase::Starting:
        if (data.elapsedTime < 2.0) {
            // 线性增长
            data.n1LeftAverage = data.n1RightAverage = 10000.0 * data.elapsedTime * 100.0 / RATED_RPM;
            data.egtLeftAverage = data.egtRightAverage = TEMPRATURE;
            data.fuelFlow = 5.0 * data.elapsedTime;
        }
        else {
            data.n1LeftAverage = data.n1RightAverage = 23000.0 * std::log10(data.elapsedTime - 1.0) * 100.0 / RATED_RPM + 50.0;
            data.egtLeftAverage = data.egtRightAverage = 900.0 * std::log10(data.elapsedTime - 1.0) + TEMPRATURE;
            data.fuelFlow = 42 * std::log10(data.elapsedTime - 1.0) + 10.0;
        }
        // 处理超温1状态
        if (data.EGTOverSpeed1) {
            data.egtLeftAverage += 840.0;
            data.egtRightAverage += 840.0;
        }
        break;

        // 达到稳态
    case EnginePhase::Stable:
        data = auxData;
        break;

        // 停车阶段
    case EnginePhase::Stopping:
        // 自定义对数衰减函数，底数为0.05
        double stopTime = data.elapsedTime - auxData.elapsedTime;

        data.n1LeftAverage = auxData.n1LeftAverage * std::log10(0.05 + 15.0 * stopTime / auxData.n1LeftAverage) / std::log10(0.05);
        data.n1RightAverage = auxData.n1RightAverage * std::log10(0.05 + 15.0 * stopTime / auxData.n1RightAverage) / std::log10(0.05);
        data.egtLeftAverage = (auxData.egtLeftAverage - TEMPRATURE) * std::log10(0.05 + 150.0 * stopTime / auxData.egtLeftAverage) / std::log10(0.05) + TEMPRATURE;
        data.egtRightAverage = (auxData.egtRightAverage - TEMPRATURE) * std::log10(0.05 + 150.0 * stopTime / auxData.egtRightAverage) / std::log10(0.05) + TEMPRATURE;
        break;
    }

    // 处理推力偏移量
    if (increaseThrust) {
        auxData.fuelFlow += 1.0; // 每次增加1 lbs/s
        auxData.n1RightAverage = auxData.n1LeftAverage *= (1 + QRandomGenerator::global()->bounded(0.02) + 0.03);
        auxData.egtLeftAverage = auxData.egtRightAverage *= (1 + QRandomGenerator::global()->bounded(0.02) + 0.03);
        increaseThrust = false; // 重置标志
    }
    if (decreaseThrust) {
        auxData.fuelFlow -= 1.0; // 每次减少1 lbs/s
        auxData.n1RightAverage = auxData.n1LeftAverage *= (1 + QRandomGenerator::global()->bounded(0.02) - 0.05);
        auxData.egtLeftAverage = auxData.egtRightAverage *= (1 + QRandomGenerator::global()->bounded(0.02) - 0.05);
        decreaseThrust = false; // 重置标志
    }

    // 随机扰动，所有噪声均分布在±2%内
    data.n1LeftAverage *= (1 + QRandomGenerator::global()->bounded(0.02) - 0.01);
    data.n1RightAverage *= (1 + QRandomGenerator::global()->bounded(0.02) - 0.01);
    data.egtLeftAverage *= (1 + QRandomGenerator::global()->bounded(0.02) - 0.01);
    data.egtRightAverage *= (1 + QRandomGenerator::global()->bounded(0.02) - 0.01);
    data.fuelFlow *= (1 + QRandomGenerator::global()->bounded(0.02) - 0.01);

    // 检查并更新阶段
    checkPhase();
}

// 更新数据外部接口
void DataGenerator::updateData(const SensorData& newData) {
    data = auxData = newData;
}

// 获取当前数据
SensorData DataGenerator::getData() {
    return data;
}

// 检查并更新阶段
void DataGenerator::checkPhase() {
    // 根据当前数据和条件检查并更新阶段
    if (data.phase == EnginePhase::Starting && (data.n1LeftAverage >= 95.0 && data.n1RightAverage >= 95.0)) {
        data.phase = EnginePhase::Stable;
        // 保存稳态数据到辅助变量
        auxData = data;
    }
    else if (data.phase == EnginePhase::Stopping) {
        double stopTime = data.elapsedTime - auxData.elapsedTime;

        // 条件1：停车时间超过10秒
        // 条件2：转速已降到很低（< 1%）
        if (stopTime >= STOP_DURATION || (data.n1LeftAverage <= 0.1 && data.n1RightAverage <= 0.1
            && data.egtLeftAverage <= (TEMPRATURE + 0.5)) && data.egtRightAverage <= (TEMPRATURE + 0.5)) {
            data.phase = EnginePhase::Idle;
            // 重置所有值到完全停止状态
            data.n1LeftAverage = data.n1RightAverage = 0.0;
            data.egtLeftAverage = data.egtRightAverage = TEMPRATURE; // 回到室温
            data.fuelFlow = 0.0;

            data.EGTOverSpeed1 = false;
        }
    }
}

// 处理数据
SensorData DataGenerator::processData(const SensorData& inputData) {
    SensorData outputData = inputData;
    if(outputData.fuelLevel < 0.0) {
        outputData.fuelLevel = 0.0;
	}
    if (outputData.n1LeftAverage < 0.0) {
        outputData.n1LeftAverage = 0.0;
    }
    if(outputData.n1RightAverage < 0.0) {
        outputData.n1RightAverage = 0.0;
	}
    if (outputData.egtLeftAverage < TEMPRATURE) {
        outputData.egtLeftAverage = TEMPRATURE;
    }
    if (outputData.egtRightAverage < TEMPRATURE) {
        outputData.egtRightAverage = TEMPRATURE;
	}
    return outputData;
}