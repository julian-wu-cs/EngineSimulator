#include "GaugeWidget.h"
#include "EngineState.h"
#include <QFontMetrics>
#include <cmath>

// 根据不同的仪表盘类型设置参数
void GaugeWidget::setupByType() {
    switch (type) {
    case N1:
        minValue = currentValue = 0.0;
        maxValue = dangerEnd = 125.0;
        gaugeName = "N1";
        unit = "%";
		dangerStart = 105.0;
        break;
    case EGT:
        minValue = -5.0;
        maxValue = dangerEnd = 1200.0;
        gaugeName = "EGT";
        unit = "C";
		dangerStart = 850.0;
		currentValue = 20.0;
        break;
    case FUEL_LEVEL:
        minValue = dangerStart = 0.0;
        maxValue = currentValue = 20000.0;
        gaugeName = "Fuel Level";
        unit = "lbs";
		dangerEnd = 1000.0;
        break;
    }
    level = AlertLevel::normal;
}

// 构造函数
GaugeWidget::GaugeWidget(GaugeType initType, QWidget* parent)
	: QWidget(parent), level(AlertLevel::normal), type(initType)
{
    setMinimumSize(300, 300);
    startAngle = 255.0; // 起始角度
    setupByType();
    angle = 7.0;
    dangerStartAngle = valueToAngle(dangerStart);
    dangerEndAngle = valueToAngle(dangerEnd);
}

// 更新数据
void GaugeWidget::updateValue(double newValue) {
    currentValue = newValue;
    update(); // 触发重绘
}

// 更新告警等级
void GaugeWidget::updateLevel(AlertLevel newLevel) {
    level = newLevel;
    update(); // 触发重绘
}

// 将数值转换为角度
double GaugeWidget::valueToAngle(double value) const {
    if (value <= minValue) return 0.0;
    if (value >= maxValue) return 210.0;
    return 210.0 * (value - minValue) / (maxValue - minValue);
}

// 获取背景颜色
QColor GaugeWidget::getBackgroundColor() const {
    switch (level) {
	case AlertLevel::normal: return QColor(0, 255, 0); // 绿色
	case AlertLevel::whiteWarning: return QColor(255, 255, 255); // 白色
	case AlertLevel::amberWarning: return QColor(255, 191, 0); // 琥珀色
	case AlertLevel::redWarning: return QColor(255, 0, 0); // 红色
    }
    return QColor(0, 0, 0);
}

// 初始化画布
void GaugeWidget::initCanvas(QPainter& painter) {
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush(QColor(0, 0, 0));
    painter.drawRect(rect());
    QPoint center(rect().width() / 2, rect().height() * 0.6);
    painter.translate(center);
}

// 绘制刻度线的圆圈
void GaugeWidget::drawMiddleCircle(QPainter& painter, int radius) {
    painter.setPen(QPen(QColor(255, 255, 255), 3));
    painter.drawEllipse(QPoint(0, 0), radius, radius);
}

// 绘制刻度线和刻度值
void GaugeWidget::drawScale(QPainter& painter, int radius) {
    painter.save();
    painter.setPen(QPen(Qt::white, 3));
    painter.rotate(startAngle);

    // 字体设置（刻度数值）：Arial、15号、加粗
    QFont textFont("Arial", 10);
    textFont.setBold(true);
    painter.setFont(textFont);


    for (int i = 0; i <= 30; ++i) {
        if (i*angle >= dangerStartAngle && i*angle <= dangerEndAngle) {
            painter.setPen(QPen(Qt::red, 3));
        }
        else {
            painter.setPen(QPen(Qt::white, 3));
        }
        if (i % 5 == 0) {
            painter.drawLine(0, -radius + 18, 0 , -radius + 3);
            painter.save();
            // 移动到数值绘制位置
            painter.translate(0, 30 - radius);

            // 计算当前刻度对应的数值（0→minValue，30→maxValue）
            double currentValue = minValue + (maxValue - minValue) * i / 30.0;

            // 绘制数值：居中显示
            painter.drawText(-20, -10, 40, 20, Qt::AlignCenter,
                QString::number(currentValue, 'f', 0));

            painter.restore(); // 恢复到移动前的状态
        }
        else {
            painter.drawLine(0,-radius + 8,0, -radius + 3);
        }
        painter.rotate(angle);
    }
    painter.restore();
    painter.setPen(QPen(Qt::white, 3));
}

// 绘制指针
void GaugeWidget::drawPoint(QPainter& painter, int radius) {
    // 故障状态不绘制指针
    if(level == AlertLevel::redWarning || (level == AlertLevel::amberWarning && type != FUEL_LEVEL)) {
        return; 
	}
    painter.save();
    painter.setBrush(Qt::white);
    painter.setPen(Qt::NoPen);
    static const QPointF points[4] = {
        QPointF(-5.0, 0.0),
        QPointF(-1.0, -radius * 2 / 3),
        QPointF(1.0, -radius * 2 / 3),
        QPointF(5.0, 0.0)
    };
    painter.rotate(startAngle + valueToAngle(currentValue));
    painter.drawPolygon(points, 4);
    painter.restore();
}

// 绘制内部渐变圆
void GaugeWidget::drawInnerEllipse(QPainter& painter, int radius) {
    QRadialGradient radial(0, 0, radius);
    radial.setColorAt(0.0,getBackgroundColor());
    radial.setColorAt(1.0, QColor(0, 0, 0));
    painter.setBrush(radial);
    painter.drawEllipse(QPoint(0, 0), radius, radius);
}

// 绘制黑色内圆
void GaugeWidget::drawInnerEllipseBlack(QPainter& painter, int radius) {
    painter.setBrush(QColor(0, 0, 0));
    painter.drawEllipse(QPoint(0, 0), radius, radius);
}

// 绘制数值和单位
void GaugeWidget::drawCurrentSpeed(QPainter& painter) {
    painter.setPen(getBackgroundColor());
    QFont font("Arial", 15);
    font.setBold(true);
    if(level == AlertLevel::redWarning || (level == AlertLevel::amberWarning && type != FUEL_LEVEL)) {
        // 不显示数值，而是显示英文的故障
        painter.drawText(QRect(-40, -30, 80, 60), Qt::AlignCenter, "Fault!");
    }
    else {
        painter.setFont(font);
        if (currentValue < minValue) {
            currentValue = minValue;
        }
        else if (currentValue > maxValue) {
            currentValue = maxValue;
        }
        else {
            currentValue = currentValue;
		}
		// 特殊处理EGT仪表，按常理来说EGT不会低于环境温度，这里强制设置一个最低值
        if (type == EGT && currentValue < TEMPRATURE) {
			currentValue = TEMPRATURE;
        }
        painter.drawText(QRect(-40, -30, 80, 60), Qt::AlignCenter, QString::number(currentValue, 'f', 1));
        QFont font_u("Arial", 10);
        painter.setFont(font_u);
        painter.drawText(QRect(-30, 0, 60, 40), Qt::AlignCenter, unit);
    }
}

void GaugeWidget::drawEllipseOutSkirts(QPainter& painter, int radius) {
    QRect outAngle(-radius, -radius, 2 * radius, 2 * radius);
    painter.setPen(Qt::NoPen);
    QRadialGradient radia(0, 0, radius);
    radia.setColorAt(1, getBackgroundColor());
    radia.setColorAt(0.97, getBackgroundColor());
    radia.setColorAt(0.9, QColor(0, 0, 0, 0));
    radia.setColorAt(0, QColor(0, 0, 0, 0));
    painter.setBrush(radia);
    painter.drawPie(outAngle, (360 - startAngle) * 16, -angle * 61 * 16);
}

// 绘制logo
void GaugeWidget::drawLogo(QPainter& painter, int radius) {
    QRect rectLogo(-65, radius * 0.38, 130, 50);
    QFont font("Arial", 14);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(rectLogo, Qt::AlignCenter, gaugeName);
}

// 重绘事件
void GaugeWidget::paintEvent(QPaintEvent* event) {
	// 根据燃油量自动调整告警级别
    if(type == FUEL_LEVEL && currentValue < 1000.0 && level != AlertLevel::redWarning) {
		level = AlertLevel::amberWarning;
	}
    QPainter painter(this);
    // 仪表盘半径
    radius = height() / 2;

    initCanvas(painter);
    drawMiddleCircle(painter, radius);
    drawScale(painter, radius);
    drawInnerEllipse(painter, 110);
    drawPoint(painter, radius);
    drawInnerEllipseBlack(painter, 40);
    drawCurrentSpeed(painter);
    drawEllipseOutSkirts(painter, radius + 25);
    drawLogo(painter, radius);
}