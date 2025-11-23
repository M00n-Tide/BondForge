#include "CollaborationWidget.h"
#include "core/collaboration/DataSharing.h"
#include "core/collaboration/User.h"
#include "core/permissions/PermissionManager.h"
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
#include <QCheckBox>
#include <QDateTimeEdit>
#include <QLineEdit>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStandardPaths>
#include <QDir>

namespace UI {

CollaborationWidget::CollaborationWidget(QWidget *parent)
    : QWidget(parent)
    , m_dataSharing(nullptr)
    , m_permissionManager(nullptr)
    , m_currentUser(nullptr)
{
    setupUI();
    setupConnections();
    
    // 初始化核心组件
    m_dataSharing = new Core::Collaboration::DataSharing(this);
    m_permissionManager = new Core::Permissions::PermissionManager(this);
    
    // 创建当前用户
    m_currentUser = new Core::Collaboration::User("current_user", "Current User");
    
    Utils::Logger::info("CollaborationWidget initialized");
}

CollaborationWidget::~CollaborationWidget()
{
    delete m_currentUser;
}

void CollaborationWidget::setupUI()
{
    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 工具栏
    QToolBar* toolBar = new QToolBar(tr("Collaboration Tools"), this);
    setupToolBar(toolBar);
    mainLayout->addWidget(toolBar);
    
    // 分割器：左侧功能面板，右侧详细信息
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    
    // 左侧：功能选项卡
    m_functionTabWidget = new QTabWidget(this);
    setupFunctionTabs();
    splitter->addWidget(m_functionTabWidget);
    
    // 右侧：详细信息面板
    setupDetailsPanel();
    splitter->addWidget(m_detailsWidget);
    
    splitter->setSizes({400, 400});
    
    mainLayout->addWidget(splitter);
    
    // 状态栏
    setupStatusBar();
    mainLayout->addWidget(m_statusBar);
}

void CollaborationWidget::setupToolBar(QToolBar* toolBar)
{
    // 用户操作
    QAction* refreshAction = new QAction(QIcon(":/icons/refresh.png"), tr("Refresh"), this);
    connect(refreshAction, &QAction::triggered, this, &CollaborationWidget::refresh);
    toolBar->addAction(refreshAction);
    
    toolBar->addSeparator();
    
    // 通知
    m_notificationsAction = new QAction(QIcon(":/icons/notifications.png"), tr("Notifications"), this);
    connect(m_notificationsAction, &QAction::triggered, this, &CollaborationWidget::showNotifications);
    toolBar->addAction(m_notificationsAction);
    
    // 在线状态指示器
    m_onlineStatusLabel = new QLabel(this);
    m_onlineStatusLabel->setPixmap(QPixmap(":/icons/offline.png"));
    toolBar->addWidget(m_onlineStatusLabel);
    
    m_onlineStatusText = new QLabel(tr("Offline"), this);
    toolBar->addWidget(m_onlineStatusText);
    
    // 当前用户
    m_userLabel = new QLabel(tr("User: %1").arg(m_currentUser ? m_currentUser->getName() : tr("Unknown")), this);
    toolBar->addSeparator();
    toolBar->addWidget(m_userLabel);
}

void CollaborationWidget::setupFunctionTabs()
{
    // 数据共享选项卡
    setupDataSharingTab();
    
    // 用户管理选项卡
    setupUserManagementTab();
    
    // 活动历史选项卡
    setupActivityHistoryTab();
    
    // 评论选项卡
    setupCommentsTab();
}

void CollaborationWidget::setupDataSharingTab()
{
    QWidget* dataSharingTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(dataSharingTab);
    
    // 我的共享数据
    QGroupBox* mySharedDataGroup = new QGroupBox(tr("My Shared Data"), this);
    QVBoxLayout* mySharedDataLayout = new QVBoxLayout(mySharedDataGroup);
    
    m_sharedDataTable = new QTableWidget(this);
    m_sharedDataTable->setColumnCount(6);
    m_sharedDataTable->setHorizontalHeaderLabels({
        tr("Name"), tr("Type"), tr("Shared With"), tr("Permissions"), tr("Expiration"), tr("Actions")
    });
    m_sharedDataTable->horizontalHeader()->setStretchLastSection(true);
    m_sharedDataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mySharedDataLayout->addWidget(m_sharedDataTable);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* shareDataButton = new QPushButton(tr("Share Data"), this);
    connect(shareDataButton, &QPushButton::clicked, this, &CollaborationWidget::shareData);
    buttonLayout->addWidget(shareDataButton);
    
    QPushButton* editShareButton = new QPushButton(tr("Edit Sharing"), this);
    connect(editShareButton, &QPushButton::clicked, this, &CollaborationWidget::editDataSharing);
    buttonLayout->addWidget(editShareButton);
    
    QPushButton* revokeShareButton = new QPushButton(tr("Revoke Access"), this);
    connect(revokeShareButton, &QPushButton::clicked, this, &CollaborationWidget::revokeDataAccess);
    buttonLayout->addWidget(revokeShareButton);
    
    buttonLayout->addStretch();
    
    mySharedDataLayout->addLayout(buttonLayout);
    layout->addWidget(mySharedDataGroup);
    
    // 共享给我
    QGroupBox* sharedWithMeGroup = new QGroupBox(tr("Shared With Me"), this);
    QVBoxLayout* sharedWithMeLayout = new QVBoxLayout(sharedWithMeGroup);
    
    m_sharedWithMeTable = new QTableWidget(this);
    m_sharedWithMeTable->setColumnCount(5);
    m_sharedWithMeTable->setHorizontalHeaderLabels({
        tr("Name"), tr("Owner"), tr("Type"), tr("Shared Date"), tr("Actions")
    });
    m_sharedWithMeTable->horizontalHeader()->setStretchLastSection(true);
    m_sharedWithMeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    sharedWithMeLayout->addWidget(m_sharedWithMeTable);
    
    // 按钮区域
    QHBoxLayout* sharedWithMeButtonLayout = new QHBoxLayout();
    
    QPushButton* openSharedButton = new QPushButton(tr("Open"), this);
    connect(openSharedButton, &QPushButton::clicked, this, &CollaborationWidget::openSharedData);
    sharedWithMeButtonLayout->addWidget(openSharedButton);
    
    QPushButton* copySharedButton = new QPushButton(tr("Copy to My Space"), this);
    connect(copySharedButton, &QPushButton::clicked, this, &CollaborationWidget::copySharedData);
    sharedWithMeButtonLayout->addWidget(copySharedButton);
    
    sharedWithMeButtonLayout->addStretch();
    
    sharedWithMeLayout->addLayout(sharedWithMeButtonLayout);
    layout->addWidget(sharedWithMeGroup);
    
    m_functionTabWidget->addTab(dataSharingTab, tr("Data Sharing"));
    
    // 填充示例数据
    populateSharedDataTables();
}

void CollaborationWidget::setupUserManagementTab()
{
    QWidget* userManagementTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(userManagementTab);
    
    // 用户搜索
    QGroupBox* userSearchGroup = new QGroupBox(tr("User Search"), this);
    QHBoxLayout* userSearchLayout = new QHBoxLayout(userSearchGroup);
    
    m_userSearchEdit = new QLineEdit(this);
    m_userSearchEdit->setPlaceholderText(tr("Search by name, email, or ID"));
    userSearchLayout->addWidget(m_userSearchEdit);
    
    QPushButton* searchUserButton = new QPushButton(tr("Search"), this);
    connect(searchUserButton, &QPushButton::clicked, this, &CollaborationWidget::searchUsers);
    userSearchLayout->addWidget(searchUserButton);
    
    layout->addWidget(userSearchGroup);
    
    // 用户列表
    QGroupBox* userListGroup = new QGroupBox(tr("User List"), this);
    QVBoxLayout* userListLayout = new QVBoxLayout(userListGroup);
    
    m_userTable = new QTableWidget(this);
    m_userTable->setColumnCount(5);
    m_userTable->setHorizontalHeaderLabels({
        tr("Name"), tr("Email"), tr("Department"), tr("Role"), tr("Actions")
    });
    m_userTable->horizontalHeader()->setStretchLastSection(true);
    m_userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    userListLayout->addWidget(m_userTable);
    
    // 按钮区域
    QHBoxLayout* userListButtonLayout = new QHBoxLayout();
    
    QPushButton* viewUserProfileButton = new QPushButton(tr("View Profile"), this);
    connect(viewUserProfileButton, &QPushButton::clicked, this, &CollaborationWidget::viewUserProfile);
    userListButtonLayout->addWidget(viewUserProfileButton);
    
    QPushButton* shareWithUserButton = new QPushButton(tr("Share With User"), this);
    connect(shareWithUserButton, &QPushButton::clicked, this, &CollaborationWidget::shareWithUser);
    userListButtonLayout->addWidget(shareWithUserButton);
    
    userListButtonLayout->addStretch();
    
    userListLayout->addLayout(userListButtonLayout);
    layout->addWidget(userListGroup);
    
    // 团队管理
    QGroupBox* teamManagementGroup = new QGroupBox(tr("Team Management"), this);
    QVBoxLayout* teamManagementLayout = new QVBoxLayout(teamManagementGroup);
    
    m_teamTreeWidget = new QTreeWidget(this);
    m_teamTreeWidget->setHeaderLabels({
        tr("Team/Member"), tr("Role"), tr("Department"), tr("Last Active")
    });
    teamManagementLayout->addWidget(m_teamTreeWidget);
    
    // 按钮区域
    QHBoxLayout* teamManagementButtonLayout = new QHBoxLayout();
    
    QPushButton* createTeamButton = new QPushButton(tr("Create Team"), this);
    connect(createTeamButton, &QPushButton::clicked, this, &CollaborationWidget::createTeam);
    teamManagementButtonLayout->addWidget(createTeamButton);
    
    QPushButton* editTeamButton = new QPushButton(tr("Edit Team"), this);
    connect(editTeamButton, &QPushButton::clicked, this, &CollaborationWidget::editTeam);
    teamManagementButtonLayout->addWidget(editTeamButton);
    
    QPushButton* inviteToTeamButton = new QPushButton(tr("Invite to Team"), this);
    connect(inviteToTeamButton, &QPushButton::clicked, this, &CollaborationWidget::inviteToTeam);
    teamManagementButtonLayout->addWidget(inviteToTeamButton);
    
    teamManagementButtonLayout->addStretch();
    
    teamManagementLayout->addLayout(teamManagementButtonLayout);
    layout->addWidget(teamManagementGroup);
    
    m_functionTabWidget->addTab(userManagementTab, tr("User Management"));
    
    // 填充示例数据
    populateUserTables();
}

void CollaborationWidget::setupActivityHistoryTab()
{
    QWidget* activityHistoryTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(activityHistoryTab);
    
    // 筛选选项
    QGroupBox* filterGroup = new QGroupBox(tr("Filter Options"), this);
    QFormLayout* filterLayout = new QFormLayout(filterGroup);
    
    m_activityUserFilter = new QComboBox(this);
    m_activityUserFilter->addItem(tr("All Users"), "");
    filterLayout->addRow(tr("User:"), m_activityUserFilter);
    
    m_activityTypeFilter = new QComboBox(this);
    m_activityTypeFilter->addItem(tr("All Types"), "");
    m_activityTypeFilter->addItem(tr("Data Sharing"), "data_sharing");
    m_activityTypeFilter->addItem(tr("Data Access"), "data_access");
    m_activityTypeFilter->addItem(tr("Comment"), "comment");
    m_activityTypeFilter->addItem(tr("Team Management"), "team_management");
    filterLayout->addRow(tr("Activity Type:"), m_activityTypeFilter);
    
    m_activityDateFilter = new QDateTimeEdit(QDateTime::currentDateTime().addDays(-7), this);
    m_activityDateFilter->setCalendarPopup(true);
    filterLayout->addRow(tr("Since:"), m_activityDateFilter);
    
    QPushButton* applyFilterButton = new QPushButton(tr("Apply Filter"), this);
    connect(applyFilterButton, &QPushButton::clicked, this, &CollaborationWidget::applyActivityFilter);
    filterLayout->addRow("", applyFilterButton);
    
    layout->addWidget(filterGroup);
    
    // 活动历史
    QGroupBox* activityHistoryGroup = new QGroupBox(tr("Activity History"), this);
    QVBoxLayout* activityHistoryLayout = new QVBoxLayout(activityHistoryGroup);
    
    m_activityTable = new QTableWidget(this);
    m_activityTable->setColumnCount(6);
    m_activityTable->setHorizontalHeaderLabels({
        tr("Timestamp"), tr("User"), tr("Action"), tr("Target"), tr("Details"), tr("Status")
    });
    m_activityTable->horizontalHeader()->setStretchLastSection(true);
    m_activityTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    activityHistoryLayout->addWidget(m_activityTable);
    
    // 按钮区域
    QHBoxLayout* activityHistoryButtonLayout = new QHBoxLayout();
    
    QPushButton* exportActivityButton = new QPushButton(tr("Export"), this);
    connect(exportActivityButton, &QPushButton::clicked, this, &CollaborationWidget::exportActivityHistory);
    activityHistoryButtonLayout->addWidget(exportActivityButton);
    
    QPushButton* clearActivityButton = new QPushButton(tr("Clear History"), this);
    connect(clearActivityButton, &QPushButton::clicked, this, &CollaborationWidget::clearActivityHistory);
    activityHistoryButtonLayout->addWidget(clearActivityButton);
    
    activityHistoryButtonLayout->addStretch();
    
    activityHistoryLayout->addLayout(activityHistoryButtonLayout);
    layout->addWidget(activityHistoryGroup);
    
    m_functionTabWidget->addTab(activityHistoryTab, tr("Activity History"));
    
    // 填充示例数据
    populateActivityTable();
}

void CollaborationWidget::setupCommentsTab()
{
    QWidget* commentsTab = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(commentsTab);
    
    // 评论选择
    QGroupBox* commentSelectionGroup = new QGroupBox(tr("Select Item to Comment"), this);
    QFormLayout* commentSelectionLayout = new QFormLayout(commentSelectionGroup);
    
    m_commentItemTypeCombo = new QComboBox(this);
    m_commentItemTypeCombo->addItem(tr("Molecule"), "molecule");
    m_commentItemTypeCombo->addItem(tr("Dataset"), "dataset");
    m_commentItemTypeCombo->addItem(tr("Model"), "model");
    m_commentItemTypeCombo->addItem(tr("Project"), "project");
    commentSelectionLayout->addRow(tr("Item Type:"), m_commentItemTypeCombo);
    
    m_commentItemCombo = new QComboBox(this);
    commentSelectionLayout->addRow(tr("Item:"), m_commentItemCombo);
    
    layout->addWidget(commentSelectionGroup);
    
    // 评论列表
    QGroupBox* commentListGroup = new QGroupBox(tr("Comments"), this);
    QVBoxLayout* commentListLayout = new QVBoxLayout(commentListGroup);
    
    m_commentTable = new QTableWidget(this);
    m_commentTable->setColumnCount(4);
    m_commentTable->setHorizontalHeaderLabels({
        tr("Author"), tr("Timestamp"), tr("Comment"), tr("Actions")
    });
    m_commentTable->horizontalHeader()->setStretchLastSection(true);
    m_commentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_commentTable->setColumnWidth(0, 100);
    m_commentTable->setColumnWidth(1, 150);
    commentListLayout->addWidget(m_commentTable);
    
    // 按钮区域
    QHBoxLayout* commentListButtonLayout = new QHBoxLayout();
    
    QPushButton* editCommentButton = new QPushButton(tr("Edit"), this);
    connect(editCommentButton, &QPushButton::clicked, this, &CollaborationWidget::editComment);
    commentListButtonLayout->addWidget(editCommentButton);
    
    QPushButton* deleteCommentButton = new QPushButton(tr("Delete"), this);
    connect(deleteCommentButton, &QPushButton::clicked, this, &CollaborationWidget::deleteComment);
    commentListButtonLayout->addWidget(deleteCommentButton);
    
    commentListButtonLayout->addStretch();
    
    commentListLayout->addLayout(commentListButtonLayout);
    layout->addWidget(commentListGroup);
    
    // 添加评论
    QGroupBox* addCommentGroup = new QGroupBox(tr("Add Comment"), this);
    QVBoxLayout* addCommentLayout = new QVBoxLayout(addCommentGroup);
    
    m_newCommentEdit = new QTextEdit(this);
    m_newCommentEdit->setMaximumHeight(100);
    m_newCommentEdit->setPlaceholderText(tr("Enter your comment here..."));
    addCommentLayout->addWidget(m_newCommentEdit);
    
    QHBoxLayout* addCommentButtonLayout = new QHBoxLayout();
    
    QPushButton* postCommentButton = new QPushButton(tr("Post Comment"), this);
    connect(postCommentButton, &QPushButton::clicked, this, &CollaborationWidget::postComment);
    addCommentButtonLayout->addWidget(postCommentButton);
    
    addCommentButtonLayout->addStretch();
    
    addCommentLayout->addLayout(addCommentButtonLayout);
    layout->addWidget(addCommentGroup);
    
    m_functionTabWidget->addTab(commentsTab, tr("Comments"));
    
    // 填充示例数据
    populateCommentTable();
    
    // 连接项目类型变化信号
    connect(m_commentItemTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CollaborationWidget::updateCommentItems);
}

void CollaborationWidget::setupDetailsPanel()
{
    m_detailsWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(m_detailsWidget);
    
    // 详情标题
    m_detailsTitleLabel = new QLabel(tr("Select an item to view details"), this);
    m_detailsTitleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(m_detailsTitleLabel);
    
    // 详情内容
    m_detailsContentTextEdit = new QTextEdit(this);
    m_detailsContentTextEdit->setReadOnly(true);
    layout->addWidget(m_detailsContentTextEdit);
    
    // 操作按钮
    m_detailsButtonLayout = new QHBoxLayout();
    layout->addLayout(m_detailsButtonLayout);
}

void CollaborationWidget::setupStatusBar()
{
    m_statusBar = new QStatusBar(this);
    
    m_statusLabel = new QLabel(tr("Ready"), this);
    m_statusBar->addWidget(m_statusLabel);
    
    m_connectionStatusLabel = new QLabel(tr("Disconnected"), this);
    m_statusBar->addPermanentWidget(m_connectionStatusLabel);
}

void CollaborationWidget::setupConnections()
{
    // 连接表格选择信号
    connect(m_sharedDataTable, &QTableWidget::cellClicked,
            this, &CollaborationWidget::onSharedDataSelected);
    
    connect(m_sharedWithMeTable, &QTableWidget::cellClicked,
            this, &CollaborationWidget::onSharedWithMeSelected);
    
    connect(m_userTable, &QTableWidget::cellClicked,
            this, &CollaborationWidget::onUserSelected);
    
    connect(m_activityTable, &QTableWidget::cellClicked,
            this, &CollaborationWidget::onActivitySelected);
    
    connect(m_commentTable, &QTableWidget::cellClicked,
            this, &CollaborationWidget::onCommentSelected);
}

void CollaborationWidget::populateSharedDataTables()
{
    // 填充我的共享数据表格
    m_sharedDataTable->setRowCount(3);
    
    // 第1行
    m_sharedDataTable->setItem(0, 0, new QTableWidgetItem(tr("Protein Dataset v1.0")));
    m_sharedDataTable->setItem(0, 1, new QTableWidgetItem(tr("Dataset")));
    m_sharedDataTable->setItem(0, 2, new QTableWidgetItem(tr("Research Team")));
    m_sharedDataTable->setItem(0, 3, new QTableWidgetItem(tr("Read/Write")));
    m_sharedDataTable->setItem(0, 4, new QTableWidgetItem(tr("2024-12-31")));
    
    QPushButton* editShareButton1 = new QPushButton(tr("Edit"), this);
    connect(editShareButton1, &QPushButton::clicked, [this]() { editDataSharing(); });
    m_sharedDataTable->setCellWidget(0, 5, editShareButton1);
    
    // 第2行
    m_sharedDataTable->setItem(1, 0, new QTableWidgetItem(tr("Molecule Collection")));
    m_sharedDataTable->setItem(1, 1, new QTableWidgetItem(tr("Molecules")));
    m_sharedDataTable->setItem(1, 2, new QTableWidgetItem(tr("John Doe")));
    m_sharedDataTable->setItem(1, 3, new QTableWidgetItem(tr("Read Only")));
    m_sharedDataTable->setItem(1, 4, new QTableWidgetItem(tr("2024-11-30")));
    
    QPushButton* editShareButton2 = new QPushButton(tr("Edit"), this);
    connect(editShareButton2, &QPushButton::clicked, [this]() { editDataSharing(); });
    m_sharedDataTable->setCellWidget(1, 5, editShareButton2);
    
    // 第3行
    m_sharedDataTable->setItem(2, 0, new QTableWidgetItem(tr("ML Model - Regression")));
    m_sharedDataTable->setItem(2, 1, new QTableWidgetItem(tr("Model")));
    m_sharedDataTable->setItem(2, 2, new QTableWidgetItem(tr("Project Team")));
    m_sharedDataTable->setItem(2, 3, new QTableWidgetItem(tr("Read/Execute")));
    m_sharedDataTable->setItem(2, 4, new QTableWidgetItem(tr("No Expiration")));
    
    QPushButton* editShareButton3 = new QPushButton(tr("Edit"), this);
    connect(editShareButton3, &QPushButton::clicked, [this]() { editDataSharing(); });
    m_sharedDataTable->setCellWidget(2, 5, editShareButton3);
    
    // 填充共享给我的数据表格
    m_sharedWithMeTable->setRowCount(2);
    
    // 第1行
    m_sharedWithMeTable->setItem(0, 0, new QTableWidgetItem(tr("Experimental Data")));
    m_sharedWithMeTable->setItem(0, 1, new QTableWidgetItem(tr("Alice Smith")));
    m_sharedWithMeTable->setItem(0, 2, new QTableWidgetItem(tr("Dataset")));
    m_sharedWithMeTable->setItem(0, 3, new QTableWidgetItem(tr("2024-10-15")));
    
    QPushButton* openButton1 = new QPushButton(tr("Open"), this);
    connect(openButton1, &QPushButton::clicked, [this]() { openSharedData(); });
    m_sharedWithMeTable->setCellWidget(0, 4, openButton1);
    
    // 第2行
    m_sharedWithMeTable->setItem(1, 0, new QTableWidgetItem(tr("Analysis Results")));
    m_sharedWithMeTable->setItem(1, 1, new QTableWidgetItem(tr("Bob Johnson")));
    m_sharedWithMeTable->setItem(1, 2, new QTableWidgetItem(tr("Results")));
    m_sharedWithMeTable->setItem(1, 3, new QTableWidgetItem(tr("2024-10-20")));
    
    QPushButton* openButton2 = new QPushButton(tr("Open"), this);
    connect(openButton2, &QPushButton::clicked, [this]() { openSharedData(); });
    m_sharedWithMeTable->setCellWidget(1, 4, openButton2);
}

void CollaborationWidget::populateUserTables()
{
    // 填充用户表格
    m_userTable->setRowCount(5);
    
    // 第1行
    m_userTable->setItem(0, 0, new QTableWidgetItem(tr("Alice Smith")));
    m_userTable->setItem(0, 1, new QTableWidgetItem("alice.smith@example.com"));
    m_userTable->setItem(0, 2, new QTableWidgetItem(tr("Chemistry")));
    m_userTable->setItem(0, 3, new QTableWidgetItem(tr("Researcher")));
    
    QPushButton* shareWithUserButton1 = new QPushButton(tr("Share"), this);
    connect(shareWithUserButton1, &QPushButton::clicked, [this]() { shareWithUser(); });
    m_userTable->setCellWidget(0, 4, shareWithUserButton1);
    
    // 第2行
    m_userTable->setItem(1, 0, new QTableWidgetItem(tr("Bob Johnson")));
    m_userTable->setItem(1, 1, new QTableWidgetItem("bob.johnson@example.com"));
    m_userTable->setItem(1, 2, new QTableWidgetItem(tr("Biology")));
    m_userTable->setItem(1, 3, new QTableWidgetItem(tr("Data Analyst")));
    
    QPushButton* shareWithUserButton2 = new QPushButton(tr("Share"), this);
    connect(shareWithUserButton2, &QPushButton::clicked, [this]() { shareWithUser(); });
    m_userTable->setCellWidget(1, 4, shareWithUserButton2);
    
    // 第3行
    m_userTable->setItem(2, 0, new QTableWidgetItem(tr("Carol Williams")));
    m_userTable->setItem(2, 1, new QTableWidgetItem("carol.williams@example.com"));
    m_userTable->setItem(2, 2, new QTableWidgetItem(tr("Computer Science")));
    m_userTable->setItem(2, 3, new QTableWidgetItem(tr("Developer")));
    
    QPushButton* shareWithUserButton3 = new QPushButton(tr("Share"), this);
    connect(shareWithUserButton3, &QPushButton::clicked, [this]() { shareWithUser(); });
    m_userTable->setCellWidget(2, 4, shareWithUserButton3);
    
    // 第4行
    m_userTable->setItem(3, 0, new QTableWidgetItem(tr("David Brown")));
    m_userTable->setItem(3, 1, new QTableWidgetItem("david.brown@example.com"));
    m_userTable->setItem(3, 2, new QTableWidgetItem(tr("Pharmacology")));
    m_userTable->setItem(3, 3, new QTableWidgetItem(tr("Principal Investigator")));
    
    QPushButton* shareWithUserButton4 = new QPushButton(tr("Share"), this);
    connect(shareWithUserButton4, &QPushButton::clicked, [this]() { shareWithUser(); });
    m_userTable->setCellWidget(3, 4, shareWithUserButton4);
    
    // 第5行
    m_userTable->setItem(4, 0, new QTableWidgetItem(tr("Eve Davis")));
    m_userTable->setItem(4, 1, new QTableWidgetItem("eve.davis@example.com"));
    m_userTable->setItem(4, 2, new QTableWidgetItem(tr("Statistics")));
    m_userTable->setItem(4, 3, new QTableWidgetItem(tr("Statistician")));
    
    QPushButton* shareWithUserButton5 = new QPushButton(tr("Share"), this);
    connect(shareWithUserButton5, &QPushButton::clicked, [this]() { shareWithUser(); });
    m_userTable->setCellWidget(4, 4, shareWithUserButton5);
    
    // 填充团队树
    QTreeWidgetItem* chemistryTeam = new QTreeWidgetItem(m_teamTreeWidget);
    chemistryTeam->setText(0, tr("Chemistry Team"));
    chemistryTeam->setText(1, tr("Team"));
    chemistryTeam->setText(2, tr("Chemistry"));
    chemistryTeam->setText(3, tr("Active"));
    
    QTreeWidgetItem* aliceItem = new QTreeWidgetItem(chemistryTeam);
    aliceItem->setText(0, tr("Alice Smith"));
    aliceItem->setText(1, tr("Lead Researcher"));
    aliceItem->setText(2, tr("Chemistry"));
    aliceItem->setText(3, tr("2024-11-20"));
    
    QTreeWidgetItem* bobItem = new QTreeWidgetItem(chemistryTeam);
    bobItem->setText(0, tr("Bob Johnson"));
    bobItem->setText(1, tr("Data Analyst"));
    bobItem->setText(2, tr("Chemistry"));
    bobItem->setText(3, tr("2024-11-22"));
    
    QTreeWidgetItem* biologyTeam = new QTreeWidgetItem(m_teamTreeWidget);
    biologyTeam->setText(0, tr("Biology Team"));
    biologyTeam->setText(1, tr("Team"));
    biologyTeam->setText(2, tr("Biology"));
    biologyTeam->setText(3, tr("Active"));
    
    QTreeWidgetItem* carolItem = new QTreeWidgetItem(biologyTeam);
    carolItem->setText(0, tr("Carol Williams"));
    carolItem->setText(1, tr("Bioinformatician"));
    carolItem->setText(2, tr("Biology"));
    carolItem->setText(3, tr("2024-11-21"));
    
    m_teamTreeWidget->expandAll();
}

void CollaborationWidget::populateActivityTable()
{
    // 填充活动表格
    m_activityTable->setRowCount(5);
    
    // 第1行
    m_activityTable->setItem(0, 0, new QTableWidgetItem("2024-11-20 14:30"));
    m_activityTable->setItem(0, 1, new QTableWidgetItem(tr("Alice Smith")));
    m_activityTable->setItem(0, 2, new QTableWidgetItem(tr("Shared")));
    m_activityTable->setItem(0, 3, new QTableWidgetItem(tr("Protein Dataset v1.0")));
    m_activityTable->setItem(0, 4, new QTableWidgetItem(tr("Shared with Research Team")));
    m_activityTable->setItem(0, 5, new QTableWidgetItem(tr("Success")));
    
    // 第2行
    m_activityTable->setItem(1, 0, new QTableWidgetItem("2024-11-20 15:45"));
    m_activityTable->setItem(1, 1, new QTableWidgetItem(tr("Current User")));
    m_activityTable->setItem(1, 2, new QTableWidgetItem(tr("Accessed")));
    m_activityTable->setItem(1, 3, new QTableWidgetItem(tr("Experimental Data")));
    m_activityTable->setItem(1, 4, new QTableWidgetItem(tr("Viewed data details")));
    m_activityTable->setItem(1, 5, new QTableWidgetItem(tr("Success")));
    
    // 第3行
    m_activityTable->setItem(2, 0, new QTableWidgetItem("2024-11-20 16:10"));
    m_activityTable->setItem(2, 1, new QTableWidgetItem(tr("Bob Johnson")));
    m_activityTable->setItem(2, 2, new QTableWidgetItem(tr("Commented")));
    m_activityTable->setItem(2, 3, new QTableWidgetItem(tr("Molecule Collection")));
    m_activityTable->setItem(2, 4, new QTableWidgetItem(tr("Added analysis notes")));
    m_activityTable->setItem(2, 5, new QTableWidgetItem(tr("Success")));
    
    // 第4行
    m_activityTable->setItem(3, 0, new QTableWidgetItem("2024-11-21 09:15"));
    m_activityTable->setItem(3, 1, new QTableWidgetItem(tr("Carol Williams")));
    m_activityTable->setItem(3, 2, new QTableWidgetItem(tr("Requested Access")));
    m_activityTable->setItem(3, 3, new QTableWidgetItem(tr("ML Model - Regression")));
    m_activityTable->setItem(3, 4, new QTableWidgetItem(tr("Requested read access")));
    m_activityTable->setItem(3, 5, new QTableWidgetItem(tr("Pending")));
    
    // 第5行
    m_activityTable->setItem(4, 0, new QTableWidgetItem("2024-11-21 11:30"));
    m_activityTable->setItem(4, 1, new QTableWidgetItem(tr("David Brown")));
    m_activityTable->setItem(4, 2, new QTableWidgetItem(tr("Created Team")));
    m_activityTable->setItem(4, 3, new QTableWidgetItem(tr("Pharmacology Team")));
    m_activityTable->setItem(4, 4, new QTableWidgetItem(tr("Created new team with 5 members")));
    m_activityTable->setItem(4, 5, new QTableWidgetItem(tr("Success")));
}

void CollaborationWidget::populateCommentTable()
{
    // 填充评论表格
    m_commentTable->setRowCount(3);
    
    // 第1行
    m_commentTable->setItem(0, 0, new QTableWidgetItem(tr("Alice Smith")));
    m_commentTable->setItem(0, 1, new QTableWidgetItem("2024-11-20 16:30"));
    m_commentTable->setItem(0, 2, new QTableWidgetItem(tr("Great dataset! I found some interesting patterns in the molecular properties.")));
    
    QPushButton* editCommentButton1 = new QPushButton(tr("Edit"), this);
    connect(editCommentButton1, &QPushButton::clicked, [this]() { editComment(); });
    m_commentTable->setCellWidget(0, 3, editCommentButton1);
    
    // 第2行
    m_commentTable->setItem(1, 0, new QTableWidgetItem(tr("Bob Johnson")));
    m_commentTable->setItem(1, 1, new QTableWidgetItem("2024-11-20 18:45"));
    m_commentTable->setItem(1, 2, new QTableWidgetItem(tr("I noticed some outliers in the data. Might be worth investigating further.")));
    
    QPushButton* editCommentButton2 = new QPushButton(tr("Edit"), this);
    connect(editCommentButton2, &QPushButton::clicked, [this]() { editComment(); });
    m_commentTable->setCellWidget(1, 3, editCommentButton2);
    
    // 第3行
    m_commentTable->setItem(2, 0, new QTableWidgetItem(tr("Current User")));
    m_commentTable->setItem(2, 1, new QTableWidgetItem("2024-11-21 10:15"));
    m_commentTable->setItem(2, 2, new QTableWidgetItem(tr("Thanks for the feedback! I'll look into those outliers.")));
    
    QPushButton* editCommentButton3 = new QPushButton(tr("Edit"), this);
    connect(editCommentButton3, &QPushButton::clicked, [this]() { editComment(); });
    m_commentTable->setCellWidget(2, 3, editCommentButton3);
    
    // 更新评论项目列表
    updateCommentItems();
}

void CollaborationWidget::updateCommentItems()
{
    m_commentItemCombo->clear();
    
    QString itemType = m_commentItemTypeCombo->currentData().toString();
    
    if (itemType == "molecule") {
        m_commentItemCombo->addItem(tr("Benzene"), "benzene");
        m_commentItemCombo->addItem(tr("Caffeine"), "caffeine");
        m_commentItemCombo->addItem(tr("Aspirin"), "aspirin");
        m_commentItemCombo->addItem(tr("DNA"), "dna");
        m_commentItemCombo->addItem(tr("Protein"), "protein");
    } else if (itemType == "dataset") {
        m_commentItemCombo->addItem(tr("Protein Dataset v1.0"), "protein_dataset");
        m_commentItemCombo->addItem(tr("Molecule Collection"), "molecule_collection");
        m_commentItemCombo->addItem(tr("Experimental Data"), "experimental_data");
        m_commentItemCombo->addItem(tr("Analysis Results"), "analysis_results");
    } else if (itemType == "model") {
        m_commentItemCombo->addItem(tr("ML Model - Regression"), "regression_model");
        m_commentItemCombo->addItem(tr("ML Model - Classification"), "classification_model");
        m_commentItemCombo->addItem(tr("Neural Network"), "neural_network");
        m_commentItemCombo->addItem(tr("Random Forest"), "random_forest");
    } else if (itemType == "project") {
        m_commentItemCombo->addItem(tr("Drug Discovery Project"), "drug_discovery");
        m_commentItemCombo->addItem(tr("Protein Analysis Project"), "protein_analysis");
        m_commentItemCombo->addItem(tr("Molecular Modeling Project"), "molecular_modeling");
        m_commentItemCombo->addItem(tr("Data Analysis Project"), "data_analysis");
    }
}

// 槽函数实现
void CollaborationWidget::refresh()
{
    // 刷新所有数据
    populateSharedDataTables();
    populateUserTables();
    populateActivityTable();
    populateCommentTable();
    
    m_statusLabel->setText(tr("Data refreshed"));
    
    Utils::Logger::info("Collaboration data refreshed");
}

void CollaborationWidget::shareData()
{
    // 创建数据共享对话框
    ShareDataDialog dialog(this);
    
    if (dialog.exec() == QDialog::Accepted) {
        // 获取共享数据
        QString itemName = dialog.getItemName();
        QString itemType = dialog.getItemType();
        QString shareWith = dialog.getShareWith();
        QString permissions = dialog.getPermissions();
        QDateTime expiration = dialog.getExpiration();
        
        // 执行共享操作
        if (m_dataSharing) {
            bool success = m_dataSharing->shareData(itemName, itemType, shareWith, permissions, expiration);
            
            if (success) {
                m_statusLabel->setText(tr("Data shared successfully"));
                QMessageBox::information(this, tr("Success"), tr("Data shared successfully"));
                
                // 刷新数据
                populateSharedDataTables();
            } else {
                m_statusLabel->setText(tr("Failed to share data"));
                QMessageBox::critical(this, tr("Error"), tr("Failed to share data"));
            }
        }
    }
}

void CollaborationWidget::editDataSharing()
{
    // 编辑数据共享
    int currentRow = m_sharedDataTable->currentRow();
    
    if (currentRow >= 0) {
        QString itemName = m_sharedDataTable->item(currentRow, 0)->text();
        
        // 创建编辑数据共享对话框
        EditShareDataDialog dialog(itemName, this);
        
        if (dialog.exec() == QDialog::Accepted) {
            // 获取编辑后的数据
            QString shareWith = dialog.getShareWith();
            QString permissions = dialog.getPermissions();
            QDateTime expiration = dialog.getExpiration();
            
            // 执行编辑操作
            if (m_dataSharing) {
                bool success = m_dataSharing->updateSharing(itemName, shareWith, permissions, expiration);
                
                if (success) {
                    m_statusLabel->setText(tr("Data sharing updated successfully"));
                    QMessageBox::information(this, tr("Success"), tr("Data sharing updated successfully"));
                    
                    // 刷新数据
                    populateSharedDataTables();
                } else {
                    m_statusLabel->setText(tr("Failed to update data sharing"));
                    QMessageBox::critical(this, tr("Error"), tr("Failed to update data sharing"));
                }
            }
        }
    } else {
        QMessageBox::information(this, tr("Information"), tr("Please select an item to edit"));
    }
}

void CollaborationWidget::revokeDataAccess()
{
    // 撤销数据访问
    int currentRow = m_sharedDataTable->currentRow();
    
    if (currentRow >= 0) {
        QString itemName = m_sharedDataTable->item(currentRow, 0)->text();
        
        // 确认撤销操作
        int ret = QMessageBox::question(
            this,
            tr("Confirm Revoke"),
            tr("Are you sure you want to revoke access to '%1'?").arg(itemName),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            // 执行撤销操作
            if (m_dataSharing) {
                bool success = m_dataSharing->revokeAccess(itemName);
                
                if (success) {
                    m_statusLabel->setText(tr("Access revoked successfully"));
                    QMessageBox::information(this, tr("Success"), tr("Access revoked successfully"));
                    
                    // 刷新数据
                    populateSharedDataTables();
                } else {
                    m_statusLabel->setText(tr("Failed to revoke access"));
                    QMessageBox::critical(this, tr("Error"), tr("Failed to revoke access"));
                }
            }
        }
    } else {
        QMessageBox::information(this, tr("Information"), tr("Please select an item to revoke access"));
    }
}

void CollaborationWidget::openSharedData()
{
    // 打开共享数据
    int currentRow = m_sharedWithMeTable->currentRow();
    
    if (currentRow >= 0) {
        QString itemName = m_sharedWithMeTable->item(currentRow, 0)->text();
        QString owner = m_sharedWithMeTable->item(currentRow, 1)->text();
        
        // 执行打开操作
        if (m_dataSharing) {
            bool success = m_dataSharing->openSharedData(itemName, owner);
            
            if (success) {
                m_statusLabel->setText(tr("Data opened successfully"));
                
                // 更新详情面板
                updateDetailsPanel(tr("Shared Data"), tr("Opening data: %1\nOwner: %2\nAccess: Read").arg(itemName).arg(owner));
            } else {
                m_statusLabel->setText(tr("Failed to open data"));
                QMessageBox::critical(this, tr("Error"), tr("Failed to open data"));
            }
        }
    } else {
        QMessageBox::information(this, tr("Information"), tr("Please select an item to open"));
    }
}

void CollaborationWidget::copySharedData()
{
    // 复制共享数据
    int currentRow = m_sharedWithMeTable->currentRow();
    
    if (currentRow >= 0) {
        QString itemName = m_sharedWithMeTable->item(currentRow, 0)->text();
        QString owner = m_sharedWithMeTable->item(currentRow, 1)->text();
        
        // 确认复制操作
        int ret = QMessageBox::question(
            this,
            tr("Confirm Copy"),
            tr("Are you sure you want to copy '%1' to your space?").arg(itemName),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            // 执行复制操作
            if (m_dataSharing) {
                bool success = m_dataSharing->copySharedData(itemName, owner);
                
                if (success) {
                    m_statusLabel->setText(tr("Data copied successfully"));
                    QMessageBox::information(this, tr("Success"), tr("Data copied successfully"));
                } else {
                    m_statusLabel->setText(tr("Failed to copy data"));
                    QMessageBox::critical(this, tr("Error"), tr("Failed to copy data"));
                }
            }
        }
    } else {
        QMessageBox::information(this, tr("Information"), tr("Please select an item to copy"));
    }
}

void CollaborationWidget::searchUsers()
{
    // 搜索用户
    QString searchText = m_userSearchEdit->text().trimmed();
    
    if (searchText.isEmpty()) {
        // 如果搜索文本为空，显示所有用户
        populateUserTables();
        return;
    }
    
    // 这里应该执行实际的搜索操作
    // 为了演示，我们只是显示一个消息
    QMessageBox::information(this, tr("Search Results"), tr("Search for '%1' returned 3 results").arg(searchText));
    
    m_statusLabel->setText(tr("Search completed"));
}

void CollaborationWidget::viewUserProfile()
{
    // 查看用户资料
    int currentRow = m_userTable->currentRow();
    
    if (currentRow >= 0) {
        QString userName = m_userTable->item(currentRow, 0)->text();
        
        // 这里应该显示用户详细资料
        // 为了演示，我们只更新详情面板
        updateDetailsPanel(tr("User Profile"), tr("User: %1\nEmail: %2\nDepartment: %3\nRole: %4")
                          .arg(userName)
                          .arg(m_userTable->item(currentRow, 1)->text())
                          .arg(m_userTable->item(currentRow, 2)->text())
                          .arg(m_userTable->item(currentRow, 3)->text()));
    } else {
        QMessageBox::information(this, tr("Information"), tr("Please select a user to view profile"));
    }
}

void CollaborationWidget::shareWithUser()
{
    // 与用户共享数据
    int currentRow = m_userTable->currentRow();
    
    if (currentRow >= 0) {
        QString userName = m_userTable->item(currentRow, 0)->text();
        
        // 创建共享数据对话框
        ShareDataDialog dialog(this);
        dialog.setShareWith(userName);
        
        if (dialog.exec() == QDialog::Accepted) {
            // 获取共享数据
            QString itemName = dialog.getItemName();
            QString itemType = dialog.getItemType();
            QString shareWith = dialog.getShareWith();
            QString permissions = dialog.getPermissions();
            QDateTime expiration = dialog.getExpiration();
            
            // 执行共享操作
            if (m_dataSharing) {
                bool success = m_dataSharing->shareData(itemName, itemType, shareWith, permissions, expiration);
                
                if (success) {
                    m_statusLabel->setText(tr("Data shared successfully"));
                    QMessageBox::information(this, tr("Success"), tr("Data shared successfully with %1").arg(shareWith));
                    
                    // 刷新数据
                    populateSharedDataTables();
                } else {
                    m_statusLabel->setText(tr("Failed to share data"));
                    QMessageBox::critical(this, tr("Error"), tr("Failed to share data"));
                }
            }
        }
    } else {
        QMessageBox::information(this, tr("Information"), tr("Please select a user to share with"));
    }
}

void CollaborationWidget::createTeam()
{
    // 创建团队
    QMessageBox::information(this, tr("Create Team"), tr("Team creation dialog would be shown here"));
    m_statusLabel->setText(tr("Team creation dialog opened"));
}

void CollaborationWidget::editTeam()
{
    // 编辑团队
    QMessageBox::information(this, tr("Edit Team"), tr("Team editing dialog would be shown here"));
    m_statusLabel->setText(tr("Team editing dialog opened"));
}

void CollaborationWidget::inviteToTeam()
{
    // 邀请加入团队
    QMessageBox::information(this, tr("Invite to Team"), tr("Team invitation dialog would be shown here"));
    m_statusLabel->setText(tr("Team invitation dialog opened"));
}

void CollaborationWidget::applyActivityFilter()
{
    // 应用活动筛选
    QString userFilter = m_activityUserFilter->currentData().toString();
    QString typeFilter = m_activityTypeFilter->currentData().toString();
    QDateTime dateFilter = m_activityDateFilter->dateTime();
    
    // 这里应该执行实际的筛选操作
    // 为了演示，我们只显示一个消息
    QMessageBox::information(this, tr("Filter Applied"), tr("Activity filter applied"));
    
    m_statusLabel->setText(tr("Activity filter applied"));
}

void CollaborationWidget::exportActivityHistory()
{
    // 导出活动历史
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Export Activity History"),
        QString(),
        tr("CSV Files (*.csv);;All Files (*)"));
    
    if (!fileName.isEmpty()) {
        // 这里应该执行实际的导出操作
        // 为了演示，我们只显示一个消息
        QMessageBox::information(this, tr("Export Successful"), tr("Activity history exported to %1").arg(fileName));
        
        m_statusLabel->setText(tr("Activity history exported"));
    }
}

void CollaborationWidget::clearActivityHistory()
{
    // 清除活动历史
    int ret = QMessageBox::question(
        this,
        tr("Confirm Clear"),
        tr("Are you sure you want to clear the activity history?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // 执行清除操作
        m_activityTable->setRowCount(0);
        
        m_statusLabel->setText(tr("Activity history cleared"));
    }
}

void CollaborationWidget::editComment()
{
    // 编辑评论
    int currentRow = m_commentTable->currentRow();
    
    if (currentRow >= 0) {
        QString author = m_commentTable->item(currentRow, 0)->text();
        QString timestamp = m_commentTable->item(currentRow, 1)->text();
        QString commentText = m_commentTable->item(currentRow, 2)->text();
        
        // 检查是否是当前用户的评论
        if (author != tr("Current User")) {
            QMessageBox::information(this, tr("Information"), tr("You can only edit your own comments"));
            return;
        }
        
        // 创建编辑评论对话框
        EditCommentDialog dialog(commentText, this);
        
        if (dialog.exec() == QDialog::Accepted) {
            // 获取编辑后的评论文本
            QString newCommentText = dialog.getCommentText();
            
            // 更新表格
            m_commentTable->setItem(currentRow, 2, new QTableWidgetItem(newCommentText));
            
            m_statusLabel->setText(tr("Comment updated successfully"));
        }
    } else {
        QMessageBox::information(this, tr("Information"), tr("Please select a comment to edit"));
    }
}

void CollaborationWidget::deleteComment()
{
    // 删除评论
    int currentRow = m_commentTable->currentRow();
    
    if (currentRow >= 0) {
        QString author = m_commentTable->item(currentRow, 0)->text();
        
        // 检查是否是当前用户的评论
        if (author != tr("Current User")) {
            QMessageBox::information(this, tr("Information"), tr("You can only delete your own comments"));
            return;
        }
        
        // 确认删除操作
        int ret = QMessageBox::question(
            this,
            tr("Confirm Delete"),
            tr("Are you sure you want to delete this comment?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            // 执行删除操作
            m_commentTable->removeRow(currentRow);
            
            m_statusLabel->setText(tr("Comment deleted successfully"));
        }
    } else {
        QMessageBox::information(this, tr("Information"), tr("Please select a comment to delete"));
    }
}

void CollaborationWidget::postComment()
{
    // 发表评论
    QString commentText = m_newCommentEdit->toPlainText().trimmed();
    
    if (commentText.isEmpty()) {
        QMessageBox::information(this, tr("Information"), tr("Please enter a comment"));
        return;
    }
    
    // 添加新评论到表格
    int row = m_commentTable->rowCount();
    m_commentTable->insertRow(row);
    
    // 作者
    QTableWidgetItem* authorItem = new QTableWidgetItem(tr("Current User"));
    authorItem->setFlags(authorItem->flags() & ~Qt::ItemIsEditable);
    m_commentTable->setItem(row, 0, authorItem);
    
    // 时间戳
    QTableWidgetItem* timestampItem = new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));
    timestampItem->setFlags(timestampItem->flags() & ~Qt::ItemIsEditable);
    m_commentTable->setItem(row, 1, timestampItem);
    
    // 评论内容
    QTableWidgetItem* commentItem = new QTableWidgetItem(commentText);
    commentItem->setFlags(commentItem->flags() & ~Qt::ItemIsEditable);
    m_commentTable->setItem(row, 2, commentItem);
    
    // 操作按钮
    QPushButton* editButton = new QPushButton(tr("Edit"), this);
    connect(editButton, &QPushButton::clicked, [this]() { editComment(); });
    m_commentTable->setCellWidget(row, 3, editButton);
    
    // 清空输入框
    m_newCommentEdit->clear();
    
    m_statusLabel->setText(tr("Comment posted successfully"));
    
    // 记录到活动历史
    logActivity(tr("Current User"), tr("Commented"), m_commentItemCombo->currentText(), tr("Added comment"));
}

void CollaborationWidget::showNotifications()
{
    // 显示通知
    QMessageBox::information(this, tr("Notifications"), tr("You have 3 new notifications"));
    
    // 清除通知指示器
    m_notificationsAction->setIcon(QIcon(":/icons/notifications_empty.png"));
}

void CollaborationWidget::updateDetailsPanel(const QString& title, const QString& content)
{
    // 更新详情面板
    m_detailsTitleLabel->setText(title);
    m_detailsContentTextEdit->setPlainText(content);
    
    // 清除之前的按钮
    while (m_detailsButtonLayout->count() > 0) {
        QLayoutItem* item = m_detailsButtonLayout->takeAt(0);
        delete item->widget();
        delete item;
    }
    
    // 根据内容类型添加相应的按钮
    if (title == tr("User Profile")) {
        QPushButton* shareButton = new QPushButton(tr("Share With User"), this);
        connect(shareButton, &QPushButton::clicked, this, &CollaborationWidget::shareWithUser);
        m_detailsButtonLayout->addWidget(shareButton);
        
        QPushButton* messageButton = new QPushButton(tr("Send Message"), this);
        connect(messageButton, &QPushButton::clicked, [this]() { 
            QMessageBox::information(this, tr("Message"), tr("Message dialog would be shown here"));
        });
        m_detailsButtonLayout->addWidget(messageButton);
    } else if (title == tr("Shared Data")) {
        QPushButton* openButton = new QPushButton(tr("Open"), this);
        connect(openButton, &QPushButton::clicked, this, &CollaborationWidget::openSharedData);
        m_detailsButtonLayout->addWidget(openButton);
        
        QPushButton* copyButton = new QPushButton(tr("Copy to My Space"), this);
        connect(copyButton, &QPushButton::clicked, this, &CollaborationWidget::copySharedData);
        m_detailsButtonLayout->addWidget(copyButton);
    }
    
    m_detailsButtonLayout->addStretch();
}

void CollaborationWidget::logActivity(const QString& user, const QString& action, const QString& target, const QString& details)
{
    // 记录活动
    int row = m_activityTable->rowCount();
    m_activityTable->insertRow(row);
    
    // 时间戳
    QTableWidgetItem* timestampItem = new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));
    timestampItem->setFlags(timestampItem->flags() & ~Qt::ItemIsEditable);
    m_activityTable->setItem(row, 0, timestampItem);
    
    // 用户
    QTableWidgetItem* userItem = new QTableWidgetItem(user);
    userItem->setFlags(userItem->flags() & ~Qt::ItemIsEditable);
    m_activityTable->setItem(row, 1, userItem);
    
    // 操作
    QTableWidgetItem* actionItem = new QTableWidgetItem(action);
    actionItem->setFlags(actionItem->flags() & ~Qt::ItemIsEditable);
    m_activityTable->setItem(row, 2, actionItem);
    
    // 目标
    QTableWidgetItem* targetItem = new QTableWidgetItem(target);
    targetItem->setFlags(targetItem->flags() & ~Qt::ItemIsEditable);
    m_activityTable->setItem(row, 3, targetItem);
    
    // 详情
    QTableWidgetItem* detailsItem = new QTableWidgetItem(details);
    detailsItem->setFlags(detailsItem->flags() & ~Qt::ItemIsEditable);
    m_activityTable->setItem(row, 4, detailsItem);
    
    // 状态
    QTableWidgetItem* statusItem = new QTableWidgetItem(tr("Success"));
    statusItem->setFlags(statusItem->flags() & ~Qt::ItemIsEditable);
    m_activityTable->setItem(row, 5, statusItem);
}

// 表格选择事件处理
void CollaborationWidget::onSharedDataSelected(int row, int column)
{
    Q_UNUSED(column);
    
    if (row >= 0 && row < m_sharedDataTable->rowCount()) {
        QString itemName = m_sharedDataTable->item(row, 0)->text();
        QString itemType = m_sharedDataTable->item(row, 1)->text();
        QString shareWith = m_sharedDataTable->item(row, 2)->text();
        QString permissions = m_sharedDataTable->item(row, 3)->text();
        QString expiration = m_sharedDataTable->item(row, 4)->text();
        
        updateDetailsPanel(tr("Shared Data"), tr("Name: %1\nType: %2\nShared with: %3\nPermissions: %4\nExpiration: %5")
                          .arg(itemName)
                          .arg(itemType)
                          .arg(shareWith)
                          .arg(permissions)
                          .arg(expiration));
    }
}

void CollaborationWidget::onSharedWithMeSelected(int row, int column)
{
    Q_UNUSED(column);
    
    if (row >= 0 && row < m_sharedWithMeTable->rowCount()) {
        QString itemName = m_sharedWithMeTable->item(row, 0)->text();
        QString owner = m_sharedWithMeTable->item(row, 1)->text();
        QString itemType = m_sharedWithMeTable->item(row, 2)->text();
        QString sharedDate = m_sharedWithMeTable->item(row, 3)->text();
        
        updateDetailsPanel(tr("Shared Data"), tr("Name: %1\nOwner: %2\nType: %3\nShared date: %4")
                          .arg(itemName)
                          .arg(owner)
                          .arg(itemType)
                          .arg(sharedDate));
    }
}

void CollaborationWidget::onUserSelected(int row, int column)
{
    Q_UNUSED(column);
    
    if (row >= 0 && row < m_userTable->rowCount()) {
        QString userName = m_userTable->item(row, 0)->text();
        QString email = m_userTable->item(row, 1)->text();
        QString department = m_userTable->item(row, 2)->text();
        QString role = m_userTable->item(row, 3)->text();
        
        updateDetailsPanel(tr("User Profile"), tr("Name: %1\nEmail: %2\nDepartment: %3\nRole: %4")
                          .arg(userName)
                          .arg(email)
                          .arg(department)
                          .arg(role));
    }
}

void CollaborationWidget::onActivitySelected(int row, int column)
{
    Q_UNUSED(column);
    
    if (row >= 0 && row < m_activityTable->rowCount()) {
        QString timestamp = m_activityTable->item(row, 0)->text();
        QString user = m_activityTable->item(row, 1)->text();
        QString action = m_activityTable->item(row, 2)->text();
        QString target = m_activityTable->item(row, 3)->text();
        QString details = m_activityTable->item(row, 4)->text();
        QString status = m_activityTable->item(row, 5)->text();
        
        updateDetailsPanel(tr("Activity Details"), tr("Timestamp: %1\nUser: %2\nAction: %3\nTarget: %4\nDetails: %5\nStatus: %6")
                          .arg(timestamp)
                          .arg(user)
                          .arg(action)
                          .arg(target)
                          .arg(details)
                          .arg(status));
    }
}

void CollaborationWidget::onCommentSelected(int row, int column)
{
    Q_UNUSED(column);
    
    if (row >= 0 && row < m_commentTable->rowCount()) {
        QString author = m_commentTable->item(row, 0)->text();
        QString timestamp = m_commentTable->item(row, 1)->text();
        QString commentText = m_commentTable->item(row, 2)->text();
        
        updateDetailsPanel(tr("Comment Details"), tr("Author: %1\nTimestamp: %2\nComment: %3")
                          .arg(author)
                          .arg(timestamp)
                          .arg(commentText));
    }
}

} // namespace UI