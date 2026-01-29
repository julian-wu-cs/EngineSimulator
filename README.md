# EngineSimulator项目说明

本项目是一个基于 Qt 的发动机模拟器程序，支持直接运行或本地编译。  
已提供**静态打包版本**（包含所有必要依赖，双击即用），也支持在本地配置开发环境后动态编译源码。

---

##  一、项目代码搭建与运行说明
### 1. 📁 项目目录结构

```
EngineSimulator/
├── EngineSimulator.sln
├── README.md
├── Report.pdf
├── Report.md
├── VideoDemo.mp4
├── EngineSimulator/
│   ├── main.cpp
│   ├── EngineSimulator.cpp
│   ├── EngineSimulator.h
│   ├── Logger.cpp
│   ├── Logger.h
│   ├── GaugeWidget.cpp
│   ├── GaugeWidget.h
│   ├── DataGenerator.cpp
│   ├── DataGenerator.h
│   └── EngineState.h
└── Release/
    ├── DataLogging/  
    ├── EngineSimulator.exe
    ├── platforms/
    │   └── qwindows.dll
    ├── Qt6Core.dll
    ├── Qt6Gui.dll
    └── Qt6Widgets.dll
```

---
### 静态打包版本（推荐普通用户）
> 无需安装 Qt、Visual Studio 或其他运行库。解压后双击`Release/EngineSimulator.exe`即可运行！

#### ✅ 文件说明
- `Release/EngineSimulator.exe`：主程序入口
- `Release/platforms/qwindows.dll`：Qt 平台插件
- `Release/Qt6Core.dll`、`Qt6Gui.dll`、`Qt6Widgets.dll` 等：Qt 运行依赖
- `Release/DataLogging/`: 用于保存日志文件（`.csv` 数据日志与 `.log` 告警日志）

**注意：** 除了 `DataLogging/` 文件夹外，请务必解压`Release/`中其他所有文件和文件夹，确保程序能正确运行。

### 📌 使用步骤
1. 解压整个压缩包到任意本地目录（例如 `D:\EngineSimulator_Package`）。
2. 进入 `Release/` 文件夹。
3. 双击 `EngineSimulator.exe` 启动程序。
4. 程序将自动记录数据，并在 `Release/DataLogging/` 中生成带时间戳的日志文件。


---

### 💻 本地动态编译源码（适用于开发者）

> 如需修改代码、调试或重新编译，请按以下步骤配置开发环境。

### 🔧 环境要求
- **操作系统**：Windows 10 / 11（64 位）
- **Visual Studio 2022**（版本 ≥ 17.0）
  - 安装时务必勾选 **“使用 C++ 的桌面开发”** 工作负载
- **Qt 6.x for MSVC 2022 64-bit**
  - 推荐版本：Qt 6.5.x 或 6.6.x
  - 下载地址：[https://www.qt.io/download](https://www.qt.io/download)
- **Qt Visual Studio Tools**
  - 在 Visual Studio Installer 中搜索并安装 “Qt Visual Studio Tools” 扩展

#### 🛠 编译步骤

##### 1. 打开项目
- 双击根目录中的 `EngineSimulator.sln` 文件，使用 Visual Studio 2022 打开解决方案。

##### 2. 配置 Qt 版本
- 在 Visual Studio 顶部菜单栏，点击 **Qt VS Tools → Qt Options**。
- 在 Qt Versions 选项卡中，点击 **Add**。
- 路径示例：`C:\Qt\6.5.0\msvc2019_64`
- 确保项目属性中指定的 Qt 版本与此一致（右键项目 → Qt Project Settings）。

##### 3. 生成解决方案
- 在 Visual Studio 工具栏中设置：
  - **平台**：`x64`
  - **配置**：`Release`（或 `Debug` 用于调试）
- 点击菜单：**生成 → 生成解决方案**（快捷键 `Ctrl+Shift+B`）

##### 4. 运行程序
- 编译成功后，可直接点击 **调试 → 开始执行(不调试)**（`Ctrl+F5`）运行。
- 生成的可执行文件位于：`EngineSimulator/x64/Release/EngineSimulator.exe`
- 程序会在该目录下自动创建 `DataLogging/` 并写入日志。

---

### ❗ 重要说明

- **日志路径**：日志始终写入 **可执行文件所在目录下的 `DataLogging/`**，与源码路径无关。
- **静态 vs 动态**：当前 `Release/` 中包含 `.dll` 文件，表明此为**动态链接打包版本**（使用 `windeployqt` 工具生成）。若需真正静态编译（单一 exe 无 dll），需使用 Qt 静态构建版本并修改项目配置（`/MT` 运行库），但此过程较复杂，一般用户无需操作。
- **兼容性**：本程序仅支持 **64 位 Windows**，不兼容 32 位系统。
- **首次运行**：请确保目标文件夹有写权限（避免放在 `Program Files` 等受保护目录）。

---


## 二、 功能完成情况说明

本项目已**100% 实现作业要求中的全部功能**，包括：

### 基础项（70 分）
- ✅ **数据模拟生成模块（30 分）**  
  按照 5ms 时间步长生成左右发动机 N1 转速、EGT 温度、燃油余量与流速，严格遵循启动（线性+对数）、稳态（±3% 波动）、停车（对数衰减）三阶段规则。
- ✅ **数据处理与界面显示模块（30 分）**  
  包含 N1 表盘（0°~210° 扇形）、EGT 表盘、燃油流速数字显示、Start/Run 状态指示灯，UI 每 500ms~1s 刷新。
- ✅ **表格文件记录（10 分）**  
  每次运行自动生成 `DataLogging/` 目录，以时间戳命名 `.csv` 文件，记录运行时间 + 所有传感器数据。

### 进阶项（30 分）
- ✅ **推力控制（10 分）**  
  “Increase Thrust” / “Decrease Thrust” 按钮可动态调整燃油流速（±1 单位/s），N1 与 EGT 随机增减 3%~5%。
- ✅ **14 种异常检测与告警（20 分）**  
  支持全部 14 类异常（传感器故障、超转、超温、低油、流速超限等），告警级别分为：
  - **正常（白色）**
  - **白色警告（单传感器故障）**
  - **琥珀色警告（发动机级异常）**
  - **红色警告（系统级故障，触发自动停车）**
  - 告警信息在 UI 显示 5 秒，同类型 5 秒内仅记录一次，并同步写入 `.log` 文件。

---

## 三、 主要功能与对应代码实现

### 1. 数据生成与阶段控制
- **文件**：`DataGenerator.h` / `DataGenerator.cpp`
- **关键逻辑**：
  ```cpp
  void DataGenerator::updateData() {
      // 燃油余量更新
      auxData.fuelLevel = data.fuelLevel -= data.fuelFlow * TIME_STEP;
      // 更新时间
      data.elapsedTime += TIME_STEP;

      // 根据当前阶段更新数据
      switch (data.phase) {
          case EnginePhase::Idle: break;
          case EnginePhase::Starting: /* 启动阶段逻辑 */;
          case EnginePhase::Stable: data = auxData; break;
          case EnginePhase::Stopping: /* 停车对数衰减 */;
      }
      // 推力控制与随机扰动
      checkPhase(); // 检查阶段转换
  }
  ```

### 2. 异常状态管理与告警
- **文件**：`EngineState.h`、`EngineSimulator.cpp`、`Logger.cpp`
- **结构体定义**：
  ```cpp
  struct AnomalyState {
      bool N1LS1Fail; bool N1RS1Fail; // ... 传感器故障标志
      int N1OverSpeedLevel;  // 0-无超转，1-超转1，2-超转2
      int EGTOverSpeedLevel; // 0-无超温，1~4-四级超温
      bool LowFuel; bool FFOverSpeed; bool FuelSFailure;
  };
  ```
- **告警去重与记录（Logger.cpp）**：
  ```cpp
  void Logger::triggerAlert(double elapsedTime, AlertLevel level, const QString & alertMessage, QTextEdit * alertDisplay) {
      // 5秒内同消息不重复
      if (lastAlertTime.contains(msg)) {
          if (elapsedTime - lastAlertTime[msg] < 5.0) {
              return;
          }
      }
      lastAlertTime[msg] = elapsedTime;
      // 写入 .log 文件 + UI 显示
  }
  ```

### 3. 日志系统
- **文件**：`Logger.h` / `Logger.cpp`
- **目录创建与文件初始化**：
  ```cpp
  void Logger::initLogFile() {
      QString exePath = QCoreApplication::applicationDirPath();
      QString loggingDir = exePath + "/DataLogging";
      QDir dir;
      if (!dir.exists(loggingDir)) {
          if (!dir.mkpath(loggingDir)) {
              qWarning() << "Failed to create DataLogging directory!";
              return;
          }
      }
      // 创建 .csv 和 .log 文件
  }
  ```

### 4. UI 交互与仪表盘
- **文件**：`EngineSimulator.cpp`、`GaugeWidget.h/cpp`
- **仪表盘颜色更新**：
  ```cpp
  n1LeftGauge->updateLevel(AlertLevel::amberWarning); // 琥珀色
  egtRightGauge->updateLevel(AlertLevel::redWarning); // 红色
  ```
- **告警按钮样式**（定义于 `EngineState.h`）如：
  ```cpp
  const QString CHECKED_BTN_STYLE_AMBER = R"(
      QPushButton {
          background-color: #FFC107;
          color: #222222;
          border: 2px solid #FFA000;
          border-radius: 5px;
          font-weight: bold;
      }
  )";
  ```

---

## 代码主要逻辑流程

1. **启动程序** → 点击 `Start` 按钮，`DataGenerator` 进入 `Starting` 阶段。
2. **5ms 定时器** → 调用 `generator.updateData()` 生成新数据。
3. **异常检测** → `checkData()` 检查 14 类异常，更新 `anomalyState`。
4. **UI 更新** → `updateDisplay()` 刷新仪表盘、指示灯、燃油流速。
5. **日志记录** → `logger.logDataAndAlerts()` 写入 `.csv` 数据，并触发告警（含 5 秒去重）。
6. **停车逻辑** → `Stop` 按钮或红色警告触发 `Stopping` 阶段，10 秒后归零。

---

## 四、 采用 Qt 与当前代码结构的优势

### 1. **Qt 框架优势**
- **信号与槽机制**：天然支持 5ms/1s 双定时器、按钮点击等异步事件，逻辑清晰。
- **跨平台 UI**：`QMainWindow` + `.ui` 文件设计，界面美观且易于维护。
- **国际化与资源管理**：支持 `.qrc` 资源嵌入，未来可轻松添加多语言或图标。

### 2. **模块化设计**
- **高内聚低耦合**：
  - `DataGenerator`：纯数据逻辑，无 UI 依赖。
  - `Logger`：独立日志模块，支持去重、多文件输出。
  - `EngineSimulator`：仅负责 UI 交互与模块调度。
- **易扩展**：新增异常类型只需在 `AnomalyState` 中添加标志位，并在 `checkData()` 中补充检测逻辑。

### 3. **工程规范**
- **常量集中定义**（`EngineState.h`）：
  ```cpp
  constexpr double RATED_RPM = 40000.0;
  constexpr double TIME_STEP = 0.005;
  ```
- **枚举清晰**：
  ```cpp
  enum class EnginePhase { Idle, Starting, Stable, Stopping };
  enum class AlertLevel { normal, whiteWarning, amberWarning, redWarning };
  ```
- **内存安全**：使用 `QTimer`（父对象管理）、智能指针风格资源管理。

---


