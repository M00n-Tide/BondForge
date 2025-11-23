#include "MLAnalysisWidget.h"
#include "core/ml/MLModels.h"
#include "core/ml/StatisticalAnalysis.h"
#include "utils/Logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QToolBar>
#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QTextEdit>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QProgressBar>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QStandardPaths>
#include <QDir>

namespace UI {

MLAnalysisWidget::MLAnalysisWidget(QWidget *parent)
    : QWidget(parent)
    , m_currentModel(nullptr)
    , m_dataProcessor(nullptr)
    , m_resultsWidget(nullptr)
{
    setupUI();
    setupConnections();
    
    // 初始化数据处理对象
    m_dataProcessor = new Core::ML::DataProcessor(this);
    
    Utils::Logger::info("MLAnalysisWidget initialized");
}

MLAnalysisWidget::~MLAnalysisWidget()
{
    // 不需要手动删除m_currentModel，由MLModelManager管理
}

void MLAnalysisWidget::setupUI()
{
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 工具栏
    QToolBar* toolBar = new QToolBar(tr("Machine Learning Tools"), this);
    setupToolBar(toolBar);
    mainLayout->addWidget(toolBar);
    
    // 分割器：左侧配置区，右侧结果区
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    
    // 左侧：配置面板（选项卡式）
    QWidget* configPanel = new QWidget(this);
    QVBoxLayout* configLayout = new QVBoxLayout(configPanel);
    
    m_configTabWidget = new QTabWidget(this);
    setupConfigTabs();
    configLayout->addWidget(m_configTabWidget);
    
    splitter->addWidget(configPanel);
    
    // 右侧：结果面板
    setupResultsPanel();
    splitter->addWidget(m_resultsWidget);
    
    splitter->setSizes({400, 600});
    
    mainLayout->addWidget(splitter);
    
    // 状态栏
    setupStatusBar();
    mainLayout->addWidget(m_statusBar);
}

void MLAnalysisWidget::setupToolBar(QToolBar* toolBar)
{
    // 数据操作
    QAction* loadDataAction = new QAction(QIcon(":/icons/open.png"), tr("Load Data"), this);
    loadDataAction->setShortcut(QKeySequence::Open);
    connect(loadDataAction, &QAction::triggered, this, &MLAnalysisWidget::loadData);
    toolBar->addAction(loadDataAction);
    
    QAction* saveDataAction = new QAction(QIcon(":/icons/save.png"), tr("Save Data"), this);
    saveDataAction->setShortcut(QKeySequence::Save);
    connect(saveDataAction, &QAction::triggered, this, &MLAnalysisWidget::saveData);
    toolBar->addAction(saveDataAction);
    
    toolBar->addSeparator();
    
    // 模型操作
    QAction* trainModelAction = new QAction(QIcon(":/icons/train.png"), tr("Train Model"), this);
    connect(trainModelAction, &QAction::triggered, this, &MLAnalysisWidget::trainModel);
    toolBar->addAction(trainModelAction);
    
    QAction* evaluateModelAction = new QAction(QIcon(":/icons/evaluate.png"), tr("Evaluate Model"), this);
    connect(evaluateModelAction, &QAction::triggered, this, &MLAnalysisWidget::evaluateModel);
    toolBar->addAction(evaluateModelAction);
    
    QAction* predictAction = new QAction(QIcon(":/icons/predict.png"), tr("Predict"), this);
    connect(predictAction, &QAction::triggered, this, &MLAnalysisWidget::predict);
    toolBar->addAction(predictAction);
    
    toolBar->addSeparator();
    
    // 模型管理
    QAction* saveModelAction = new QAction(QIcon(":/icons/save_model.png"), tr("Save Model"), this);
    connect(saveModelAction, &QAction::triggered, this, &MLAnalysisWidget::saveModel);
    toolBar->addAction(saveModelAction);
    
    QAction* loadModelAction = new QAction(QIcon(":/icons/load_model.png"), tr("Load Model"), this);
    connect(loadModelAction, &QAction::triggered, this, &MLAnalysisWidget::loadModel);
    toolBar->addAction(loadModelAction);
}

void MLAnalysisWidget::setupConfigTabs()
{
    // 数据预处理选项卡
    setupDataPreprocessingTab();
    
    // 模型配置选项卡
    setupModelConfigTab();
    
    // 训练参数选项卡
    setupTrainingParamsTab();
}

void MLAnalysisWidget::setupDataPreprocessingTab()
{
    QWidget* preprocessingTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(preprocessingTab);
    
    // 数据源选择
    QGroupBox* dataSourceGroup = new QGroupBox(tr("Data Source"), this);
    QFormLayout* dataSourceLayout = new QFormLayout(dataSourceGroup);
    
    m_dataPathEdit = new QLineEdit(this);
    m_dataPathEdit->setPlaceholderText(tr("Path to dataset file"));
    QPushButton* browseDataButton = new QPushButton(tr("Browse..."), this);
    connect(browseDataButton, &QPushButton::clicked, this, &MLAnalysisWidget::browseDataFile);
    
    QHBoxLayout* dataPathLayout = new QHBoxLayout();
    dataPathLayout->addWidget(m_dataPathEdit);
    dataPathLayout->addWidget(browseDataButton);
    
    dataSourceLayout->addRow(tr("Dataset:"), dataPathLayout);
    
    m_dataFormatCombo = new QComboBox(this);
    m_dataFormatCombo->addItem(tr("CSV"), "csv");
    m_dataFormatCombo->addItem(tr("JSON"), "json");
    m_dataFormatCombo->addItem(tr("Excel"), "excel");
    dataSourceLayout->addRow(tr("Format:"), m_dataFormatCombo);
    
    layout->addWidget(dataSourceGroup);
    
    // 特征选择
    QGroupBox* featureSelectionGroup = new QGroupBox(tr("Feature Selection"), this);
    QVBoxLayout* featureSelectionLayout = new QVBoxLayout(featureSelectionGroup);
    
    m_featureTable = new QTableWidget(this);
    m_featureTable->setColumnCount(3);
    m_featureTable->setHorizontalHeaderLabels({tr("Feature"), tr("Type"), tr("Selected")});
    m_featureTable->horizontalHeader()->setStretchLastSection(true);
    featureSelectionLayout->addWidget(m_featureTable);
    
    layout->addWidget(featureSelectionGroup);
    
    // 预处理选项
    QGroupBox* preprocessingOptionsGroup = new QGroupBox(tr("Preprocessing Options"), this);
    QVBoxLayout* preprocessingOptionsLayout = new QVBoxLayout(preprocessingOptionsGroup);
    
    m_normalizeCheck = new QCheckBox(tr("Normalize Features"), this);
    m_normalizeCheck->setChecked(true);
    preprocessingOptionsLayout->addWidget(m_normalizeCheck);
    
    m_standardizeCheck = new QCheckBox(tr("Standardize Features"), this);
    preprocessingOptionsLayout->addWidget(m_standardizeCheck);
    
    m_handleMissingCheck = new QCheckBox(tr("Handle Missing Values"), this);
    m_handleMissingCheck->setChecked(true);
    preprocessingOptionsLayout->addWidget(m_handleMissingCheck);
    
    QHBoxLayout* missingStrategyLayout = new QHBoxLayout();
    QLabel* missingStrategyLabel = new QLabel(tr("Missing Values Strategy:"), this);
    m_missingStrategyCombo = new QComboBox(this);
    m_missingStrategyCombo->addItem(tr("Mean"), "mean");
    m_missingStrategyCombo->addItem(tr("Median"), "median");
    m_missingStrategyCombo->addItem(tr("Mode"), "mode");
    m_missingStrategyCombo->addItem(tr("Forward Fill"), "ffill");
    m_missingStrategyCombo->addItem(tr("Backward Fill"), "bfill");
    m_missingStrategyCombo->setEnabled(false);
    
    missingStrategyLayout->addWidget(missingStrategyLabel);
    missingStrategyLayout->addWidget(m_missingStrategyCombo);
    missingStrategyLayout->addStretch();
    
    preprocessingOptionsLayout->addLayout(missingStrategyLayout);
    
    connect(m_handleMissingCheck, &QCheckBox::toggled, m_missingStrategyCombo, &QComboBox::setEnabled);
    
    layout->addWidget(preprocessingOptionsGroup);
    
    // 特征工程
    QGroupBox* featureEngineeringGroup = new QGroupBox(tr("Feature Engineering"), this);
    QVBoxLayout* featureEngineeringLayout = new QVBoxLayout(featureEngineeringGroup);
    
    m_polynomialFeaturesCheck = new QCheckBox(tr("Polynomial Features"), this);
    m_polynomialDegreeSpin = new QSpinBox(this);
    m_polynomialDegreeSpin->setRange(2, 5);
    m_polynomialDegreeSpin->setValue(2);
    m_polynomialDegreeSpin->setEnabled(false);
    
    QHBoxLayout* polynomialLayout = new QHBoxLayout();
    polynomialLayout->addWidget(m_polynomialFeaturesCheck);
    polynomialLayout->addWidget(new QLabel(tr("Degree:"), this));
    polynomialLayout->addWidget(m_polynomialDegreeSpin);
    polynomialLayout->addStretch();
    
    connect(m_polynomialFeaturesCheck, &QCheckBox::toggled, m_polynomialDegreeSpin, &QSpinBox::setEnabled);
    
    featureEngineeringLayout->addLayout(polynomialLayout);
    
    m_interactionFeaturesCheck = new QCheckBox(tr("Interaction Features"), this);
    featureEngineeringLayout->addWidget(m_interactionFeaturesCheck);
    
    layout->addWidget(featureEngineeringGroup);
    
    layout->addStretch();
    
    m_configTabWidget->addTab(preprocessingTab, tr("Data Preprocessing"));
}

void MLAnalysisWidget::setupModelConfigTab()
{
    QWidget* modelConfigTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(modelConfigTab);
    
    // 模型类型选择
    QGroupBox* modelTypeGroup = new QGroupBox(tr("Model Type"), this);
    QVBoxLayout* modelTypeLayout = new QVBoxLayout(modelTypeGroup);
    
    m_regressionRadio = new QRadioButton(tr("Regression"), this);
    m_classificationRadio = new QRadioButton(tr("Classification"), this);
    m_clusteringRadio = new QRadioButton(tr("Clustering"), this);
    m_dimensionalityReductionRadio = new QRadioButton(tr("Dimensionality Reduction"), this);
    
    m_regressionRadio->setChecked(true);
    
    modelTypeLayout->addWidget(m_regressionRadio);
    modelTypeLayout->addWidget(m_classificationRadio);
    modelTypeLayout->addWidget(m_clusteringRadio);
    modelTypeLayout->addWidget(m_dimensionalityReductionRadio);
    
    m_modelTypeGroup = new QButtonGroup(this);
    m_modelTypeGroup->addButton(m_regressionRadio, 0);
    m_modelTypeGroup->addButton(m_classificationRadio, 1);
    m_modelTypeGroup->addButton(m_clusteringRadio, 2);
    m_modelTypeGroup->addButton(m_dimensionalityReductionRadio, 3);
    
    layout->addWidget(modelTypeGroup);
    
    // 具体模型选择
    QGroupBox* modelSelectionGroup = new QGroupBox(tr("Model Selection"), this);
    QVBoxLayout* modelSelectionLayout = new QVBoxLayout(modelSelectionGroup);
    
    m_modelSelectionCombo = new QComboBox(this);
    modelSelectionLayout->addWidget(m_modelSelectionCombo);
    
    m_modelDescriptionText = new QTextEdit(this);
    m_modelDescriptionText->setReadOnly(true);
    m_modelDescriptionText->setMaximumHeight(100);
    modelSelectionLayout->addWidget(m_modelDescriptionText);
    
    layout->addWidget(modelSelectionGroup);
    
    // 模型参数
    QGroupBox* modelParamsGroup = new QGroupBox(tr("Model Parameters"), this);
    QVBoxLayout* modelParamsLayout = new QVBoxLayout(modelParamsGroup);
    
    m_modelParamsWidget = new QWidget(this);
    m_modelParamsLayout = new QFormLayout(m_modelParamsWidget);
    modelParamsLayout->addWidget(m_modelParamsWidget);
    
    layout->addWidget(modelParamsGroup);
    
    layout->addStretch();
    
    m_configTabWidget->addTab(modelConfigTab, tr("Model Configuration"));
    
    // 初始化模型列表
    updateModelList();
    
    // 连接模型类型变化信号
    connect(m_modelTypeGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &MLAnalysisWidget::updateModelList);
    
    // 连接模型选择变化信号
    connect(m_modelSelectionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MLAnalysisWidget::updateModelDescription);
    connect(m_modelSelectionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MLAnalysisWidget::updateModelParameters);
    
    // 初始化显示
    updateModelDescription();
    updateModelParameters();
}

void MLAnalysisWidget::setupTrainingParamsTab()
{
    QWidget* trainingParamsTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(trainingParamsTab);
    
    // 数据分割
    QGroupBox* dataSplitGroup = new QGroupBox(tr("Data Split"), this);
    QFormLayout* dataSplitLayout = new QFormLayout(dataSplitGroup);
    
    m_testSizeSpin = new QDoubleSpinBox(this);
    m_testSizeSpin->setRange(0.1, 0.5);
    m_testSizeSpin->setSingleStep(0.05);
    m_testSizeSpin->setValue(0.2);
    m_testSizeSpin->setSuffix(" (10%-50%)");
    dataSplitLayout->addRow(tr("Test Size:"), m_testSizeSpin);
    
    m_validationSizeSpin = new QDoubleSpinBox(this);
    m_validationSizeSpin->setRange(0.0, 0.3);
    m_validationSizeSpin->setSingleStep(0.05);
    m_validationSizeSpin->setValue(0.1);
    m_validationSizeSpin->setSuffix(" (0%-30%)");
    dataSplitLayout->addRow(tr("Validation Size:"), m_validationSizeSpin);
    
    m_randomSeedSpin = new QSpinBox(this);
    m_randomSeedSpin->setRange(0, 9999);
    m_randomSeedSpin->setValue(42);
    dataSplitLayout->addRow(tr("Random Seed:"), m_randomSeedSpin);
    
    m_stratifyCheck = new QCheckBox(tr("Stratified Split (for classification)"), this);
    m_stratifyCheck->setChecked(true);
    dataSplitLayout->addRow("", m_stratifyCheck);
    
    layout->addWidget(dataSplitGroup);
    
    // 训练参数
    QGroupBox* trainingParamsGroup = new QGroupBox(tr("Training Parameters"), this);
    QFormLayout* trainingParamsLayout = new QFormLayout(trainingParamsGroup);
    
    m_crossValidationSpin = new QSpinBox(this);
    m_crossValidationSpin->setRange(2, 10);
    m_crossValidationSpin->setValue(5);
    trainingParamsLayout->addRow(tr("Cross-Validation Folds:"), m_crossValidationSpin);
    
    m_earlyStoppingCheck = new QCheckBox(tr("Enable Early Stopping"), this);
    m_earlyStoppingCheck->setChecked(false);
    trainingParamsLayout->addRow("", m_earlyStoppingCheck);
    
    m_maxEpochsSpin = new QSpinBox(this);
    m_maxEpochsSpin->setRange(10, 1000);
    m_maxEpochsSpin->setValue(100);
    m_maxEpochsSpin->setEnabled(false);
    trainingParamsLayout->addRow(tr("Max Epochs:"), m_maxEpochsSpin);
    
    m_patienceSpin = new QSpinBox(this);
    m_patienceSpin->setRange(5, 50);
    m_patienceSpin->setValue(10);
    m_patienceSpin->setEnabled(false);
    trainingParamsLayout->addRow(tr("Patience:"), m_patienceSpin);
    
    connect(m_earlyStoppingCheck, &QCheckBox::toggled, m_maxEpochsSpin, &QSpinBox::setEnabled);
    connect(m_earlyStoppingCheck, &QCheckBox::toggled, m_patienceSpin, &QSpinBox::setEnabled);
    
    layout->addWidget(trainingParamsGroup);
    
    // 评估指标
    QGroupBox* metricsGroup = new QGroupBox(tr("Evaluation Metrics"), this);
    QVBoxLayout* metricsLayout = new QVBoxLayout(metricsGroup);
    
    // 回归指标
    QGroupBox* regressionMetricsGroup = new QGroupBox(tr("Regression Metrics"), this);
    QVBoxLayout* regressionMetricsLayout = new QVBoxLayout(regressionMetricsGroup);
    
    m_maeCheck = new QCheckBox(tr("Mean Absolute Error (MAE)"), this);
    m_maeCheck->setChecked(true);
    regressionMetricsLayout->addWidget(m_maeCheck);
    
    m_mseCheck = new QCheckBox(tr("Mean Squared Error (MSE)"), this);
    m_mseCheck->setChecked(true);
    regressionMetricsLayout->addWidget(m_mseCheck);
    
    m_rmseCheck = new QCheckBox(tr("Root Mean Squared Error (RMSE)"), this);
    m_rmseCheck->setChecked(true);
    regressionMetricsLayout->addWidget(m_rmseCheck);
    
    m_r2Check = new QCheckBox(tr("R-squared (R²)"), this);
    m_r2Check->setChecked(true);
    regressionMetricsLayout->addWidget(m_r2Check);
    
    metricsLayout->addWidget(regressionMetricsGroup);
    
    // 分类指标
    QGroupBox* classificationMetricsGroup = new QGroupBox(tr("Classification Metrics"), this);
    QVBoxLayout* classificationMetricsLayout = new QVBoxLayout(classificationMetricsGroup);
    
    m_accuracyCheck = new QCheckBox(tr("Accuracy"), this);
    m_accuracyCheck->setChecked(true);
    classificationMetricsLayout->addWidget(m_accuracyCheck);
    
    m_precisionCheck = new QCheckBox(tr("Precision"), this);
    m_precisionCheck->setChecked(true);
    classificationMetricsLayout->addWidget(m_precisionCheck);
    
    m_recallCheck = new QCheckBox(tr("Recall"), this);
    m_recallCheck->setChecked(true);
    classificationMetricsLayout->addWidget(m_recallCheck);
    
    m_f1Check = new QCheckBox(tr("F1-Score"), this);
    m_f1Check->setChecked(true);
    classificationMetricsLayout->addWidget(m_f1Check);
    
    m_aucCheck = new QCheckBox(tr("Area Under ROC Curve (AUC)"), this);
    m_aucCheck->setChecked(true);
    classificationMetricsLayout->addWidget(m_aucCheck);
    
    metricsLayout->addWidget(classificationMetricsGroup);
    
    // 聚类指标
    QGroupBox* clusteringMetricsGroup = new QGroupBox(tr("Clustering Metrics"), this);
    QVBoxLayout* clusteringMetricsLayout = new QVBoxLayout(clusteringMetricsGroup);
    
    m_silhouetteCheck = new QCheckBox(tr("Silhouette Score"), this);
    m_silhouetteCheck->setChecked(true);
    clusteringMetricsLayout->addWidget(m_silhouetteCheck);
    
    m_daviesBouldinCheck = new QCheckBox(tr("Davies-Bouldin Index"), this);
    m_daviesBouldinCheck->setChecked(true);
    clusteringMetricsLayout->addWidget(m_daviesBouldinCheck);
    
    m_calinskiHarabaszCheck = new QCheckBox(tr("Calinski-Harabasz Index"), this);
    m_calinskiHarabaszCheck->setChecked(true);
    clusteringMetricsLayout->addWidget(m_calinskiHarabaszCheck);
    
    metricsLayout->addWidget(clusteringMetricsGroup);
    
    layout->addWidget(metricsGroup);
    
    layout->addStretch();
    
    m_configTabWidget->addTab(trainingParamsTab, tr("Training Parameters"));
    
    // 连接模型类型变化信号
    connect(m_modelTypeGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &MLAnalysisWidget::updateMetricsVisibility);
    
    // 初始化指标显示
    updateMetricsVisibility();
}

void MLAnalysisWidget::setupResultsPanel()
{
    m_resultsWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(m_resultsWidget);
    
    // 结果选项卡
    m_resultsTabWidget = new QTabWidget(this);
    
    // 训练历史选项卡
    setupTrainingHistoryTab();
    
    // 模型评估选项卡
    setupModelEvaluationTab();
    
    // 预测结果选项卡
    setupPredictionResultsTab();
    
    layout->addWidget(m_resultsTabWidget);
}

void MLAnalysisWidget::setupTrainingHistoryTab()
{
    QWidget* trainingHistoryTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(trainingHistoryTab);
    
    // 训练进度
    QGroupBox* trainingProgressGroup = new QGroupBox(tr("Training Progress"), this);
    QVBoxLayout* trainingProgressLayout = new QVBoxLayout(trainingProgressGroup);
    
    m_trainingProgressBar = new QProgressBar(this);
    trainingProgressLayout->addWidget(m_trainingProgressBar);
    
    m_trainingStatusLabel = new QLabel(tr("Ready to train"), this);
    trainingProgressLayout->addWidget(m_trainingStatusLabel);
    
    layout->addWidget(trainingProgressGroup);
    
    // 训练日志
    QGroupBox* trainingLogGroup = new QGroupBox(tr("Training Log"), this);
    QVBoxLayout* trainingLogLayout = new QVBoxLayout(trainingLogGroup);
    
    m_trainingLogText = new QTextEdit(this);
    m_trainingLogText->setReadOnly(true);
    trainingLogLayout->addWidget(m_trainingLogText);
    
    layout->addWidget(trainingLogGroup);
    
    m_resultsTabWidget->addTab(trainingHistoryTab, tr("Training History"));
}

void MLAnalysisWidget::setupModelEvaluationTab()
{
    QWidget* modelEvaluationTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(modelEvaluationTab);
    
    // 评估结果表格
    QGroupBox* evaluationResultsGroup = new QGroupBox(tr("Evaluation Results"), this);
    QVBoxLayout* evaluationResultsLayout = new QVBoxLayout(evaluationResultsGroup);
    
    m_evaluationTable = new QTableWidget(this);
    m_evaluationTable->setColumnCount(2);
    m_evaluationTable->setHorizontalHeaderLabels({tr("Metric"), tr("Value")});
    m_evaluationTable->horizontalHeader()->setStretchLastSection(true);
    evaluationResultsLayout->addWidget(m_evaluationTable);
    
    layout->addWidget(evaluationResultsGroup);
    
    // 混淆矩阵（分类用）
    m_confusionMatrixGroup = new QGroupBox(tr("Confusion Matrix"), this);
    QVBoxLayout* confusionMatrixLayout = new QVBoxLayout(m_confusionMatrixGroup);
    
    m_confusionMatrixTable = new QTableWidget(this);
    confusionMatrixLayout->addWidget(m_confusionMatrixTable);
    
    layout->addWidget(m_confusionMatrixGroup);
    
    m_resultsTabWidget->addTab(modelEvaluationTab, tr("Model Evaluation"));
}

void MLAnalysisWidget::setupPredictionResultsTab()
{
    QWidget* predictionResultsTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(predictionResultsTab);
    
    // 预测输入
    QGroupBox* predictionInputGroup = new QGroupBox(tr("Prediction Input"), this);
    QVBoxLayout* predictionInputLayout = new QVBoxLayout(predictionInputGroup);
    
    m_predictionInputTable = new QTableWidget(this);
    predictionInputLayout->addWidget(m_predictionInputTable);
    
    QHBoxLayout* predictButtonLayout = new QHBoxLayout();
    predictButtonLayout->addStretch();
    
    m_predictButton = new QPushButton(tr("Predict"), this);
    connect(m_predictButton, &QPushButton::clicked, this, &MLAnalysisWidget::predict);
    predictButtonLayout->addWidget(m_predictButton);
    
    predictionInputLayout->addLayout(predictButtonLayout);
    
    layout->addWidget(predictionInputGroup);
    
    // 预测结果
    QGroupBox* predictionResultsGroup = new QGroupBox(tr("Prediction Results"), this);
    QVBoxLayout* predictionResultsLayout = new QVBoxLayout(predictionResultsGroup);
    
    m_predictionResultsTable = new QTableWidget(this);
    predictionResultsLayout->addWidget(m_predictionResultsTable);
    
    layout->addWidget(predictionResultsGroup);
    
    m_resultsTabWidget->addTab(predictionResultsTab, tr("Prediction Results"));
}

void MLAnalysisWidget::setupStatusBar()
{
    m_statusBar = new QStatusBar(this);
    
    m_statusLabel = new QLabel(tr("Ready"), this);
    m_statusBar->addWidget(m_statusLabel);
    
    m_modelInfoLabel = new QLabel(this);
    m_statusBar->addPermanentWidget(m_modelInfoLabel);
}

void MLAnalysisWidget::setupConnections()
{
    // 连接其他必要的信号和槽
}

void MLAnalysisWidget::loadData()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Load Dataset"),
        QString(),
        tr("Dataset Files (*.csv *.json *.xlsx);;CSV Files (*.csv);;JSON Files (*.json);;Excel Files (*.xlsx);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        m_dataPathEdit->setText(fileName);
        
        if (m_dataProcessor) {
            // 加载数据
            Core::ML::DataProcessor::LoadResult result = m_dataProcessor->loadData(fileName, m_dataFormatCombo->currentData().toString());
            
            if (result.success) {
                updateFeatureTable(result.features);
                m_statusLabel->setText(tr("Dataset loaded: %1 rows, %2 columns").arg(result.rowCount).arg(result.columnCount));
                
                // 记录日志
                m_trainingLogText->append(tr("Dataset loaded from: %1").arg(fileName));
                m_trainingLogText->append(tr("Data shape: (%1, %2)").arg(result.rowCount).arg(result.columnCount));
            } else {
                QMessageBox::critical(this, tr("Error"), tr("Failed to load dataset: %1").arg(result.errorMessage));
                m_statusLabel->setText(tr("Failed to load dataset"));
            }
        }
    }
}

void MLAnalysisWidget::saveData()
{
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save Dataset"),
        QString(),
        tr("Dataset Files (*.csv *.json *.xlsx);;CSV Files (*.csv);;JSON Files (*.json);;Excel Files (*.xlsx);;All Files (*)"));
    
    if (!fileName.isEmpty() && m_dataProcessor) {
        // 保存数据
        if (m_dataProcessor->saveData(fileName, m_dataFormatCombo->currentData().toString())) {
            m_statusLabel->setText(tr("Dataset saved: %1").arg(fileName));
            m_trainingLogText->append(tr("Dataset saved to: %1").arg(fileName));
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to save dataset"));
            m_statusLabel->setText(tr("Failed to save dataset"));
        }
    }
}

void MLAnalysisWidget::trainModel()
{
    if (!m_dataProcessor) {
        QMessageBox::critical(this, tr("Error"), tr("Data processor not initialized"));
        return;
    }
    
    if (!m_dataProcessor->hasData()) {
        QMessageBox::information(this, tr("Information"), tr("No data loaded"));
        return;
    }
    
    // 获取选定的模型
    QString modelType = m_modelSelectionCombo->currentData().toString();
    if (modelType.isEmpty()) {
        QMessageBox::information(this, tr("Information"), tr("No model selected"));
        return;
    }
    
    // 获取模型参数
    QMap<QString, QVariant> modelParams = getModelParameters();
    
    // 获取训练参数
    Core::ML::TrainingParams trainingParams;
    trainingParams.testSize = m_testSizeSpin->value();
    trainingParams.validationSize = m_validationSizeSpin->value();
    trainingParams.randomSeed = m_randomSeedSpin->value();
    trainingParams.cvFolds = m_crossValidationSpin->value();
    trainingParams.earlyStopping = m_earlyStoppingCheck->isChecked();
    trainingParams.maxEpochs = m_earlyStoppingCheck->isChecked() ? m_maxEpochsSpin->value() : 0;
    trainingParams.patience = m_earlyStoppingCheck->isChecked() ? m_patienceSpin->value() : 0;
    trainingParams.stratify = m_stratifyCheck->isChecked();
    
    // 设置进度条
    m_trainingProgressBar->setValue(0);
    m_trainingStatusLabel->setText(tr("Initializing model..."));
    
    // 开始训练
    connect(m_dataProcessor, &Core::ML::DataProcessor::trainingProgress,
            this, [this](int progress, const QString& status) {
                m_trainingProgressBar->setValue(progress);
                m_trainingStatusLabel->setText(status);
                m_trainingLogText->append(status);
            });
    
    // 获取选定的特征
    QStringList selectedFeatures;
    for (int row = 0; row < m_featureTable->rowCount(); ++row) {
        QCheckBox* checkBox = qobject_cast<QCheckBox*>(m_featureTable->cellWidget(row, 2));
        if (checkBox && checkBox->isChecked()) {
            selectedFeatures << m_featureTable->item(row, 0)->text();
        }
    }
    
    // 训练模型
    Core::ML::DataProcessor::TrainingResult result = m_dataProcessor->trainModel(
        modelType, modelParams, trainingParams, selectedFeatures);
    
    if (result.success) {
        m_currentModel = result.model;
        m_trainingProgressBar->setValue(100);
        m_trainingStatusLabel->setText(tr("Training completed successfully"));
        m_trainingLogText->append(tr("Training completed in %1 seconds").arg(result.trainingTime));
        m_trainingLogText->append(tr("Best parameters: %1").arg(result.bestParams.toString()));
        
        // 更新模型信息
        m_modelInfoLabel->setText(tr("Model: %1, Accuracy: %2").arg(modelType).arg(result.metrics.value("accuracy", 0).toDouble(), 0, 'f', 4));
        
        // 更新评估结果
        updateEvaluationResults(result.metrics);
        
        // 切换到评估结果选项卡
        m_resultsTabWidget->setCurrentIndex(1);
        
        // 启用预测按钮
        m_predictButton->setEnabled(true);
    } else {
        m_trainingStatusLabel->setText(tr("Training failed"));
        m_trainingLogText->append(tr("Training failed: %1").arg(result.errorMessage));
        QMessageBox::critical(this, tr("Error"), tr("Training failed: %1").arg(result.errorMessage));
    }
    
    // 断开连接
    disconnect(m_dataProcessor, nullptr, this, nullptr);
}

void MLAnalysisWidget::evaluateModel()
{
    if (!m_currentModel) {
        QMessageBox::information(this, tr("Information"), tr("No trained model available"));
        return;
    }
    
    if (!m_dataProcessor) {
        QMessageBox::critical(this, tr("Error"), tr("Data processor not initialized"));
        return;
    }
    
    // 评估模型
    Core::ML::DataProcessor::EvaluationResult result = m_dataProcessor->evaluateModel(m_currentModel);
    
    if (result.success) {
        updateEvaluationResults(result.metrics);
        m_statusLabel->setText(tr("Model evaluated successfully"));
        
        // 切换到评估结果选项卡
        m_resultsTabWidget->setCurrentIndex(1);
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Model evaluation failed: %1").arg(result.errorMessage));
        m_statusLabel->setText(tr("Model evaluation failed"));
    }
}

void MLAnalysisWidget::predict()
{
    if (!m_currentModel) {
        QMessageBox::information(this, tr("Information"), tr("No trained model available"));
        return;
    }
    
    if (!m_dataProcessor) {
        QMessageBox::critical(this, tr("Error"), tr("Data processor not initialized"));
        return;
    }
    
    // 检查预测输入表是否有数据
    if (m_predictionInputTable->rowCount() == 0) {
        QMessageBox::information(this, tr("Information"), tr("No prediction data available"));
        return;
    }
    
    // 获取预测数据
    QVariantList predictionData = getPredictionData();
    
    // 进行预测
    Core::ML::DataProcessor::PredictionResult result = m_dataProcessor->predict(m_currentModel, predictionData);
    
    if (result.success) {
        updatePredictionResults(result.predictions);
        m_statusLabel->setText(tr("Prediction completed successfully"));
        
        // 切换到预测结果选项卡
        m_resultsTabWidget->setCurrentIndex(2);
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Prediction failed: %1").arg(result.errorMessage));
        m_statusLabel->setText(tr("Prediction failed"));
    }
}

void MLAnalysisWidget::saveModel()
{
    if (!m_currentModel) {
        QMessageBox::information(this, tr("Information"), tr("No trained model available"));
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save Model"),
        QString(),
        tr("Model Files (*.model *.pkl);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        if (Core::ML::MLModelManager::saveModel(m_currentModel, fileName)) {
            m_statusLabel->setText(tr("Model saved: %1").arg(fileName));
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to save model"));
            m_statusLabel->setText(tr("Failed to save model"));
        }
    }
}

void MLAnalysisWidget::loadModel()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Load Model"),
        QString(),
        tr("Model Files (*.model *.pkl);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        m_currentModel = Core::ML::MLModelManager::loadModel(fileName);
        
        if (m_currentModel) {
            m_statusLabel->setText(tr("Model loaded: %1").arg(fileName));
            m_modelInfoLabel->setText(tr("Model: %1").arg(m_currentModel->getName()));
            m_predictButton->setEnabled(true);
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to load model"));
            m_statusLabel->setText(tr("Failed to load model"));
        }
    }
}

void MLAnalysisWidget::browseDataFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Select Dataset"),
        QString(),
        tr("Dataset Files (*.csv *.json *.xlsx);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        m_dataPathEdit->setText(fileName);
        loadData();
    }
}

void MLAnalysisWidget::updateModelList()
{
    // 清空当前列表
    m_modelSelectionCombo->clear();
    
    // 根据选择的模型类型更新模型列表
    switch (m_modelTypeGroup->checkedId()) {
        case 0: // 回归
            addRegressionModels();
            break;
        case 1: // 分类
            addClassificationModels();
            break;
        case 2: // 聚类
            addClusteringModels();
            break;
        case 3: // 降维
            addDimensionalityReductionModels();
            break;
    }
}

void MLAnalysisWidget::addRegressionModels()
{
    // 添加回归模型
    m_modelSelectionCombo->addItem(tr("Linear Regression"), "linear_regression");
    m_modelSelectionCombo->addItem(tr("Ridge Regression"), "ridge_regression");
    m_modelSelectionCombo->addItem(tr("Lasso Regression"), "lasso_regression");
    m_modelSelectionCombo->addItem(tr("Elastic Net"), "elastic_net");
    m_modelSelectionCombo->addItem(tr("Support Vector Regression"), "svr");
    m_modelSelectionCombo->addItem(tr("Decision Tree Regressor"), "decision_tree_regressor");
    m_modelSelectionCombo->addItem(tr("Random Forest Regressor"), "random_forest_regressor");
    m_modelSelectionCombo->addItem(tr("Gradient Boosting Regressor"), "gradient_boosting_regressor");
    m_modelSelectionCombo->addItem(tr("Neural Network Regressor"), "neural_network_regressor");
}

void MLAnalysisWidget::addClassificationModels()
{
    // 添加分类模型
    m_modelSelectionCombo->addItem(tr("Logistic Regression"), "logistic_regression");
    m_modelSelectionCombo->addItem(tr("Support Vector Machine"), "svm");
    m_modelSelectionCombo->addItem(tr("Decision Tree Classifier"), "decision_tree_classifier");
    m_modelSelectionCombo->addItem(tr("Random Forest Classifier"), "random_forest_classifier");
    m_modelSelectionCombo->addItem(tr("Gradient Boosting Classifier"), "gradient_boosting_classifier");
    m_modelSelectionCombo->addItem(tr("AdaBoost Classifier"), "adaboost_classifier");
    m_modelSelectionCombo->addItem(tr("K-Nearest Neighbors"), "knn");
    m_modelSelectionCombo->addItem(tr("Naive Bayes"), "naive_bayes");
    m_modelSelectionCombo->addItem(tr("Neural Network Classifier"), "neural_network_classifier");
}

void MLAnalysisWidget::addClusteringModels()
{
    // 添加聚类模型
    m_modelSelectionCombo->addItem(tr("K-Means"), "kmeans");
    m_modelSelectionCombo->addItem(tr("DBSCAN"), "dbscan");
    m_modelSelectionCombo->addItem(tr("Hierarchical Clustering"), "hierarchical_clustering");
    m_modelSelectionCombo->addItem(tr("Gaussian Mixture Model"), "gaussian_mixture");
    m_modelSelectionCombo->addItem(tr("Agglomerative Clustering"), "agglomerative_clustering");
    m_modelSelectionCombo->addItem(tr("Mean Shift"), "mean_shift");
    m_modelSelectionCombo->addItem(tr("Spectral Clustering"), "spectral_clustering");
    m_modelSelectionCombo->addItem(tr("Affinity Propagation"), "affinity_propagation");
}

void MLAnalysisWidget::addDimensionalityReductionModels()
{
    // 添加降维模型
    m_modelSelectionCombo->addItem(tr("Principal Component Analysis (PCA)"), "pca");
    m_modelSelectionCombo->addItem(tr("Linear Discriminant Analysis (LDA)"), "lda");
    m_modelSelectionCombo->addItem(tr("Independent Component Analysis (ICA)"), "ica");
    m_modelSelectionCombo->addItem(tr("t-SNE"), "tsne");
    m_modelSelectionCombo->addItem(tr("UMAP"), "umap");
    m_modelSelectionCombo->addItem(tr("Factor Analysis"), "factor_analysis");
    m_modelSelectionCombo->addItem(tr("Non-negative Matrix Factorization"), "nmf");
    m_modelSelectionCombo->addItem(tr("Truncated SVD"), "truncated_svd");
}

void MLAnalysisWidget::updateModelDescription()
{
    QString modelType = m_modelSelectionCombo->currentData().toString();
    QString description;
    
    if (modelType == "linear_regression") {
        description = tr("Linear Regression models the relationship between a dependent variable and one or more independent variables by fitting a linear equation to observed data.");
    } else if (modelType == "logistic_regression") {
        description = tr("Logistic Regression is a statistical model that models the probability of an event taking place by having the log-odds for the event be a linear combination of one or more independent variables.");
    } else if (modelType == "kmeans") {
        description = tr("K-Means is an unsupervised learning algorithm that groups similar data points together by minimizing the distance between data points and the centroid of their cluster.");
    } else if (modelType == "pca") {
        description = tr("Principal Component Analysis (PCA) is a technique used to emphasize variation and bring out strong patterns in a dataset by transforming the data to a new coordinate system.");
    } else {
        description = tr("A machine learning algorithm for data analysis and prediction.");
    }
    
    m_modelDescriptionText->setText(description);
}

void MLAnalysisWidget::updateModelParameters()
{
    // 清除当前参数控件
    while (m_modelParamsLayout->count() > 0) {
        QLayoutItem* item = m_modelParamsLayout->takeAt(0);
        delete item->widget();
        delete item;
    }
    
    QString modelType = m_modelSelectionCombo->currentData().toString();
    
    if (modelType == "linear_regression") {
        // 添加线性回归参数
        m_modelParamsLayout->addRow(tr("Fit Intercept:"), createCheckBoxParameter("fit_intercept", true));
    } else if (modelType == "logistic_regression") {
        // 添加逻辑回归参数
        m_modelParamsLayout->addRow(tr("C (Regularization Strength):"), createDoubleSpinBoxParameter("C", 1.0, 0.01, 100.0, 0.1));
        m_modelParamsLayout->addRow(tr("Max Iterations:"), createSpinBoxParameter("max_iter", 100, 10, 1000));
        m_modelParamsLayout->addRow(tr("Solver:"), createComboBoxParameter("solver", {"liblinear", "lbfgs", "newton-cg", "sag", "saga"}, "lbfgs"));
    } else if (modelType == "kmeans") {
        // 添加K-Means参数
        m_modelParamsLayout->addRow(tr("Number of Clusters:"), createSpinBoxParameter("n_clusters", 8, 2, 50));
        m_modelParamsLayout->addRow(tr("Init Method:"), createComboBoxParameter("init", {"k-means++", "random"}, "k-means++"));
        m_modelParamsLayout->addRow(tr("Max Iterations:"), createSpinBoxParameter("max_iter", 300, 10, 1000));
        m_modelParamsLayout->addRow(tr("Random State:"), createSpinBoxParameter("random_state", 42, 0, 9999));
    } else if (modelType == "pca") {
        // 添加PCA参数
        m_modelParamsLayout->addRow(tr("Number of Components:"), createSpinBoxParameter("n_components", 2, 1, 100));
        m_modelParamsLayout->addRow(tr("Whiten:"), createCheckBoxParameter("whiten", false));
        m_modelParamsLayout->addRow(tr("SVD Solver:"), createComboBoxParameter("svd_solver", {"auto", "full", "arpack", "randomized"}, "auto"));
    } else {
        // 默认参数
        m_modelParamsLayout->addRow(tr("Parameter 1:"), createDoubleSpinBoxParameter("param1", 1.0, 0.1, 10.0, 0.1));
        m_modelParamsLayout->addRow(tr("Parameter 2:"), createSpinBoxParameter("param2", 100, 1, 1000));
    }
}

QCheckBox* MLAnalysisWidget::createCheckBoxParameter(const QString& name, bool defaultValue)
{
    QCheckBox* checkBox = new QCheckBox(this);
    checkBox->setChecked(defaultValue);
    checkBox->setObjectName(name);
    return checkBox;
}

QDoubleSpinBox* MLAnalysisWidget::createDoubleSpinBoxParameter(const QString& name, double defaultValue, double min, double max, double step)
{
    QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
    spinBox->setRange(min, max);
    spinBox->setSingleStep(step);
    spinBox->setValue(defaultValue);
    spinBox->setObjectName(name);
    return spinBox;
}

QSpinBox* MLAnalysisWidget::createSpinBoxParameter(const QString& name, int defaultValue, int min, int max)
{
    QSpinBox* spinBox = new QSpinBox(this);
    spinBox->setRange(min, max);
    spinBox->setValue(defaultValue);
    spinBox->setObjectName(name);
    return spinBox;
}

QComboBox* MLAnalysisWidget::createComboBoxParameter(const QString& name, const QStringList& options, const QString& defaultValue)
{
    QComboBox* comboBox = new QComboBox(this);
    comboBox->addItems(options);
    comboBox->setCurrentText(defaultValue);
    comboBox->setObjectName(name);
    return comboBox;
}

QMap<QString, QVariant> MLAnalysisWidget::getModelParameters()
{
    QMap<QString, QVariant> params;
    
    // 遍历模型参数控件并获取值
    for (int i = 0; i < m_modelParamsLayout->count(); ++i) {
        QLayoutItem* item = m_modelParamsLayout->itemAt(i);
        if (item->widget()) {
            QWidget* widget = item->widget();
            
            if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget)) {
                params[checkBox->objectName()] = checkBox->isChecked();
            } else if (QDoubleSpinBox* doubleSpinBox = qobject_cast<QDoubleSpinBox*>(widget)) {
                params[doubleSpinBox->objectName()] = doubleSpinBox->value();
            } else if (QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget)) {
                params[spinBox->objectName()] = spinBox->value();
            } else if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                params[comboBox->objectName()] = comboBox->currentText();
            }
        }
    }
    
    return params;
}

void MLAnalysisWidget::updateFeatureTable(const QStringList& features)
{
    // 清空表格
    m_featureTable->setRowCount(0);
    
    // 添加特征行
    for (const QString& feature : features) {
        int row = m_featureTable->rowCount();
        m_featureTable->insertRow(row);
        
        // 特征名称
        QTableWidgetItem* nameItem = new QTableWidgetItem(feature);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        m_featureTable->setItem(row, 0, nameItem);
        
        // 特征类型（这里简化处理，实际应该从数据中推断）
        QTableWidgetItem* typeItem = new QTableWidgetItem("Numerical");
        typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
        m_featureTable->setItem(row, 1, typeItem);
        
        // 选择复选框
        QCheckBox* checkBox = new QCheckBox(this);
        checkBox->setChecked(true);
        m_featureTable->setCellWidget(row, 2, checkBox);
    }
}

void MLAnalysisWidget::updateEvaluationResults(const QMap<QString, QVariant>& metrics)
{
    // 清空表格
    m_evaluationTable->setRowCount(0);
    
    // 添加指标行
    for (auto it = metrics.constBegin(); it != metrics.constEnd(); ++it) {
        int row = m_evaluationTable->rowCount();
        m_evaluationTable->insertRow(row);
        
        // 指标名称
        QTableWidgetItem* nameItem = new QTableWidgetItem(it.key());
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        m_evaluationTable->setItem(row, 0, nameItem);
        
        // 指标值
        QTableWidgetItem* valueItem = new QTableWidgetItem(QString::number(it.value().toDouble(), 'f', 4));
        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
        m_evaluationTable->setItem(row, 1, valueItem);
    }
}

void MLAnalysisWidget::updatePredictionResults(const QVariantList& predictions)
{
    // 清空表格
    m_predictionResultsTable->setRowCount(0);
    
    // 添加预测结果行
    for (int i = 0; i < predictions.size(); ++i) {
        int row = m_predictionResultsTable->rowCount();
        m_predictionResultsTable->insertRow(row);
        
        // 序号
        QTableWidgetItem* indexItem = new QTableWidgetItem(QString::number(i + 1));
        indexItem->setFlags(indexItem->flags() & ~Qt::ItemIsEditable);
        m_predictionResultsTable->setItem(row, 0, indexItem);
        
        // 预测值
        QTableWidgetItem* valueItem = new QTableWidgetItem(predictions[i].toString());
        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
        m_predictionResultsTable->setItem(row, 1, valueItem);
    }
    
    // 设置列标题
    if (m_predictionResultsTable->columnCount() == 0) {
        m_predictionResultsTable->setColumnCount(2);
        m_predictionResultsTable->setHorizontalHeaderLabels({tr("Index"), tr("Prediction")});
        m_predictionResultsTable->horizontalHeader()->setStretchLastSection(true);
    }
}

QVariantList MLAnalysisWidget::getPredictionData()
{
    QVariantList data;
    
    // 获取预测输入表格中的数据
    int rowCount = m_predictionInputTable->rowCount();
    int colCount = m_predictionInputTable->columnCount();
    
    for (int row = 0; row < rowCount; ++row) {
        QVariantMap rowData;
        for (int col = 0; col < colCount; ++col) {
            QTableWidgetItem* item = m_predictionInputTable->item(row, col);
            if (item) {
                QString header = m_predictionInputTable->horizontalHeaderItem(col)->text();
                rowData[header] = item->text();
            }
        }
        data.append(rowData);
    }
    
    return data;
}

void MLAnalysisWidget::updateMetricsVisibility()
{
    // 根据选择的模型类型显示相应的评估指标
    int modelType = m_modelTypeGroup->checkedId();
    
    // 回归指标
    bool showRegression = (modelType == 0);  // 回归
    m_maeCheck->setVisible(showRegression);
    m_mseCheck->setVisible(showRegression);
    m_rmseCheck->setVisible(showRegression);
    m_r2Check->setVisible(showRegression);
    
    // 分类指标
    bool showClassification = (modelType == 1);  // 分类
    m_accuracyCheck->setVisible(showClassification);
    m_precisionCheck->setVisible(showClassification);
    m_recallCheck->setVisible(showClassification);
    m_f1Check->setVisible(showClassification);
    m_aucCheck->setVisible(showClassification);
    
    // 聚类指标
    bool showClustering = (modelType == 2);  // 聚类
    m_silhouetteCheck->setVisible(showClustering);
    m_daviesBouldinCheck->setVisible(showClustering);
    m_calinskiHarabaszCheck->setVisible(showClustering);
    
    // 隐藏混淆矩阵（仅用于分类）
    m_confusionMatrixGroup->setVisible(showClassification);
}

} // namespace UI