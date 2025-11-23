#include "VisualizationWidget.h"
#include "core/chemistry/MoleculeRenderer.h"
#include "utils/Logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QToolBar>
#include <QAction>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QColorDialog>
#include <QSpinBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QTextEdit>
#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QWebChannel>
#include <QOpenGLWidget>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

namespace UI {

VisualizationWidget::VisualizationWidget(QWidget *parent)
    : QWidget(parent)
    , m_moleculeRenderer(nullptr)
    , m_2dView(nullptr)
    , m_3dView(nullptr)
    , m_webView(nullptr)
    , m_propertiesWidget(nullptr)
    , m_renderingOptionsWidget(nullptr)
    , m_currentViewType(View2D)
    , m_currentMolecule(nullptr)
{
    setupUI();
    setupConnections();
    
    // 创建分子渲染器
    m_moleculeRenderer = std::make_unique<Core::Chemistry::MoleculeRenderer>();
    
    Utils::Logger::info("VisualizationWidget initialized");
}

VisualizationWidget::~VisualizationWidget()
{
    if (m_currentMolecule) {
        delete m_currentMolecule;
    }
}

void VisualizationWidget::setupUI()
{
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 工具栏
    QToolBar* toolBar = new QToolBar(tr("Visualization Tools"), this);
    setupToolBar(toolBar);
    mainLayout->addWidget(toolBar);
    
    // 分割器：左侧视图，右侧控制面板
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    
    // 左侧：可视化视图区域（选项卡式）
    m_viewTabWidget = new QTabWidget(this);
    setupViews();
    splitter->addWidget(m_viewTabWidget);
    
    // 右侧：属性和控制面板
    QWidget* rightPanel = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    
    // 分子属性面板
    setupPropertiesPanel();
    rightLayout->addWidget(m_propertiesWidget);
    
    // 渲染选项面板
    setupRenderingOptionsPanel();
    rightLayout->addWidget(m_renderingOptionsWidget);
    
    splitter->addWidget(rightPanel);
    splitter->setSizes({600, 300});
    
    mainLayout->addWidget(splitter);
    
    // 状态栏
    setupStatusBar();
    mainLayout->addWidget(m_statusBar);
}

void VisualizationWidget::setupToolBar(QToolBar* toolBar)
{
    // 文件操作
    QAction* openAction = new QAction(QIcon(":/icons/open.png"), tr("Open Molecule"), this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &VisualizationWidget::openMolecule);
    toolBar->addAction(openAction);
    
    QAction* saveAction = new QAction(QIcon(":/icons/save.png"), tr("Save Molecule"), this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &VisualizationWidget::saveMolecule);
    toolBar->addAction(saveAction);
    
    toolBar->addSeparator();
    
    // 视图切换
    QAction* view2DAction = new QAction(QIcon(":/icons/2d.png"), tr("2D View"), this);
    view2DAction->setCheckable(true);
    view2DAction->setChecked(true);
    connect(view2DAction, &QAction::triggered, [this]() { switchView(View2D); });
    toolBar->addAction(view2DAction);
    
    QAction* view3DAction = new QAction(QIcon(":/icons/3d.png"), tr("3D View"), this);
    view3DAction->setCheckable(true);
    connect(view3DAction, &QAction::triggered, [this]() { switchView(View3D); });
    toolBar->addAction(view3DAction);
    
    QAction* viewWebAction = new QAction(QIcon(":/icons/web.png"), tr("Web View"), this);
    viewWebAction->setCheckable(true);
    connect(viewWebAction, &QAction::triggered, [this]() { switchView(ViewWeb); });
    toolBar->addAction(viewWebAction);
    
    m_viewActionGroup = new QActionGroup(this);
    m_viewActionGroup->addAction(view2DAction);
    m_viewActionGroup->addAction(view3DAction);
    m_viewActionGroup->addAction(viewWebAction);
    
    toolBar->addSeparator();
    
    // 视图控制
    m_zoomSlider = new QSlider(Qt::Horizontal, this);
    m_zoomSlider->setRange(10, 500);
    m_zoomSlider->setValue(100);
    m_zoomSlider->setMaximumWidth(150);
    connect(m_zoomSlider, &QSlider::valueChanged, this, &VisualizationWidget::setZoomLevel);
    
    QLabel* zoomLabel = new QLabel(tr("Zoom:"), this);
    toolBar->addWidget(zoomLabel);
    toolBar->addWidget(m_zoomSlider);
    
    m_zoomLevelLabel = new QLabel("100%", this);
    toolBar->addWidget(m_zoomLevelLabel);
    
    toolBar->addSeparator();
    
    // 渲染控制
    m_renderButton = new QPushButton(tr("Render"), this);
    connect(m_renderButton, &QPushButton::clicked, this, &VisualizationWidget::renderMolecule);
    toolBar->addWidget(m_renderButton);
    
    QAction* clearAction = new QAction(QIcon(":/icons/clear.png"), tr("Clear"), this);
    connect(clearAction, &QAction::triggered, this, &VisualizationWidget::clearView);
    toolBar->addAction(clearAction);
}

void VisualizationWidget::setupViews()
{
    // 2D视图 - 基于QGraphicsView的分子2D渲染
    m_2dView = new QGraphicsView(this);
    m_2dView->setRenderHint(QPainter::Antialiasing);
    m_2dView->setDragMode(QGraphicsView::RubberBandDrag);
    m_2dView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    
    m_2dScene = new QGraphicsScene(this);
    m_2dScene->setSceneRect(-500, -500, 1000, 1000);
    m_2dView->setScene(m_2dScene);
    
    m_viewTabWidget->addTab(m_2dView, tr("2D View"));
    
    // 3D视图 - 基于QOpenGLWidget的3D分子渲染
    m_3dView = new QOpenGLWidget(this);
    m_viewTabWidget->addTab(m_3dView, tr("3D View"));
    
    // Web视图 - 基于QWebEngineView的WebGL分子渲染
    m_webView = new QWebEngineView(this);
    m_webView->settings()->setAttribute(QWebEngineSettings::WebGLEnabled, true);
    m_webView->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    m_viewTabWidget->addTab(m_webView, tr("Web View"));
    
    // 设置当前视图为2D视图
    m_currentViewType = View2D;
}

void VisualizationWidget::setupPropertiesPanel()
{
    m_propertiesWidget = new QGroupBox(tr("Molecule Properties"), this);
    
    QFormLayout* layout = new QFormLayout(m_propertiesWidget);
    
    // 基本信息
    m_nameLabel = new QLabel(tr("No molecule loaded"), this);
    layout->addRow(tr("Name:"), m_nameLabel);
    
    m_formulaLabel = new QLabel("-", this);
    layout->addRow(tr("Molecular Formula:"), m_formulaLabel);
    
    m_mwLabel = new QLabel("-", this);
    layout->addRow(tr("Molecular Weight:"), m_mwLabel);
    
    m_atomsCountLabel = new QLabel("0", this);
    layout->addRow(tr("Atoms Count:"), m_atomsCountLabel);
    
    m_bondsCountLabel = new QLabel("0", this);
    layout->addRow(tr("Bonds Count:"), m_bondsCountLabel);
    
    layout->addItem(new QSpacerItem(0, 20, QSizePolicy::Minimum, QSizePolicy::Fixed));
    
    // 详细信息文本框
    m_detailsTextEdit = new QTextEdit(this);
    m_detailsTextEdit->setReadOnly(true);
    m_detailsTextEdit->setMaximumHeight(150);
    layout->addRow(tr("Details:"), m_detailsTextEdit);
}

void VisualizationWidget::setupRenderingOptionsPanel()
{
    m_renderingOptionsWidget = new QGroupBox(tr("Rendering Options"), this);
    
    QVBoxLayout* layout = new QVBoxLayout(m_renderingOptionsWidget);
    
    // 渲染模式
    QGroupBox* renderModeGroup = new QGroupBox(tr("Render Mode"), this);
    QFormLayout* renderModeLayout = new QFormLayout(renderModeGroup);
    
    m_renderStyleCombo = new QComboBox(this);
    m_renderStyleCombo->addItem(tr("Ball and Stick"), "ball_stick");
    m_renderStyleCombo->addItem(tr("Space Filling"), "space_filling");
    m_renderStyleCombo->addItem(tr("Wireframe"), "wireframe");
    m_renderStyleCombo->addItem(tr("Sticks"), "sticks");
    m_renderStyleCombo->setCurrentIndex(0);
    connect(m_renderStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VisualizationWidget::updateRenderingStyle);
    renderModeLayout->addRow(tr("Style:"), m_renderStyleCombo);
    
    m_colorSchemeCombo = new QComboBox(this);
    m_colorSchemeCombo->addItem(tr("Element"), "element");
    m_colorSchemeCombo->addItem(tr("Charge"), "charge");
    m_colorSchemeCombo->addItem(tr("Chain"), "chain");
    m_colorSchemeCombo->addItem(tr("Secondary Structure"), "secondary_structure");
    m_colorSchemeCombo->setCurrentIndex(0);
    connect(m_colorSchemeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &VisualizationWidget::updateColorScheme);
    renderModeLayout->addRow(tr("Color Scheme:"), m_colorSchemeCombo);
    
    layout->addWidget(renderModeGroup);
    
    // 显示选项
    QGroupBox* displayOptionsGroup = new QGroupBox(tr("Display Options"), this);
    QVBoxLayout* displayOptionsLayout = new QVBoxLayout(displayOptionsGroup);
    
    m_showHydrogensCheck = new QCheckBox(tr("Show Hydrogens"), this);
    m_showHydrogensCheck->setChecked(true);
    connect(m_showHydrogensCheck, &QCheckBox::toggled, this, &VisualizationWidget::updateDisplayOptions);
    displayOptionsLayout->addWidget(m_showHydrogensCheck);
    
    m_showLabelsCheck = new QCheckBox(tr("Show Atom Labels"), this);
    connect(m_showLabelsCheck, &QCheckBox::toggled, this, &VisualizationWidget::updateDisplayOptions);
    displayOptionsLayout->addWidget(m_showLabelsCheck);
    
    m_showBondsCheck = new QCheckBox(tr("Show Bonds"), this);
    m_showBondsCheck->setChecked(true);
    connect(m_showBondsCheck, &QCheckBox::toggled, this, &VisualizationWidget::updateDisplayOptions);
    displayOptionsLayout->addWidget(m_showBondsCheck);
    
    layout->addWidget(displayOptionsGroup);
    
    // 高级选项
    QGroupBox* advancedOptionsGroup = new QGroupBox(tr("Advanced Options"), this);
    QFormLayout* advancedOptionsLayout = new QFormLayout(advancedOptionsGroup);
    
    m_bondRadiusSpinBox = new QDoubleSpinBox(this);
    m_bondRadiusSpinBox->setRange(0.1, 2.0);
    m_bondRadiusSpinBox->setSingleStep(0.1);
    m_bondRadiusSpinBox->setValue(0.3);
    m_bondRadiusSpinBox->setSuffix(" Å");
    connect(m_bondRadiusSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &VisualizationWidget::updateAdvancedOptions);
    advancedOptionsLayout->addRow(tr("Bond Radius:"), m_bondRadiusSpinBox);
    
    m_atomScaleSpinBox = new QDoubleSpinBox(this);
    m_atomScaleSpinBox->setRange(0.1, 3.0);
    m_atomScaleSpinBox->setSingleStep(0.1);
    m_atomScaleSpinBox->setValue(1.0);
    connect(m_atomScaleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &VisualizationWidget::updateAdvancedOptions);
    advancedOptionsLayout->addRow(tr("Atom Scale:"), m_atomScaleSpinBox);
    
    layout->addWidget(advancedOptionsGroup);
    
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

void VisualizationWidget::setupStatusBar()
{
    m_statusBar = new QStatusBar(this);
    
    m_statusLabel = new QLabel(tr("Ready"), this);
    m_statusBar->addWidget(m_statusLabel);
    
    m_atomInfoLabel = new QLabel(this);
    m_statusBar->addPermanentWidget(m_atomInfoLabel);
}

void VisualizationWidget::setupConnections()
{
    // 连接视图切换信号
    connect(m_viewTabWidget, &QTabWidget::currentChanged, 
            [this](int index) {
                switch (index) {
                    case 0: m_currentViewType = View2D; break;
                    case 1: m_currentViewType = View3D; break;
                    case 2: m_currentViewType = ViewWeb; break;
                }
                
                if (m_currentMolecule) {
                    updateView();
                }
            });
}

void VisualizationWidget::openMolecule()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open Molecule"),
        QString(),
        tr("Molecule Files (*.mol *.sdf *.smi *.pdb *.xyz *.cml);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        loadMolecule(fileName);
    }
}

void VisualizationWidget::loadMolecule(const QString& fileName)
{
    if (!m_moleculeRenderer) {
        QMessageBox::critical(this, tr("Error"), tr("Molecule renderer not initialized"));
        return;
    }
    
    // 使用分子渲染器加载分子
    if (m_currentMolecule) {
        delete m_currentMolecule;
    }
    
    m_currentMolecule = m_moleculeRenderer->loadMolecule(fileName);
    
    if (m_currentMolecule) {
        updateMoleculeInfo();
        renderMolecule();
        m_statusLabel->setText(tr("Molecule loaded: %1").arg(fileName));
    } else {
        m_statusLabel->setText(tr("Failed to load molecule: %1").arg(fileName));
        QMessageBox::critical(this, tr("Error"), tr("Failed to load molecule file"));
    }
}

void VisualizationWidget::saveMolecule()
{
    if (!m_currentMolecule) {
        QMessageBox::information(this, tr("Information"), tr("No molecule to save"));
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save Molecule"),
        QString(),
        tr("Molecule Files (*.mol *.sdf *.pdb *.xyz);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        if (m_moleculeRenderer->saveMolecule(m_currentMolecule, fileName)) {
            m_statusLabel->setText(tr("Molecule saved: %1").arg(fileName));
        } else {
            m_statusLabel->setText(tr("Failed to save molecule: %1").arg(fileName));
            QMessageBox::critical(this, tr("Error"), tr("Failed to save molecule file"));
        }
    }
}

void VisualizationWidget::renderMolecule()
{
    if (!m_currentMolecule) {
        QMessageBox::information(this, tr("Information"), tr("No molecule to render"));
        return;
    }
    
    if (!m_moleculeRenderer) {
        QMessageBox::critical(this, tr("Error"), tr("Molecule renderer not initialized"));
        return;
    }
    
    // 获取当前渲染选项
    Core::Chemistry::RenderingOptions options;
    options.style = m_renderStyleCombo->currentData().toString();
    options.colorScheme = m_colorSchemeCombo->currentData().toString();
    options.showHydrogens = m_showHydrogensCheck->isChecked();
    options.showLabels = m_showLabelsCheck->isChecked();
    options.showBonds = m_showBondsCheck->isChecked();
    options.bondRadius = m_bondRadiusSpinBox->value();
    options.atomScale = m_atomScaleSpinBox->value();
    
    switch (m_currentViewType) {
        case View2D:
            render2DView(options);
            break;
        case View3D:
            render3DView(options);
            break;
        case ViewWeb:
            renderWebView(options);
            break;
    }
    
    m_statusLabel->setText(tr("Molecule rendered successfully"));
}

void VisualizationWidget::render2DView(const Core::Chemistry::RenderingOptions& options)
{
    if (!m_moleculeRenderer || !m_currentMolecule) {
        return;
    }
    
    // 清除当前场景
    m_2dScene->clear();
    
    // 渲染2D分子结构到场景
    if (m_moleculeRenderer->renderToGraphicsScene(m_currentMolecule, m_2dScene, options)) {
        // 自适应视图大小
        m_2dView->fitInView(m_2dScene->sceneRect(), Qt::KeepAspectRatio);
    } else {
        // 如果渲染失败，使用简化的渲染
        renderSimple2DStructure();
    }
}

void VisualizationWidget::renderSimple2DStructure()
{
    if (!m_currentMolecule) {
        return;
    }
    
    // 简化的2D渲染实现
    // 这里使用基本图形元素绘制分子结构
    const double bondLength = 50.0;  // 键长度（像素）
    const double atomRadius = 10.0;  // 原子半径（像素）
    
    QPen pen(Qt::black, 2);
    QBrush brush(Qt::white);
    
    // 绘制分子结构
    // 这是一个简化的示例，实际实现应该根据分子结构数据绘制
    m_2dScene->addEllipse(-atomRadius, -atomRadius, atomRadius*2, atomRadius*2, pen, brush);
    m_2dScene->addEllipse(-atomRadius + bondLength, -atomRadius, atomRadius*2, atomRadius*2, pen, brush);
    m_2dScene->addLine(0, 0, bondLength, 0, pen);
    
    // 添加原子标签
    QFont font("Arial", 8);
    m_2dScene->addText("C", font)->setPos(-atomRadius/2, -atomRadius/2);
    m_2dScene->addText("C", font)->setPos(bondLength-atomRadius/2, -atomRadius/2);
    
    // 自适应视图大小
    m_2dView->fitInView(m_2dScene->sceneRect(), Qt::KeepAspectRatio);
}

void VisualizationWidget::render3DView(const Core::Chemistry::RenderingOptions& options)
{
    if (!m_moleculeRenderer || !m_currentMolecule) {
        return;
    }
    
    // 3D渲染实现
    // 在实际实现中，这里应该使用OpenGL渲染3D分子结构
    // 这里我们留空，只是显示一个占位符
    if (!m_3dView->isValid()) {
        m_3dView->makeCurrent();
    }
    
    // 发送更新信号触发重绘
    m_3dView->update();
}

void VisualizationWidget::renderWebView(const Core::Chemistry::RenderingOptions& options)
{
    if (!m_moleculeRenderer || !m_currentMolecule) {
        return;
    }
    
    // Web视图实现
    // 在实际实现中，这里应该生成WebGL代码来渲染3D分子
    // 这里我们加载一个简单的HTML页面作为占位符
    QString html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>3D Molecular Viewer</title>
    <meta charset="utf-8">
    <script src="https://cdnjs.cloudflare.com/ajax/libs/three.js/r128/three.min.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/three@0.128.0/examples/js/loaders/PDBLoader.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/three@0.128.0/examples/js/controls/OrbitControls.js"></script>
    <style>
        body { margin: 0; overflow: hidden; }
        canvas { display: block; }
    </style>
</head>
<body>
    <div id="container"></div>
    
    <script>
        // 初始化3D场景
        var container = document.getElementById('container');
        var scene = new THREE.Scene();
        var camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
        var renderer = new THREE.WebGLRenderer();
        renderer.setSize(window.innerWidth, window.innerHeight);
        container.appendChild(renderer.domElement);
        
        // 添加控制器
        var controls = new THREE.OrbitControls(camera, renderer.domElement);
        controls.enableDamping = true;
        controls.dampingFactor = 0.05;
        controls.enableZoom = true;
        
        // 添加灯光
        var ambientLight = new THREE.AmbientLight(0x404040);
        scene.add(ambientLight);
        
        var directionalLight = new THREE.DirectionalLight(0xffffff, 0.8);
        directionalLight.position.set(1, 1, 1);
        scene.add(directionalLight);
        
        // 添加简单的分子结构（苯环）
        var benzeneGeometry = new THREE.BufferGeometry();
        var vertices = new Float32Array([
            0, 0, 0,
            1, 0, 0,
            1.5, 0.87, 0,
            1, 1.73, 0,
            0, 1.73, 0,
            -0.5, 0.87, 0
        ]);
        
        benzeneGeometry.setAttribute('position', new THREE.BufferAttribute(vertices, 3));
        
        var benzeneMaterial = new THREE.PointsMaterial({ color: 0x00ff00, size: 0.2 });
        var benzene = new THREE.Points(benzeneGeometry, benzeneMaterial);
        scene.add(benzene);
        
        // 添加键
        var bondGeometry = new THREE.BufferGeometry();
        var bondVertices = new Float32Array([
            0, 0, 0,
            1, 0, 0,
            1, 0, 0,
            1.5, 0.87, 0,
            1.5, 0.87, 0,
            1, 1.73, 0,
            1, 1.73, 0,
            0, 1.73, 0,
            0, 1.73, 0,
            -0.5, 0.87, 0,
            -0.5, 0.87, 0,
            0, 0, 0
        ]);
        
        bondGeometry.setAttribute('position', new THREE.BufferAttribute(bondVertices, 3));
        
        var bondMaterial = new THREE.LineBasicMaterial({ color: 0x0000ff });
        var bonds = new THREE.LineSegments(bondGeometry, bondMaterial);
        scene.add(bonds);
        
        // 设置相机位置
        camera.position.z = 5;
        
        // 动画循环
        function animate() {
            requestAnimationFrame(animate);
            controls.update();
            renderer.render(scene, camera);
        }
        
        animate();
        
        // 窗口大小调整
        window.addEventListener('resize', function() {
            camera.aspect = window.innerWidth / window.innerHeight;
            camera.updateProjectionMatrix();
            renderer.setSize(window.innerWidth, window.innerHeight);
        });
    </script>
</body>
</html>
    )";
    
    m_webView->setHtml(html);
}

void VisualizationWidget::clearView()
{
    if (m_currentViewType == View2D && m_2dScene) {
        m_2dScene->clear();
    }
    
    if (m_currentViewType == View3D && m_3dView) {
        // 清除3D视图
    }
    
    if (m_currentViewType == ViewWeb && m_webView) {
        m_webView->setHtml("");
    }
    
    m_statusLabel->setText(tr("View cleared"));
}

void VisualizationWidget::switchView(ViewType viewType)
{
    if (m_currentViewType == viewType) {
        return;
    }
    
    m_currentViewType = viewType;
    
    switch (viewType) {
        case View2D:
            m_viewTabWidget->setCurrentIndex(0);
            break;
        case View3D:
            m_viewTabWidget->setCurrentIndex(1);
            break;
        case ViewWeb:
            m_viewTabWidget->setCurrentIndex(2);
            break;
    }
    
    if (m_currentMolecule) {
        updateView();
    }
}

void VisualizationWidget::setZoomLevel(int level)
{
    m_zoomLevelLabel->setText(QString("%1%").arg(level));
    
    double scale = level / 100.0;
    
    if (m_currentViewType == View2D && m_2dView) {
        QTransform transform;
        transform.scale(scale, scale);
        m_2dView->setTransform(transform);
    }
    
    // 3D视图和Web视图的缩放可以通过相机控制实现
}

void VisualizationWidget::updateView()
{
    renderMolecule();
}

void VisualizationWidget::updateRenderingStyle()
{
    if (m_currentMolecule) {
        renderMolecule();
    }
}

void VisualizationWidget::updateColorScheme()
{
    if (m_currentMolecule) {
        renderMolecule();
    }
}

void VisualizationWidget::updateDisplayOptions()
{
    if (m_currentMolecule) {
        renderMolecule();
    }
}

void VisualizationWidget::updateAdvancedOptions()
{
    if (m_currentMolecule) {
        renderMolecule();
    }
}

void VisualizationWidget::updateMoleculeInfo()
{
    if (!m_currentMolecule) {
        return;
    }
    
    // 更新基本信息
    m_nameLabel->setText(m_currentMolecule->getName());
    m_formulaLabel->setText(m_currentMolecule->getMolecularFormula());
    m_mwLabel->setText(QString::number(m_currentMolecule->getMolecularWeight(), 'f', 2) + " g/mol");
    m_atomsCountLabel->setText(QString::number(m_currentMolecule->getAtomsCount()));
    m_bondsCountLabel->setText(QString::number(m_currentMolecule->getBondsCount()));
    
    // 更新详细信息
    QString details;
    details += tr("ID: %1\n").arg(m_currentMolecule->getId());
    details += tr("SMILES: %1\n").arg(m_currentMolecule->getSMILES());
    details += tr("InChI: %1\n").arg(m_currentMolecule->getInChI());
    details += tr("LogP: %1\n").arg(m_currentMolecule->getLogP(), 0, 'f', 3);
    details += tr("TPSA: %1\n").arg(m_currentMolecule->getTPSA(), 0, 'f', 1);
    details += tr("Rotatable Bonds: %1\n").arg(m_currentMolecule->getRotatableBonds());
    details += tr("H-Bond Donors: %1\n").arg(m_currentMolecule->getHBondDonors());
    details += tr("H-Bond Acceptors: %1\n").arg(m_currentMolecule->getHBondAcceptors());
    
    m_detailsTextEdit->setText(details);
}

} // namespace UI