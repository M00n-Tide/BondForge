#ifndef BONDFORGE_COLLABORATIONWIDGET_H
#define BONDFORGE_COLLABORATIONWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QTableView>
#include <QTreeView>
#include <QTableWidget>
#include <QTreeWidget>
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
#include <QListWidget>
#include <QProgressBar>
#include <QDateTimeEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QMessageBox>
#include <memory>

// 前向声明
namespace BondForge {
    namespace Core {
        namespace Data {
            class DataService;
        }
        namespace Collaboration {
            class User;
            class Comment;
            class DataSharing;
            class Project;
        }
        namespace Permissions {
            class PermissionManager;
        }
    }
}

namespace BondForge {
namespace UI {

class CollaborationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CollaborationWidget(std::shared_ptr<Core::Data::DataService> dataService, QWidget *parent = nullptr);
    ~CollaborationWidget();

protected:
    void showEvent(QShowEvent *event) override;

private slots:
    // 标签页变化
    void onTabChanged(int index);
    
    // 用户管理槽函数
    void onUserAddClicked();
    void onUserEditClicked();
    void onUserDeleteClicked();
    void onUserRefreshClicked();
    void onUserSelectionChanged();
    void onUserFilterChanged(const QString &text);
    void onUserRoleChanged();
    void onUserStatusChanged();
    
    // 数据共享槽函数
    void onSharingAddClicked();
    void onSharingEditClicked();
    void onSharingDeleteClicked();
    void onSharingRefreshClicked();
    void onSharingSelectionChanged();
    void onShareableDataSelectionChanged();
    void onSharePermissionChanged();
    void onShareExpirationChanged();
    
    // 评论系统槽函数
    void onCommentAddClicked();
    void onCommentEditClicked();
    void onCommentDeleteClicked();
    void onCommentRefreshClicked();
    void onCommentSelectionChanged();
    void onRecordSelectionChanged();
    void onReplyToComment();
    
    // 版本历史槽函数
    void onVersionHistoryRefreshClicked();
    void onVersionDiffClicked();
    void onVersionRestoreClicked();
    void onVersionSelectionChanged();
    void onRecordSelectedForHistory();
    
    // 项目管理槽函数
    void onProjectAddClicked();
    void onProjectEditClicked();
    void onProjectDeleteClicked();
    void onProjectRefreshClicked();
    void onProjectSelectionChanged();
    void onProjectMemberAddClicked();
    void onProjectMemberRemoveClicked();
    void onProjectMemberRoleChanged();
    
    // 数据服务信号
    void onUserLoaded(const Core::Collaboration::User &user);
    void onUserUpdated(const Core::Collaboration::User &user);
    void onUserDeleted(const QString &userId);
    void onSharingLoaded(const Core::Collaboration::DataSharing &sharing);
    void onSharingUpdated(const Core::Collaboration::DataSharing &sharing);
    void onSharingDeleted(const QString &sharingId);
    void onCommentLoaded(const Core::Collaboration::Comment &comment);
    void onCommentUpdated(const Core::Collaboration::Comment &comment);
    void onCommentDeleted(const QString &commentId);

private:
    void setupUI();
    void setupUserManagementTab();
    void setupDataSharingTab();
    void setupCommentSystemTab();
    void setupVersionHistoryTab();
    void setupProjectManagementTab();
    void setupToolbar();
    
    void connectSignals();
    
    // 用户管理
    void loadUsers();
    void loadUserDetails(const QString &userId);
    void saveUser();
    void deleteUser(const QString &userId);
    void clearUserDetails();
    void updateUsersTable();
    void filterUsers(const QString &filter);
    
    // 数据共享
    void loadSharedData();
    void loadShareableData();
    void loadSharingDetails(const QString &sharingId);
    void saveSharing();
    void deleteSharing(const QString &sharingId);
    void clearSharingDetails();
    void updateSharedDataTable();
    void updateShareableDataTable();
    
    // 评论系统
    void loadComments(const QString &recordId);
    void loadRecords();
    void saveComment();
    void deleteComment(const QString &commentId);
    void replyToComment(const QString &commentId);
    void clearCommentDetails();
    void updateCommentsList();
    void updateRecordsList();
    
    // 版本历史
    void loadVersionHistory(const QString &recordId);
    void loadVersionDetails(const QString &versionId);
    void showVersionDiff(const QString &versionId1, const QString &versionId2);
    void restoreVersion(const QString &versionId);
    void clearVersionDetails();
    void updateVersionHistory();
    
    // 项目管理
    void loadProjects();
    void loadProjectDetails(const QString &projectId);
    void loadProjectMembers(const QString &projectId);
    void saveProject();
    void deleteProject(const QString &projectId);
    void addProjectMember(const QString &projectId, const QString &userId, const QString &role);
    void removeProjectMember(const QString &projectId, const QString &userId);
    void changeProjectMemberRole(const QString &projectId, const QString &userId, const QString &role);
    void clearProjectDetails();
    void updateProjectsTable();
    void updateProjectMembersList();
    
    // 工具函数
    void updateStatusMessage(const QString &message);
    void showProgress(bool show);
    void setProgressValue(int value);
    
    // UI组件
    QSplitter* m_mainSplitter;
    
    // 工具栏
    QWidget* m_toolbarWidget;
    QPushButton* m_refreshButton;
    
    // 主标签页
    QTabWidget* m_mainTabs;
    
    // 用户管理标签页
    QWidget* m_userTab;
    QSplitter* m_userSplitter;
    QTableView* m_userTable;
    QWidget* m_userDetailsWidget;
    QLineEdit* m_userNameEdit;
    QLineEdit* m_userEmailEdit;
    QLineEdit* m_userUsernameEdit;
    QComboBox* m_userRoleCombo;
    QComboBox* m_userStatusCombo;
    QLineEdit* m_userFilterEdit;
    QPushButton* m_userAddButton;
    QPushButton* m_userEditButton;
    QPushButton* m_userDeleteButton;
    QPushButton* m_userRefreshButton;
    
    // 数据共享标签页
    QWidget* m_sharingTab;
    QSplitter* m_sharingSplitter;
    QTableView* m_shareableDataTable;
    QTableView* m_sharedDataTable;
    QWidget* m_sharingDetailsWidget;
    QLineEdit* m_sharingIdEdit;
    QLineEdit* m_sharingNameEdit;
    QComboBox* m_sharingPermissionCombo;
    QComboBox* m_sharingShareeCombo;
    QDateTimeEdit* m_sharingExpirationEdit;
    QTextEdit* m_sharingDescriptionEdit;
    QPushButton* m_sharingAddButton;
    QPushButton* m_sharingEditButton;
    QPushButton* m_sharingDeleteButton;
    QPushButton* m_sharingRefreshButton;
    
    // 评论系统标签页
    QWidget* m_commentTab;
    QSplitter* m_commentSplitter;
    QListWidget* m_recordsList;
    QListWidget* m_commentsList;
    QWidget* m_commentDetailsWidget;
    QLineEdit* m_commentIdEdit;
    QTextEdit* m_commentContentEdit;
    QLabel* m_commentAuthorLabel;
    QLabel* m_commentTimestampLabel;
    QPushButton* m_commentAddButton;
    QPushButton* m_commentEditButton;
    QPushButton* m_commentDeleteButton;
    QPushButton* m_commentReplyButton;
    QPushButton* m_commentRefreshButton;
    
    // 版本历史标签页
    QWidget* m_versionTab;
    QSplitter* m_versionSplitter;
    QTableWidget* m_recordsForHistoryTable;
    QTableWidget* m_versionHistoryTable;
    QWidget* m_versionDetailsWidget;
    QTextEdit* m_versionDiffEdit;
    QLabel* m_versionIdLabel;
    QLabel* m_versionAuthorLabel;
    QLabel* m_versionTimestampLabel;
    QLabel* m_versionDescriptionLabel;
    QPushButton* m_versionDiffButton;
    QPushButton* m_versionRestoreButton;
    QPushButton* m_versionRefreshButton;
    
    // 项目管理标签页
    QWidget* m_projectTab;
    QSplitter* m_projectSplitter;
    QTableWidget* m_projectTable;
    QTreeWidget* m_projectMembersTree;
    QWidget* m_projectDetailsWidget;
    QLineEdit* m_projectNameEdit;
    QLineEdit* m_projectDescriptionEdit;
    QDateTimeEdit* m_projectCreatedEdit;
    QDateTimeEdit* m_projectModifiedEdit;
    QPushButton* m_projectAddButton;
    QPushButton* m_projectEditButton;
    QPushButton* m_projectDeleteButton;
    QPushButton* m_projectMemberAddButton;
    QPushButton* m_projectMemberRemoveButton;
    QPushButton* m_projectRefreshButton;
    
    // 状态栏
    QLabel* m_statusLabel;
    QProgressBar* m_progressBar;
    
    // 服务
    std::shared_ptr<Core::Data::DataService> m_dataService;
    std::shared_ptr<Core::Permissions::PermissionManager> m_permissionManager;
    
    // 状态
    bool m_isDataLoaded;
    QString m_selectedUserId;
    QString m_selectedSharingId;
    QString m_selectedCommentId;
    QString m_selectedRecordId;
    QString m_selectedVersionId;
    QString m_selectedProjectId;
};

} // namespace UI
} // namespace BondForge

#endif // BONDFORGE_COLLABORATIONWIDGET_H