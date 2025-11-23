#ifndef BONDFORGE_MLANALYSISWIDGET_H
#define BONDFORGE_MLANALYSISWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QTableView>
#include <QTableWidget>
#include <QTreeView>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTextEdit>
#include <QSplitter>
#include <QProgressBar>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QChart>
#include <QChartView>
#include <QFileDialog>
#include <QMessageBox>
#include <memory>

// 前向声明
namespace BondForge {
    namespace Core {
        namespace Data {
            class DataService;
        }
        namespace ML {
            class MLModels;
            class StatisticalAnalysis;
        }
    }
}

namespace BondForge {
namespace UI {

class MLAnalysisWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MLAnalysisWidget(std::shared_ptr<Core::Data::DataService> dataService, QWidget *parent = nullptr);
    ~MLAnalysisWidget();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    // 标签页变化
    void onTabChanged(int index);
    
    // 模型训练槽函数
    void onModelTypeChanged();
    void onDataSourceChanged();
    void onFeatureSelectionChanged();
    void onTrainingParametersChanged();
    void onTrainModelClicked();
    void onSaveModelClicked();
    void onLoadModelClicked();
    void onTestModelClicked();
    void onPredictClicked();
    void onExportResultsClicked();
    
    // 统计分析槽函数
    void onStatisticsDataSetChanged();
    void onStatisticsVariableChanged();
    void onStatisticsTypeChanged();
    void onCalculateStatisticsClicked();
    void onHypothesisTestClicked();
    void onCorrelationAnalysisClicked();
    void onExportStatisticsClicked();
    
    // 数据预处理槽函数
    void onPreprocessingDataSetChanged();
    void onPreprocessingOperationChanged();
    void onPreprocessingParametersChanged();
    void onApplyPreprocessingClicked();
    void onPreviewPreprocessingClicked();
    void onResetPreprocessingClicked();
    
    // 数据服务信号
    void onDataLoaded(int count);
    void onModelTrainingProgress(int progress);
    void onModelTrainingCompleted(bool success);
    void onPredictionCompleted(bool success);
    void onStatisticsCompleted(bool success);

private:
    void setupUI();
    void setupModelTrainingTab();
    void setupStatisticalAnalysisTab();
    void setupDataPreprocessingTab();
    void setupToolbar();
    
    void connectSignals();
    
    // 模型训练
    void loadAvailableModels();
    void loadAvailableDataSets();
    void loadAvailableFeatures();
    void loadFeaturePreview();
    void trainModel();
    void saveModel();
    void loadModel();
    void testModel();
    void predict();
    void exportResults();
    void updateModelResults(const QString &results);
    void updatePredictionResults(const QString &results);
    void clearModelResults();
    void clearPredictionResults();
    
    // 统计分析
    void loadStatisticalDataSets();
    void loadStatisticalVariables();
    void calculateStatistics();
    void performHypothesisTest();
    void performCorrelationAnalysis();
    void exportStatistics();
    void updateStatisticsResults(const QString &results);
    void clearStatisticsResults();
    
    // 数据预处理
    void loadPreprocessingDataSets();
    void applyPreprocessing();
    void previewPreprocessing();
    void resetPreprocessing();
    void updatePreprocessingPreview();
    void clearPreprocessingPreview();
    
    // 工具函数
    void updateStatusMessage(const QString &message);
    void showProgress(bool show);
    void setProgressValue(int value);
    
    // UI组件
    QSplitter* m_mainSplitter;
    
    // 工具栏
    QWidget* m_toolbarWidget;
    QPushButton* m_refreshButton;
    QPushButton* m_exportButton;
    
    // 主标签页
    QTabWidget* m_mainTabs;
    
    // 模型训练标签页
    QWidget* m_modelTrainingTab;
    QSplitter* m_trainingSplitter;
    QWidget* m_trainingControlPanel;
    QComboBox* m_modelTypeCombo;
    QComboBox* m_dataSourceCombo;
    QListWidget* m_featuresList;
    QWidget* m_featureSelectionWidget;
    QWidget* m_trainingParametersWidget;
    QDoubleSpinBox* m_learningRateSpin;
    QSpinBox* m_epochsSpin;
    QSpinBox* m_batchSizeSpin;
    QDoubleSpinBox* m_validationSplitSpin;
    QComboBox* m_optimizerCombo;
    QComboBox* m_lossFunctionCombo;
    QPushButton* m_trainModelButton;
    QPushButton* m_saveModelButton;
    QPushButton* m_loadModelButton;
    QPushButton* m_testModelButton;
    QPushButton* m_predictButton;
    QPushButton* m_exportResultsButton;
    QWidget* m_trainingResultsWidget;
    QTextEdit* m_trainingResultsEdit;
    QChartView* m_trainingChartView;
    QChart* m_trainingChart;
    
    // 统计分析标签页
    QWidget* m_statisticalAnalysisTab;
    QSplitter* m_statisticsSplitter;
    QWidget* m_statisticsControlPanel;
    QComboBox* m_statisticsDataSetCombo;
    QListWidget* m_statisticsVariablesList;
    QComboBox* m_statisticsTypeCombo;
    QComboBox* m_hypothesisTestCombo;
    QComboBox* m_correlationTypeCombo;
    QDoubleSpinBox* m_significanceLevelSpin;
    QPushButton* m_calculateStatisticsButton;
    QPushButton* m_hypothesisTestButton;
    QPushButton* m_correlationAnalysisButton;
    QPushButton* m_exportStatisticsButton;
    QWidget* m_statisticsResultsWidget;
    QTextEdit* m_statisticsResultsEdit;
    QChartView* m_statisticsChartView;
    QChart* m_statisticsChart;
    
    // 数据预处理标签页
    QWidget* m_dataPreprocessingTab;
    QSplitter* m_preprocessingSplitter;
    QWidget* m_preprocessingControlPanel;
    QComboBox* m_preprocessingDataSetCombo;
    QComboBox* m_preprocessingOperationCombo;
    QWidget* m_preprocessingParametersWidget;
    QDoubleSpinBox* m_missingValueThresholdSpin;
    QComboBox* m_missingValueHandlingCombo;
    QCheckBox* m_normalizeCheckBox;
    QCheckBox* m_standardizeCheckBox;
    QComboBox* m_outlierDetectionCombo;
    QPushButton* m_applyPreprocessingButton;
    QPushButton* m_previewPreprocessingButton;
    QPushButton* m_resetPreprocessingButton;
    QWidget* m_preprocessingPreviewWidget;
    QTableView* m_preprocessingPreviewTable;
    QTextEdit* m_preprocessingLogEdit;
    
    // 状态栏
    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;
    
    // 服务
    std::shared_ptr<Core::Data::DataService> m_dataService;
    std::shared_ptr<Core::ML::MLModels> m_mlModels;
    std::shared_ptr<Core::ML::StatisticalAnalysis> m_statisticalAnalysis;
    
    // 状态
    bool m_isDataLoaded;
    bool m_isModelTrained;
    bool m_isModelLoaded;
    bool m_isStatisticsCalculated;
    QString m_currentModelPath;
    QString m_selectedDataSet;
    QStringList m_selectedFeatures;
};

} // namespace UI
} // namespace BondForge

#endif // BONDFORGE_MLANALYSISWIDGET_H