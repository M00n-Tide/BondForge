#ifndef BONDFORGE_VISUALIZATIONWIDGET_H
#define BONDFORGE_VISUALIZATIONWIDGET_H

#include <QWidget>
#include <QSplitter>
#include <QTabWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTableView>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMenuBar>
#include <QToolBar>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QColorDialog>
#include <QProgressBar>
#include <QTextEdit>
#include <QChart>
#include <QChartView>
#include <QBarSeries>
#include <QLineSeries>
#include <QPieSeries>
#include <QScatterSeries>
#include <QBarSet>
#include <QValueAxis>
#include <QCategoryAxis>
#include <QLegend>
#include <memory>

// QT_CHARTS_USE_NAMESPACE

// 前向声明
namespace BondForge {
    namespace Core {
        namespace Data {
            class DataService;
            class DataRecord;
        }
        namespace Chemistry {
            class MoleculeRenderer;
        }
    }
}

namespace BondForge {
namespace UI {

class VisualizationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VisualizationWidget(std::shared_ptr<Core::Data::DataService> dataService, QWidget *parent = nullptr);
    ~VisualizationWidget();

protected:
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // 标签页变化
    void onTabChanged(int index);
    
    // 分子结构可视化
    void onMoleculeSelected(const QString &recordId);
    void onMoleculeLoadClicked();
    void onViewModeChanged(const QString &mode);
    void onZoomChanged(int value);
    void onRotationChanged(int value);
    void onRendererChanged(const QString &renderer);
    void onStyleChanged(const QString &style);
    void onBackgroundColorClicked();
    void onExportMoleculeClicked();
    
    // 数据图表
    void onChartTypeChanged(const QString &type);
    void onChartDataSourceChanged(const QString &source);
    void onDataCategoryChanged(const QString &category);
    void onChartRefreshClicked();
    void onChartExportClicked();
    void onChartAnimationToggled(bool enabled);
    
    // 趋势分析
    void onTrendTimeRangeChanged(const QString &range);
    void onTrendTypeChanged(const QString &type);
    void onTrendRefreshClicked();
    void onTrendExportClicked();
    void onMovingAverageChanged(int days);
    
    // 数据对比
    void onComparisonSelectionChanged();
    void onComparisonAddClicked();
    void onComparisonRemoveClicked();
    void onComparisonClearClicked();
    void onComparisonRefreshClicked();
    void onComparisonExportClicked();
    
    // 数据服务信号
    void onDataLoaded(int count);
    void onMoleculeRendered(bool success);
    void onChartGenerated(bool success);

private:
    void setupUI();
    void setupMoleculeVisualizationTab();
    void setupDataChartsTab();
    void setupTrendAnalysisTab();
    void setupDataComparisonTab();
    void setupToolbar();
    void setupStatusBar();
    
    void connectSignals();
    
    // 分子结构可视化
    void loadMoleculeList();
    void renderMolecule(const QString &recordId);
    void updateMoleculeControls();
    void exportMolecule(const QString &filePath);
    
    // 数据图表
    void updateChartData();
    void generateBarChart();
    void generatePieChart();
    void generateLineChart();
    void generateScatterChart();
    void exportChart(const QString &filePath);
    
    // 趋势分析
    void updateTrendData();
    void generateTrendChart();
    void exportTrendChart(const QString &filePath);
    
    // 数据对比
    void updateComparisonData();
    void generateComparisonChart();
    void updateComparisonTable();
    void exportComparison(const QString &filePath);
    
    // 工具函数
    void updateStatusMessage(const QString &message);
    void showProgress(bool show);
    void setProgressValue(int value);
    void clearChart();
    
    // UI组件
    QSplitter* m_mainSplitter;
    QSplitter* m_rightSplitter;
    
    // 工具栏
    QWidget* m_toolbarWidget;
    QPushButton* m_refreshButton;
    QPushButton* m_exportButton;
    QPushButton* m_settingsButton;
    
    // 主标签页
    QTabWidget* m_mainTabs;
    
    // 分子结构可视化标签页
    QWidget* m_moleculeTab;
    QWidget* m_moleculeControlPanel;
    QComboBox* m_moleculeCombo;
    QPushButton* m_loadMoleculeButton;
    QComboBox* m_viewModeCombo;
    QSlider* m_zoomSlider;
    QSlider* m_rotationSlider;
    QComboBox* m_rendererCombo;
    QComboBox* m_styleCombo;
    QPushButton* m_backgroundColorButton;
    QPushButton* m_exportMoleculeButton;
    
    QGraphicsView* m_moleculeView;
    QGraphicsScene* m_moleculeScene;
    
    // 数据图表标签页
    QWidget* m_chartTab;
    QWidget* m_chartControlPanel;
    QComboBox* m_chartTypeCombo;
    QComboBox* m_chartDataSourceCombo;
    QComboBox* m_dataCategoryCombo;
    QPushButton* m_chartRefreshButton;
    QPushButton* m_chartExportButton;
    QCheckBox* m_animationCheckBox;
    
    QChartView* m_chartView;
    QChart* m_currentChart;
    
    // 趋势分析标签页
    QWidget* m_trendTab;
    QWidget* m_trendControlPanel;
    QComboBox* m_trendTimeRangeCombo;
    QComboBox* m_trendTypeCombo;
    QPushButton* m_trendRefreshButton;
    QPushButton* m_trendExportButton;
    QSpinBox* m_movingAverageSpin;
    
    QChartView* m_trendView;
    QChart* m_trendChart;
    
    // 数据对比标签页
    QWidget* m_comparisonTab;
    QWidget* m_comparisonControlPanel;
    QTableView* m_comparisonTable;
    QTableView* m_availableDataTable;
    QPushButton* m_addComparisonButton;
    QPushButton* m_removeComparisonButton;
    QPushButton* m_clearComparisonButton;
    QPushButton* m_comparisonRefreshButton;
    QPushButton* m_comparisonExportButton;
    
    QChartView* m_comparisonView;
    QChart* m_comparisonChart;
    
    // 状态栏
    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;
    
    // 服务
    std::shared_ptr<Core::Data::DataService> m_dataService;
    std::unique_ptr<Core::Chemistry::MoleculeRenderer> m_moleculeRenderer;
    
    // 状态
    bool m_isDataLoaded;
    QString m_selectedMoleculeId;
    QColor m_backgroundColor;
    
    // 对比数据
    QStringList m_comparisonRecordIds;
};

} // namespace UI
} // namespace BondForge

#endif // BONDFORGE_VISUALIZATIONWIDGET_H