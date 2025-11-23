#include "DataManagementWidget.h"
#include <QShowEvent>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QApplication>

#include "../core/data/DataService.h"
#include "../core/data/DataRecord.h"
#include "../utils/Logger.h"
#include "../utils/ConfigManager.h"

namespace BondForge {
namespace UI {

DataManagementWidget::DataManagementWidget(std::shared_ptr<Core::Data::DataService> dataService, QWidget *parent)
    : QWidget(parent)
    , m_mainSplitter(nullptr)
    , m_leftSplitter(nullptr)
    , m_toolbarWidget(nullptr)
    , m_addButton(nullptr)
    , m_editButton(nullptr)
    , m_deleteButton(nullptr)
    , m_importButton(nullptr)
    , m_exportButton(nullptr)
    , m_refreshButton(nullptr)
    , m_filterWidget(nullptr)
    , m_searchEdit(nullptr)
    , m_categoryCombo(nullptr)
    , m_formatCombo(nullptr)
    , m_viewTabs(nullptr)
    , m_dataTableView(nullptr)
    , m_dataTreeView(nullptr)
    , m_dataDetailsWidget(nullptr)
    , m_idLabel(nullptr)
    , m_nameLabel(nullptr)
    , m_formatLabel(nullptr)
    , m_categoryLabel(nullptr)
    , m_sizeLabel(nullptr)
    , m_createdLabel(nullptr)
    , m_modifiedLabel(nullptr)
    , m_contentEdit(nullptr)
    , m_metadataLabel(nullptr)
    , m_statusLabel(nullptr)
    , m_progressBar(nullptr)
    , m_dataService(dataService)
    , m_isDataLoaded(false)
    , m_selectedRecordId("")
{
    setupUI();
    connectSignals();
    
    // 初始化加载状态
    updateStatusMessage(tr("Ready to load data"));
    enableDataActions(false);
    
    Utils::Logger::info("DataManagementWidget initialized");
}

DataManagementWidget::~DataManagementWidget()
{
    // 清理资源
}

void DataManagementWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    
    // 每次显示时刷新数据
    if (!m_isDataLoaded || event->spontaneous()) {
        loadData();
    }
}

void DataManagementWidget::setupUI()
{
    // 创建主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // 创建工具栏
    setupToolbar();
    mainLayout->addWidget(m_toolbarWidget);
    
    // 创建过滤器栏
    setupFilterBar();
    mainLayout->addWidget(m_filterWidget);
    
    // 创建主分割器
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 创建左侧分割器
    m_leftSplitter = new QSplitter(Qt::Vertical, this);
    
    // 设置数据视图标签页
    m_viewTabs = new QTabWidget(this);
    
    // 设置数据表格视图
    setupDataTableView();
    m_viewTabs->addTab(m_dataTableView, tr("Table View"));
    
    // 设置数据树视图
    setupDataTreeView();
    m_viewTabs->addTab(m_dataTreeView, tr("Tree View"));
    
    m_leftSplitter->addWidget(m_viewTabs);
    
    // 设置数据详细视图
    setupDataDetailsView();
    m_leftSplitter->addWidget(m_dataDetailsWidget);
    
    // 设置分割比例
    m_leftSplitter->setStretchFactor(0, 3);
    m_leftSplitter->setStretchFactor(1, 1);
    
    m_mainSplitter->addWidget(m_leftSplitter);
    
    // 设置主布局
    mainLayout->addWidget(m_mainSplitter);
    
    // 创建状态栏
    QHBoxLayout* statusLayout = new QHBoxLayout();
    
    m_statusLabel = new QLabel(tr("Ready"));
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(150);
    
    statusLayout->addWidget(m_statusLabel, 1);
    statusLayout->addWidget(m_progressBar);
    
    mainLayout->addLayout(statusLayout);
}

void DataManagementWidget::setupDataTableView()
{
    // 创建数据表格视图
    m_dataTableView = new QTableView(this);
    m_dataTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_dataTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_dataTableView->setSortingEnabled(true);
    m_dataTableView->setAlternatingRowColors(true);
    m_dataTableView->horizontalHeader()->setStretchLastSection(true);
    m_dataTableView->verticalHeader()->setVisible(false);
    
    // 创建数据模型
    m_dataModel = std::make_unique<QStandardItemModel>(0, 7, this);
    m_dataModel->setHorizontalHeaderLabels({
        tr("ID"), tr("Name"), tr("Format"), tr("Category"), 
        tr("Size"), tr("Created"), tr("Modified")
    });
    
    // 创建代理模型用于过滤和排序
    m_proxyModel = std::make_unique<QSortFilterProxyModel>(this);
    m_proxyModel->setSourceModel(m_dataModel.get());
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(-1); // 搜索所有列
    
    m_dataTableView->setModel(m_proxyModel.get());
    
    // 设置列宽
    m_dataTableView->setColumnWidth(0, 100); // ID
    m_dataTableView->setColumnWidth(1, 200); // Name
    m_dataTableView->setColumnWidth(2, 100); // Format
    m_dataTableView->setColumnWidth(3, 120); // Category
    m_dataTableView->setColumnWidth(4, 80);  // Size
    m_dataTableView->setColumnWidth(5, 150); // Created
}

void DataManagementWidget::setupDataTreeView()
{
    // 创建数据树视图
    m_dataTreeView = new QTreeView(this);
    m_dataTreeView->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_dataTreeView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_dataTreeView->setAlternatingRowColors(true);
    m_dataTreeView->header()->setStretchLastSection(true);
    
    // TODO: 实现树状模型的实现
    // 这里可以使用QStandardItemModel实现分层数据显示
    QStandardItemModel* treeModel = new QStandardItemModel(this);
    treeModel->setHorizontalHeaderLabels({tr("Name"), tr("Type"), tr("Count")});
    
    // 添加分类节点
    QStandardItem* root = treeModel->invisibleRootItem();
    
    // 按类别分组
    QStandardItem* categoriesItem = new QStandardItem(tr("By Category"));
    root->appendRow(categoriesItem);
    
    // 按格式分组
    QStandardItem* formatsItem = new QStandardItem(tr("By Format"));
    root->appendRow(formatsItem);
    
    // 按时间分组
    QStandardItem* timeItem = new QStandardItem(tr("By Time"));
    root->appendRow(timeItem);
    
    m_dataTreeView->setModel(treeModel);
}

void DataManagementWidget::setupDataDetailsView()
{
    // 创建数据详细视图
    m_dataDetailsWidget = new QWidget(this);
    
    QVBoxLayout* detailsLayout = new QVBoxLayout(m_dataDetailsWidget);
    
    // 添加一个组框显示详细信息
    QGroupBox* detailsGroup = new QGroupBox(tr("Record Details"), this);
    QGridLayout* detailsGrid = new QGridLayout(detailsGroup);
    
    // 创建标签
    int row = 0;
    
    m_idLabel = new QLabel(tr("ID:"), this);
    detailsGrid->addWidget(m_idLabel, row, 0, Qt::AlignRight);
    
    m_nameLabel = new QLabel(tr("Name:"), this);
    detailsGrid->addWidget(m_nameLabel, ++row, 0, Qt::AlignRight);
    
    m_formatLabel = new QLabel(tr("Format:"), this);
    detailsGrid->addWidget(m_formatLabel, ++row, 0, Qt::AlignRight);
    
    m_categoryLabel = new QLabel(tr("Category:"), this);
    detailsGrid->addWidget(m_categoryLabel, ++row, 0, Qt::AlignRight);
    
    m_sizeLabel = new QLabel(tr("Size:"), this);
    detailsGrid->addWidget(m_sizeLabel, ++row, 0, Qt::AlignRight);
    
    m_createdLabel = new QLabel(tr("Created:"), this);
    detailsGrid->addWidget(m_createdLabel, ++row, 0, Qt::AlignRight);
    
    m_modifiedLabel = new QLabel(tr("Modified:"), this);
    detailsGrid->addWidget(m_modifiedLabel, ++row, 0, Qt::AlignRight);
    
    // 创建实际内容标签
    m_idLabel = new QLabel("-", this);
    detailsGrid->addWidget(m_idLabel, row - 6, 1);
    
    m_nameLabel = new QLabel("-", this);
    detailsGrid->addWidget(m_nameLabel, row - 5, 1);
    
    m_formatLabel = new QLabel("-", this);
    detailsGrid->addWidget(m_formatLabel, row - 4, 1);
    
    m_categoryLabel = new QLabel("-", this);
    detailsGrid->addWidget(m_categoryLabel, row - 3, 1);
    
    m_sizeLabel = new QLabel("-", this);
    detailsGrid->addWidget(m_sizeLabel, row - 2, 1);
    
    m_createdLabel = new QLabel("-", this);
    detailsGrid->addWidget(m_createdLabel, row - 1, 1);
    
    m_modifiedLabel = new QLabel("-", this);
    detailsGrid->addWidget(m_modifiedLabel, row, 1);
    
    detailsLayout->addWidget(detailsGroup);
    
    // 添加内容文本框
    QGroupBox* contentGroup = new QGroupBox(tr("Content"), this);
    QVBoxLayout* contentLayout = new QVBoxLayout(contentGroup);
    
    m_contentEdit = new QTextEdit(this);
    m_contentEdit->setReadOnly(true);
    m_contentEdit->setMaximumHeight(200);
    
    contentLayout->addWidget(m_contentEdit);
    detailsLayout->addWidget(contentGroup);
    
    // 添加元数据标签
    QGroupBox* metadataGroup = new QGroupBox(tr("Metadata"), this);
    QVBoxLayout* metadataLayout = new QVBoxLayout(metadataGroup);
    
    m_metadataLabel = new QLabel(tr("No metadata available"), this);
    m_metadataLabel->setWordWrap(true);
    
    metadataLayout->addWidget(m_metadataLabel);
    detailsLayout->addWidget(metadataGroup);
    
    detailsLayout->addStretch();
}

void DataManagementWidget::setupToolbar()
{
    // 创建工具栏
    m_toolbarWidget = new QWidget(this);
    QHBoxLayout* toolbarLayout = new QHBoxLayout(m_toolbarWidget);
    
    // 创建按钮
    m_addButton = new QPushButton(tr("Add"), this);
    m_addButton->setIcon(QIcon(":/icons/add.png"));
    m_addButton->setToolTip(tr("Add new data record"));
    
    m_editButton = new QPushButton(tr("Edit"), this);
    m_editButton->setIcon(QIcon(":/icons/edit.png"));
    m_editButton->setToolTip(tr("Edit selected data record"));
    m_editButton->setEnabled(false);
    
    m_deleteButton = new QPushButton(tr("Delete"), this);
    m_deleteButton->setIcon(QIcon(":/icons/delete.png"));
    m_deleteButton->setToolTip(tr("Delete selected data record"));
    m_deleteButton->setEnabled(false);
    
    m_importButton = new QPushButton(tr("Import"), this);
    m_importButton->setIcon(QIcon(":/icons/import.png"));
    m_importButton->setToolTip(tr("Import data from file"));
    
    m_exportButton = new QPushButton(tr("Export"), this);
    m_exportButton->setIcon(QIcon(":/icons/export.png"));
    m_exportButton->setToolTip(tr("Export selected data record"));
    m_exportButton->setEnabled(false);
    
    m_refreshButton = new QPushButton(tr("Refresh"), this);
    m_refreshButton->setIcon(QIcon(":/icons/refresh.png"));
    m_refreshButton->setToolTip(tr("Refresh data list"));
    
    // 添加到工具栏
    toolbarLayout->addWidget(m_addButton);
    toolbarLayout->addWidget(m_editButton);
    toolbarLayout->addWidget(m_deleteButton);
    toolbarLayout->addSeparator();
    toolbarLayout->addWidget(m_importButton);
    toolbarLayout->addWidget(m_exportButton);
    toolbarLayout->addSeparator();
    toolbarLayout->addWidget(m_refreshButton);
    toolbarLayout->addStretch();
}

void DataManagementWidget::setupFilterBar()
{
    // 创建过滤器栏
    m_filterWidget = new QWidget(this);
    QHBoxLayout* filterLayout = new QHBoxLayout(m_filterWidget);
    
    // 搜索框
    QLabel* searchLabel = new QLabel(tr("Search:"), this);
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Search in all fields..."));
    
    // 类别过滤器
    QLabel* categoryLabel = new QLabel(tr("Category:"), this);
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->addItem(tr("All"), "");
    
    // 格式过滤器
    QLabel* formatLabel = new QLabel(tr("Format:"), this);
    m_formatCombo = new QComboBox(this);
    m_formatCombo->addItem(tr("All"), "");
    
    // 添加到过滤器栏
    filterLayout->addWidget(searchLabel);
    filterLayout->addWidget(m_searchEdit, 1);
    filterLayout->addWidget(categoryLabel);
    filterLayout->addWidget(m_categoryCombo);
    filterLayout->addWidget(formatLabel);
    filterLayout->addWidget(m_formatCombo);
}

void DataManagementWidget::connectSignals()
{
    // 工具栏按钮信号
    connect(m_addButton, &QPushButton::clicked, this, &DataManagementWidget::addNewData);
    connect(m_editButton, &QPushButton::clicked, this, &DataManagementWidget::editData);
    connect(m_deleteButton, &QPushButton::clicked, this, &DataManagementWidget::deleteData);
    connect(m_importButton, &QPushButton::clicked, this, &DataManagementWidget::importData);
    connect(m_exportButton, &QPushButton::clicked, this, &DataManagementWidget::exportData);
    connect(m_refreshButton, &QPushButton::clicked, this, &DataManagementWidget::refreshData);
    
    // 过滤器信号
    connect(m_searchEdit, &QLineEdit::textChanged, this, &DataManagementWidget::onSearchTextChanged);
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
                onFilterCategoryChanged(m_categoryCombo->itemData(index).toString());
            });
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
                onFilterFormatChanged(m_formatCombo->itemData(index).toString());
            });
    
    // 表格选择信号
    QItemSelectionModel* selectionModel = m_dataTableView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
            this, &DataManagementWidget::onDataSelectionChanged);
    connect(m_dataTableView, &QTableView::doubleClicked,
            this, &DataManagementWidget::onDataDoubleClicked);
    
    // 标签页变化信号
    connect(m_viewTabs, &QTabWidget::currentChanged, this, &DataManagementWidget::onTabChanged);
    
    // 数据服务信号
    if (m_dataService) {
        // 这里可以连接数据服务的信号
        // connect(m_dataService.get(), &Core::Data::DataService::dataLoaded,
        //         this, &DataManagementWidget::onDataLoaded);
        // connect(m_dataService.get(), &Core::Data::DataService::dataAdded,
        //         this, &DataManagementWidget::onDataAdded);
        // connect(m_dataService.get(), &Core::Data::DataService::dataUpdated,
        //         this, &DataManagementWidget::onDataUpdated);
        // connect(m_dataService.get(), &Core::Data::DataService::dataDeleted,
        //         this, &DataManagementWidget::onDataDeleted);
        // connect(m_dataService.get(), &Core::Data::DataService::errorOccurred,
        //         this, &DataManagementWidget::onDataError);
    }
}

// 槽函数实现
void DataManagementWidget::loadData()
{
    if (!m_dataService) {
        Utils::Logger::error("DataService is not available");
        updateStatusMessage(tr("Error: Data service is not available"));
        return;
    }
    
    updateStatusMessage(tr("Loading data..."));
    showProgress(true);
    setProgressValue(0);
    
    // 获取数据记录
    QList<Core::Data::DataRecord> records = m_dataService->getAllRecords();
    
    // 清除当前数据
    clearDataModel();
    
    // 加载数据到模型
    for (const auto& record : records) {
        QList<QStandardItem*> rowItems;
        
        // ID
        rowItems << new QStandardItem(QString::fromStdString(record.id));
        
        // Name
        rowItems << new QStandardItem(QString::fromStdString(record.name));
        
        // Format
        rowItems << new QStandardItem(QString::fromStdString(record.format));
        
        // Category
        rowItems << new QStandardItem(QString::fromStdString(record.category));
        
        // Size
        rowItems << new QStandardItem(QString::number(record.content.length()));
        
        // Created
        rowItems << new QStandardItem(QDateTime::fromTime_t(record.createdAt).toString("yyyy-MM-dd hh:mm"));
        
        // Modified
        rowItems << new QStandardItem(QDateTime::fromTime_t(record.modifiedAt).toString("yyyy-MM-dd hh:mm"));
        
        m_dataModel->appendRow(rowItems);
    }
    
    // 更新过滤器
    updateFilters();
    
    m_isDataLoaded = true;
    enableDataActions(true);
    updateStatusMessage(tr("Loaded %1 records").arg(records.count()));
    showProgress(false);
    
    Utils::Logger::info(QString("Loaded %1 data records").arg(records.count()).toStdString());
}

void DataManagementWidget::addNewData()
{
    // TODO: 实现添加新数据的功能
    // 这里可以打开一个对话框来输入新数据
    
    QMessageBox::information(this, tr("Add New Data"), 
                             tr("Add new data functionality will be implemented in a future version."));
}

void DataManagementWidget::editData()
{
    // TODO: 实现编辑数据的功能
    // 这里可以打开一个对话框来编辑选中的数据
    
    QMessageBox::information(this, tr("Edit Data"), 
                             tr("Edit data functionality will be implemented in a future version."));
}

void DataManagementWidget::deleteData()
{
    // TODO: 实现删除数据的功能
    // 这里可以显示一个确认对话框，然后删除选中的数据
    
    QMessageBox::information(this, tr("Delete Data"), 
                             tr("Delete data functionality will be implemented in a future version."));
}

void DataManagementWidget::importData()
{
    // 打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Import Data"),
        QDir::homePath(),
        tr("Data Files (*.csv *.json *.sdf *.mol);;All Files (*)")
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // TODO: 实现导入数据的功能
    // 这里可以解析选中的文件并将数据导入到数据库
    
    QMessageBox::information(this, tr("Import Data"), 
                             tr("Import data from %1 functionality will be implemented in a future version.").arg(fileName));
}

void DataManagementWidget::exportData()
{
    // TODO: 实现导出数据的功能
    // 这里可以打开一个对话框来选择导出选项
    
    QMessageBox::information(this, tr("Export Data"), 
                             tr("Export data functionality will be implemented in a future version."));
}

void DataManagementWidget::refreshData()
{
    loadData();
}

void DataManagementWidget::onSearchTextChanged(const QString &text)
{
    if (m_proxyModel) {
        m_proxyModel->setFilterFixedString(text);
    }
}

void DataManagementWidget::onFilterCategoryChanged(const QString &category)
{
    if (m_proxyModel) {
        // 设置类别过滤器
        if (category.isEmpty()) {
            m_proxyModel->setFilterKeyColumn(-1); // 搜索所有列
        } else {
            m_proxyModel->setFilterKeyColumn(2); // 搜索类别列
            m_proxyModel->setFilterFixedString(category);
        }
    }
}

void DataManagementWidget::onFilterFormatChanged(const QString &format)
{
    if (m_proxyModel) {
        // 设置格式过滤器
        if (format.isEmpty()) {
            m_proxyModel->setFilterKeyColumn(-1); // 搜索所有列
        } else {
            m_proxyModel->setFilterKeyColumn(3); // 搜索格式列
            m_proxyModel->setFilterFixedString(format);
        }
    }
}

void DataManagementWidget::onDataSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)
    
    // 获取选中的行
    QModelIndexList indexes = selected.indexes();
    
    if (indexes.isEmpty()) {
        m_selectedRecordId = "";
        enableDataActions(false);
        return;
    }
    
    // 只处理第一列（第一列的索引）
    QModelIndex sourceIndex = m_proxyModel->mapToSource(indexes.first());
    int row = sourceIndex.row();
    
    // 获取记录ID
    QStandardItem* idItem = m_dataModel->item(row, 0);
    if (idItem) {
        m_selectedRecordId = idItem->text().toStdString();
        enableDataActions(true);
        
        // 加载详细数据
        if (m_dataService) {
            Core::Data::DataRecord record;
            if (m_dataService->getRecord(m_selectedRecordId, record)) {
                updateDataDetails(record);
            }
        }
    }
}

void DataManagementWidget::onDataDoubleClicked(const QModelIndex &index)
{
    // 双击时切换到可视化视图
    QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    int row = sourceIndex.row();
    
    QStandardItem* idItem = m_dataModel->item(row, 0);
    if (idItem) {
        m_selectedRecordId = idItem->text().toStdString();
        
        // TODO: 发送信号切换到可视化视图
        // emit switchToVisualization(m_selectedRecordId);
        
        QMessageBox::information(this, tr("Switch to Visualization"), 
                                 tr("Switching to visualization for record %1").arg(idItem->text()));
    }
}

void DataManagementWidget::onTabChanged(int index)
{
    // TODO: 处理标签页变化
    // 根据不同的标签页显示不同的内容
    Utils::Logger::info(QString("Switched to tab index: %1").arg(index).toStdString());
}

// 数据服务信号槽
void DataManagementWidget::onDataLoaded(int count)
{
    updateStatusMessage(tr("Data loaded: %1 records").arg(count));
}

void DataManagementWidget::onDataAdded(const Core::Data::DataRecord &record)
{
    // 添加新记录到模型
    QList<QStandardItem*> rowItems;
    
    // ID
    rowItems << new QStandardItem(QString::fromStdString(record.id));
    
    // Name
    rowItems << new QStandardItem(QString::fromStdString(record.name));
    
    // Format
    rowItems << new QStandardItem(QString::fromStdString(record.format));
    
    // Category
    rowItems << new QStandardItem(QString::fromStdString(record.category));
    
    // Size
    rowItems << new QStandardItem(QString::number(record.content.length()));
    
    // Created
    rowItems << new QStandardItem(QDateTime::fromTime_t(record.createdAt).toString("yyyy-MM-dd hh:mm"));
    
    // Modified
    rowItems << new QStandardItem(QDateTime::fromTime_t(record.modifiedAt).toString("yyyy-MM-dd hh:mm"));
    
    m_dataModel->appendRow(rowItems);
}

void DataManagementWidget::onDataUpdated(const Core::Data::DataRecord &record)
{
    // 查找并更新记录
    for (int row = 0; row < m_dataModel->rowCount(); ++row) {
        QStandardItem* idItem = m_dataModel->item(row, 0);
        if (idItem && idItem->text().toStdString() == record.id) {
            // 更新行数据
            m_dataModel->item(row, 1)->setText(QString::fromStdString(record.name));
            m_dataModel->item(row, 2)->setText(QString::fromStdString(record.format));
            m_dataModel->item(row, 3)->setText(QString::fromStdString(record.category));
            m_dataModel->item(row, 4)->setText(QString::number(record.content.length()));
            m_dataModel->item(row, 6)->setText(QDateTime::fromTime_t(record.modifiedAt).toString("yyyy-MM-dd hh:mm"));
            
            // 如果是当前选中的记录，更新详细信息
            if (m_selectedRecordId == record.id) {
                updateDataDetails(record);
            }
            break;
        }
    }
}

void DataManagementWidget::onDataDeleted(const std::string &id)
{
    // 查找并删除记录
    for (int row = 0; row < m_dataModel->rowCount(); ++row) {
        QStandardItem* idItem = m_dataModel->item(row, 0);
        if (idItem && idItem->text().toStdString() == id) {
            m_dataModel->removeRow(row);
            
            // 如果是当前选中的记录，清空详细信息
            if (m_selectedRecordId == id) {
                m_selectedRecordId = "";
                clearDataModel();
                enableDataActions(false);
            }
            break;
        }
    }
}

void DataManagementWidget::onDataError(const QString &message)
{
    QMessageBox::critical(this, tr("Data Error"), message);
    updateStatusMessage(tr("Error: %1").arg(message));
}

// 辅助函数
void DataManagementWidget::clearDataModel()
{
    m_dataModel->removeRows(0, m_dataModel->rowCount());
    m_selectedRecordId = "";
    updateDataDetails(Core::Data::DataRecord()); // 清空详细信息
}

void DataManagementWidget::updateDataDetails(const Core::Data::DataRecord &record)
{
    if (record.id.empty()) {
        // 清空详细信息
        m_idLabel->setText("-");
        m_nameLabel->setText("-");
        m_formatLabel->setText("-");
        m_categoryLabel->setText("-");
        m_sizeLabel->setText("-");
        m_createdLabel->setText("-");
        m_modifiedLabel->setText("-");
        m_contentEdit->clear();
        m_metadataLabel->setText(tr("No metadata available"));
        return;
    }
    
    // 更新详细信息
    m_idLabel->setText(QString::fromStdString(record.id));
    m_nameLabel->setText(QString::fromStdString(record.name));
    m_formatLabel->setText(QString::fromStdString(record.format));
    m_categoryLabel->setText(QString::fromStdString(record.category));
    m_sizeLabel->setText(QString::number(record.content.length()));
    m_createdLabel->setText(QDateTime::fromTime_t(record.createdAt).toString("yyyy-MM-dd hh:mm:ss"));
    m_modifiedLabel->setText(QDateTime::fromTime_t(record.modifiedAt).toString("yyyy-MM-dd hh:mm:ss"));
    
    // 更新内容
    m_contentEdit->setPlainText(QString::fromStdString(record.content));
    
    // 更新元数据
    QString metadataText;
    for (const auto& pair : record.metadata) {
        metadataText += QString("%1: %2\n").arg(QString::fromStdString(pair.first))
                                           .arg(QString::fromStdString(pair.second));
    }
    
    if (metadataText.isEmpty()) {
        m_metadataLabel->setText(tr("No metadata available"));
    } else {
        m_metadataLabel->setText(metadataText);
    }
}

void DataManagementWidget::updateStatusMessage(const QString &message)
{
    m_statusLabel->setText(message);
}

void DataManagementWidget::showProgress(bool show)
{
    m_progressBar->setVisible(show);
}

void DataManagementWidget::setProgressValue(int value)
{
    m_progressBar->setValue(value);
}

void DataManagementWidget::enableDataActions(bool enabled)
{
    m_editButton->setEnabled(enabled && !m_selectedRecordId.empty());
    m_deleteButton->setEnabled(enabled && !m_selectedRecordId.empty());
    m_exportButton->setEnabled(enabled && !m_selectedRecordId.empty());
}

void DataManagementWidget::updateFilters()
{
    // TODO: 根据当前数据更新过滤器的选项
    // 可以在这里添加类别和格式的选项
}

} // namespace UI
} // namespace BondForge