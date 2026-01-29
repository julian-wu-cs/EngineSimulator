#ifndef DATAGENERATOR_H
#define DATAGENERATOR_H

#include "EngineState.h"
#include <QVector>

// 数据生成器类
class DataGenerator {
public:
    DataGenerator();

	// 控制接口
    void startSequence();
    void stopSequence();
    void setIncreaseThrust(bool val);
    void setDecreaseThrust(bool val);

    // 更新数据
    void updateData();
    void updateData(const SensorData& newData);

	// 检查并更新阶段
    void checkPhase();

	// 获取当前数据
    SensorData getData();

    // 处理数据
	SensorData processData(const SensorData& inputData);

private:
    // 数据和辅助数据
    SensorData data , auxData;

    // 是否增加推力
    bool increaseThrust;  
    // 是否减小推力
    bool decreaseThrust;  
};

#endif // DATAGENERATOR_H