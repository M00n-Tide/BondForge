#ifndef BONDFORGE_DATAMANAGEMENTWIDGET_H
#define BONDFORGE_DATAMANAGEMENTWIDGET_H

#include <QWidget>
#include <QSplitter>
#include <QTableView>
#include <QTreeView>
#include <QTabWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QProgressBar>
#include <QTextEdit>
#include <QMenuBar>
#include <memory>

// 前向声明
namespace BondForge {
    namespace Core {
        namespace Data {
            class DataService;
            class DataRecord;
        }
    }
}

namespace BondForge {
namespace UI {

class DataManagementWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataManagementWidget(std::shared_ptr<Core::Data::DataService> dataService, QWidget *parent = nullptr);
    ~DataManagementWidget();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    // 数据操作槽函数
    void loadData();
    void addNewData();
    void editData();
    void deleteData();
    void importData();
    void exportData();
    void refreshData();
    
    // 数据过滤和搜索
    void onSearchTextChanged(const QString &text);
    void onFilterCategoryChanged(const QString &category);
    void onFilterFormatChanged(const QString &format);
    
    // 选择变化
    void onDataSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onDataDoubleClicked(const QModelIndex &index);
    
    // 标签页变化
    void onTabChanged(int index);
    
    // 数据服务信号
    void onDataLoaded(int count);
    void onDataAdded(const Core::Data::DataRecord &record);
    void onDataUpdated(const Core::Data::DataRecord &record);
    void onDataDeleted(const std::string &id);
    void onDataError(const QString &message);

private:
    void setupUI();
    void setupDataTableView();
    void setupDataTreeView();
    void setupDataDetailsView();
    void setupToolbar();
    void setupFilterBar();
    
    void connectSignals();
    void loadDataIntoModel();
    void clearDataModel();
    void updateDataDetails(const Core::Data::DataRecord &record);
    void updateStatusMessage(const QString &message);
    void enableDataActions(bool enabled);
    
    // UI组件
    QSplitter* m_mainSplitter;
    QSplitter* m_leftSplitter;
    
    // 工具栏
    QWidget* m_toolbarWidget;
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_importButton;
    QPushButton* m_exportButton;
    QPushButton* m_refreshButton;
    
    // 过滤栏
    QWidget* m_filterWidget;
    QLineEdit* m_searchEdit;
    QComboBox* m_categoryCombo;
    QComboBox* m_formatCombo;
    
    // 数据视图
    QTabWidget* m_viewTabs;
    QTableView* m_dataTableView;
    QTreeView* m_dataTreeView;
    QWidget* m_dataDetailsWidget;
    
    // 详细视图组件
    QLabel* m_idLabel;
    QLabel* m_nameLabel;
    QLabel* m_formatLabel;
    QLabel* m_categoryLabel;
    QLabel* m_sizeLabel;
    QLabel* m_createdLabel;
    QLabel* m_modifiedLabel;
    QTextEdit* m_contentEdit;
    QLabel* m_metadataLabel;
    
    // 状态栏
    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;
    
    // 模型
    std::unique_ptr<QStandardItemModel> m_dataModel;
    std::unique_ptr<QSortFilterProxyModel> m_proxyModel;
    
    // 服务
    std::shared_ptr<Core::Data::DataService> m_dataService;
    
    // 状态
    bool m_isDataLoaded;
    std::string m_selectedRecordId;
};

} // namespace UI
} // namespace BondForge

#endif // BONDFORGE_DATAMANAGEMENTWIDGET_H