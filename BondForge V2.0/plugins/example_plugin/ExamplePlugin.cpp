#include "ExamplePlugin.h"
#include <QAction>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QUuid>
#include <QRegularExpression>
#include <QJsonDocument>

ExamplePlugin::ExamplePlugin(QObject *parent)
    : QObject(parent)
    , m_context(nullptr)
    , m_name("Example Plugin")
    , m_version("1.0.0")
    , m_description("A comprehensive example plugin for BondForge demonstrating UI integration and chemical calculations")
    , m_author("BondForge Team")
    , m_website("https://bondforge.org")
    , m_initialized(false)
{
    // Initialize metadata
    m_metadata = QJsonObject{
        {"name", m_name},
        {"version", m_version},
        {"description", m_description},
        {"author", m_author},
        {"website", m_website},
        {"dependencies", QJsonArray()},
        {"extensions", QJsonArray{"molecular_calc", "solubility_predict", "image_gen"}},
        {"license", "MIT"},
        {"url", "https://github.com/bondforge/example-plugin"},
        {"min_bondforge_version", "2.0.0"}
    };
    
    registerCommands();
}

ExamplePlugin::~ExamplePlugin()
{
    shutdown();
}

QString ExamplePlugin::name() const
{
    return m_name;
}

QString ExamplePlugin::version() const
{
    return m_version;
}

QString ExamplePlugin::description() const
{
    return m_description;
}

QString ExamplePlugin::author() const
{
    return m_author;
}

QString ExamplePlugin::website() const
{
    return m_website;
}

QStringList ExamplePlugin::dependencies() const
{
    return m_dependencies;
}

bool ExamplePlugin::initialize(BondForge::Core::Plugins::PluginContext *context)
{
    if (m_initialized) {
        return true;
    }
    
    m_context = context;
    
    if (!m_context) {
        qDebug() << "Plugin context is null";
        return false;
    }
    
    // 设置UI
    setupUI();
    
    m_initialized = true;
    qDebug() << "ExamplePlugin initialized";
    return true;
}

void ExamplePlugin::shutdown()
{
    if (!m_initialized) {
        return;
    }
    
    // 清理资源
    // 在这里可以移除菜单项、工具栏按钮等
    
    m_context = nullptr;
    m_initialized = false;
    
    qDebug() << "ExamplePlugin shutdown";
}

void ExamplePlugin::configure()
{
    if (!m_initialized) {
        return;
    }
    
    QMessageBox::information(nullptr, 
                             "Example Plugin Configuration", 
                             "This is a configuration dialog for Example Plugin.\n"
                             "In a real plugin, you would have actual configuration options here.");
}

void ExamplePlugin::execute(const QString &command, const QVariantMap &parameters)
{
    Q_UNUSED(parameters)
    
    if (!m_initialized) {
        return;
    }
    
    if (command == "show_message") {
        QString message = parameters.value("message", "Hello from Example Plugin!").toString();
        QMessageBox::information(nullptr, "Example Plugin", message);
    } else if (command == "add_menu_item") {
        QString title = parameters.value("title", "Example Action").toString();
        
        if (m_context && m_context->mainWindow) {
            QAction *action = new QAction(title, this);
            connect(action, &QAction::triggered, this, &ExamplePlugin::onMenuAction);
            
            // 添加到工具菜单
            QMenu *toolsMenu = m_context->mainWindow->findChild<QMenu*>("toolsMenu");
            if (toolsMenu) {
                toolsMenu->addAction(action);
            } else {
                // 如果没有工具菜单，可以添加到其他菜单或创建新菜单
                QMenuBar *menuBar = m_context->mainWindow->menuBar();
                if (menuBar) {
                    QMenu *toolsMenu = menuBar->addMenu("Tools");
                    toolsMenu->addAction(action);
                }
            }
        }
    }
}

QVariantMap ExamplePlugin::executeWithReturn(const QString &command, const QVariantMap &parameters)
{
    QVariantMap result;
    
    if (!m_initialized) {
        result["success"] = false;
        result["error"] = "Plugin not initialized";
        return result;
    }
    
    if (command == "get_info") {
        result["success"] = true;
        result["name"] = m_name;
        result["version"] = m_version;
        result["description"] = m_description;
        result["author"] = m_author;
        result["website"] = m_website;
    } else if (command == "generate_uuid") {
        result["success"] = true;
        result["uuid"] = QUuid::createUuid().toString();
    } else if (command == "molecular_calc") {
        QString formula = parameters.value("formula", "").toString();
        if (!formula.isEmpty()) {
            result["success"] = true;
            result["molecular_weight"] = calculateMolecularWeight(formula);
        } else {
            result["success"] = false;
            result["error"] = "No formula provided";
        }
    } else if (command == "solubility_predict") {
        QVariantMap properties = parameters.value("properties", QVariantMap()).toMap();
        if (!properties.isEmpty()) {
            result["success"] = true;
            result["solubility"] = predictSolubility(properties);
        } else {
            result["success"] = false;
            result["error"] = "No properties provided";
        }
    } else if (command == "image_gen") {
        QString smiles = parameters.value("smiles", "").toString();
        QString format = parameters.value("format", "png").toString();
        if (!smiles.isEmpty()) {
            result["success"] = true;
            result["image_path"] = generateMoleculeImage(smiles, format);
        } else {
            result["success"] = false;
            result["error"] = "No SMILES provided";
        }
    } else {
        result["success"] = false;
        result["error"] = QString("Unknown command: %1").arg(command);
    }
    
    return result;
}

bool ExamplePlugin::isExecutable(const QString &command) const
{
    static const QStringList executableCommands = {
        "show_message", "add_menu_item", "get_info", "generate_uuid",
        "molecular_calc", "solubility_predict", "image_gen"
    };
    
    return executableCommands.contains(command);
}

QStringList ExamplePlugin::commands() const
{
    return QStringList() << "show_message" << "add_menu_item" << "get_info" 
                        << "generate_uuid" << "molecular_calc" << "solubility_predict" 
                        << "image_gen";
}

QJsonObject ExamplePlugin::getMetadata() const
{
    return m_metadata;
}

void ExamplePlugin::onMenuAction()
{
    QMessageBox::information(nullptr, 
                             "Example Plugin", 
                             "Example action triggered from menu!");
}

void ExamplePlugin::processQueue()
{
    // 处理队列中的任务
    // 这里可以实现一些定期执行的任务
}

void ExamplePlugin::setupUI()
{
    if (!m_context || !m_context->mainWindow) {
        return;
    }
    
    // 添加菜单项
    QMenuBar *menuBar = m_context->mainWindow->menuBar();
    if (!menuBar) {
        return;
    }
    
    // 查找或创建工具菜单
    QMenu *toolsMenu = nullptr;
    foreach (QAction *action, menuBar->actions()) {
        QMenu *menu = action->menu();
        if (menu && menu->title() == "Tools") {
            toolsMenu = menu;
            break;
        }
    }
    
    if (!toolsMenu) {
        toolsMenu = menuBar->addMenu("Tools");
    }
    
    // 添加插件菜单项
    QAction *exampleAction = new QAction("Example Plugin Action", this);
    connect(exampleAction, &QAction::triggered, this, &ExamplePlugin::onMenuAction);
    toolsMenu->addAction(exampleAction);
    
    // 添加工具栏按钮
    QToolBar *mainToolBar = m_context->mainWindow->findChild<QToolBar*>("mainToolBar");
    if (mainToolBar) {
        QAction *toolBarAction = new QAction("Example", this);
        toolBarAction->setToolTip("Example Plugin Tool");
        connect(toolBarAction, &QAction::triggered, this, &ExamplePlugin::onMenuAction);
        mainToolBar->addAction(toolBarAction);
    }
    
    // 设置状态栏消息
    if (QStatusBar *statusBar = m_context->mainWindow->statusBar()) {
        statusBar->showMessage("Example Plugin loaded", 3000);
    }
    
    // 设置定时器处理队列
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ExamplePlugin::processQueue);
    timer->start(5000); // 每5秒处理一次队列
}

void ExamplePlugin::registerCommands()
{
    // 在这里注册插件支持的命令
    // 这可以被插件管理器使用
}

QString ExamplePlugin::calculateMolecularWeight(const QString& formula)
{
    // Simple molecular weight calculation
    // This is just an example implementation
    
    QRegularExpression elementRegex("([A-Z][a-z]*)(\\d*)");
    QRegularExpressionMatchIterator it = elementRegex.globalMatch(formula);
    
    double totalWeight = 0.0;
    QMap<QString, double> atomicWeights = {
        {"H", 1.008}, {"C", 12.011}, {"N", 14.007}, {"O", 15.999},
        {"F", 18.998}, {"P", 30.974}, {"S", 32.06}, {"Cl", 35.45}
    };
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString element = match.captured(1);
        QString countStr = match.captured(2);
        int count = countStr.isEmpty() ? 1 : countStr.toInt();
        
        if (atomicWeights.contains(element)) {
            totalWeight += atomicWeights[element] * count;
        }
    }
    
    return QString::number(totalWeight, 'f', 2);
}

QString ExamplePlugin::predictSolubility(const QVariantMap& properties)
{
    // Simple solubility prediction based on logP
    // This is just an example implementation
    
    double logp = properties.value("logp", 0.0).toDouble();
    double polarSurfaceArea = properties.value("polar_surface_area", 0.0).toDouble();
    double molecularWeight = properties.value("molecular_weight", 0.0).toDouble();
    
    // Simple empirical formula for demonstration
    double solubility = 1.0 / (1.0 + qAbs(logp) + polarSurfaceArea/100.0 + molecularWeight/1000.0);
    
    return QString::number(solubility, 'f', 3);
}

QString ExamplePlugin::generateMoleculeImage(const QString& smiles, const QString& format)
{
    // This would normally call RDKit or another cheminformatics library
    // For demonstration purposes, we'll just return a placeholder
    
    Q_UNUSED(smiles)
    Q_UNUSED(format)
    
    return "/tmp/molecule_image.png"; // Placeholder path
}