#ifndef GAUGEWIDGET_H
#define GAUGEWIDGET_H

#include "EngineState.h"
#include <QWidget>
#include <QPainter>
#include <QTimer>

// 仪表盘控件类
class GaugeWidget : public QWidget {
    Q_OBJECT

public:
	// 仪表盘类型枚举
    enum GaugeType {
        N1,
        EGT,
        FUEL_LEVEL
    };

	// 构造函数
    explicit GaugeWidget(GaugeType initType, QWidget* parent = nullptr);

	// 根据不同的仪表盘类型设置参数
	void setupByType();

	// 更新当前数值和告警等级
    void updateValue(double newValue);
    void updateLevel(AlertLevel newLevel);

protected:
	// 重绘事件
    void paintEvent(QPaintEvent* event) override;

private:
    double minValue;
    double maxValue;
    double currentValue;
    double dangerStart;
    double dangerEnd;
    GaugeType type;
    QString gaugeName;
    QString unit;
	AlertLevel level;

    int startAngle; // 起始角度
    double angle;   // 每个刻度的角度值
    double dangerStartAngle;
    double dangerEndAngle;
    int radius;     // 仪表盘半径

	// 绘制各个部分的辅助函数
    void initCanvas(QPainter& painter);
    void drawMiddleCircle(QPainter& painter, int radius);
    void drawScale(QPainter& painter, int radius);
    void drawPoint(QPainter& painter, int radius);
    void drawInnerEllipse(QPainter& painter, int radius);
    void drawInnerEllipseBlack(QPainter& painter, int radius);
    void drawCurrentSpeed(QPainter& painter);
    void drawEllipseOutSkirts(QPainter& painter, int radius);
    void drawLogo(QPainter& painter, int radius);
    double valueToAngle(double value) const;
    QColor getBackgroundColor() const;
};

#endif