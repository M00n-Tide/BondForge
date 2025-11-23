#include "User.h"
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

namespace BondForge {
namespace Core {
namespace Collaboration {

// UserService 实现
UserService::UserService() {
    // 初始化角色权限映射
    initializeRolePermissions();
    
    // 创建默认管理员用户
    User adminUser;
    adminUser.id = "admin-001";
    adminUser.username = "admin";
    adminUser.email = "admin@bondforge.com";
    adminUser.fullName = "System Administrator";
    adminUser.role = UserRole::Admin;
    adminUser.status = UserStatus::Active;
    adminUser.department = "IT";
    adminUser.createdTime = std::chrono::system_clock::now();
    adminUser.lastLoginTime = std::chrono::system_clock::now();
    adminUser.bio = "Default system administrator account";
    
    m_users.push_back(adminUser);
    m_userCredentials["admin"] = hashPassword("admin123"); // 默认密码
    
    // 添加一些示例用户
    addUserSampleUsers();
}

void UserService::initializeRolePermissions() {
    // 查看者 - 只能读取数据
    m_rolePermissions[UserRole::Viewer] = {Permission::Read};
    
    // 访客 - 能读取数据和添加评论
    m_rolePermissions[UserRole::Guest] = {Permission::Read};
    
    // 分析师 - 能读取、分析和导出数据
    m_rolePermissions[UserRole::Analyst] = {
        Permission::Read, Permission::ExportData, Permission::ImportData
    };
    
    // 研究员 - 能读取、编辑、分析和导出数据
    m_rolePermissions[UserRole::Researcher] = {
        Permission::Read, Permission::Write, Permission::ExportData, Permission::ImportData
    };
    
    // 管理员 - 能管理用户和数据
    m_rolePermissions[UserRole::Manager] = {
        Permission::Read, Permission::Write, Permission::Delete, 
        Permission::Share, Permission::ManageUsers, Permission::ExportData, Permission::ImportData
    };
    
    // 系统管理员 - 拥有所有权限
    m_rolePermissions[UserRole::Admin] = {
        Permission::Read, Permission::Write, Permission::Delete, 
        Permission::Share, Permission::ManageUsers, Permission::ManageSystem,
        Permission::ExportData, Permission::ImportData
    };
}

void UserService::addUserSampleUsers() {
    // 添加示例研究员
    User researcher;
    researcher.id = "user-researcher-001";
    researcher.username = "researcher1";
    researcher.email = "researcher1@example.com";
    researcher.fullName = "张研究员";
    researcher.role = UserRole::Researcher;
    researcher.status = UserStatus::Active;
    researcher.department = "化学研究部";
    researcher.createdTime = std::chrono::system_clock::now();
    researcher.lastLoginTime = std::chrono::system_clock::now();
    researcher.bio = "专注于有机化学分子结构研究";
    
    m_users.push_back(researcher);
    m_userCredentials["researcher1"] = hashPassword("password123");
    
    // 添加示例分析师
    User analyst;
    analyst.id = "user-analyst-001";
    analyst.username = "analyst1";
    analyst.email = "analyst1@example.com";
    analyst.fullName = "李分析师";
    analyst.role = UserRole::Analyst;
    analyst.status = UserStatus::Active;
    analyst.department = "数据分析部";
    analyst.createdTime = std::chrono::system_clock::now();
    analyst.lastLoginTime = std::chrono::system_clock::now();
    analyst.bio = "专业化学数据分析师，擅长机器学习";
    
    m_users.push_back(analyst);
    m_userCredentials["analyst1"] = hashPassword("password123");
    
    // 添加示例访客
    User guest;
    guest.id = "user-guest-001";
    guest.username = "guest1";
    guest.email = "guest1@example.com";
    guest.fullName = "王访客";
    guest.role = UserRole::Guest;
    guest.status = UserStatus::Active;
    guest.department = "外部合作";
    guest.createdTime = std::chrono::system_clock::now();
    guest.lastLoginTime = std::chrono::system_clock::now();
    guest.bio = "外部合作研究者";
    
    m_users.push_back(guest);
    m_userCredentials["guest1"] = hashPassword("password123");
}

std::string UserService::hashPassword(const std::string& password) {
    // 使用SHA-256哈希（实际应用中应使用更安全的哈希算法和加盐）
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password.c_str(), password.length());
    SHA256_Final(hash, &sha256);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return ss.str();
}

bool UserService::verifyPassword(const std::string& password, const std::string& hash) {
    return hashPassword(password) == hash;
}

bool UserService::addUser(const User& user) {
    // 检查用户名是否已存在
    for (const auto& existingUser : m_users) {
        if (existingUser.username == user.username) {
            return false; // 用户名已存在
        }
    }
    
    // 添加用户
    m_users.push_back(user);
    return true;
}

bool UserService::updateUser(const User& user) {
    for (auto& existingUser : m_users) {
        if (existingUser.id == user.id) {
            existingUser = user;
            return true;
        }
    }
    return false; // 用户不存在
}

bool UserService::deleteUser(const std::string& userId) {
    auto it = std::find_if(m_users.begin(), m_users.end(),
        [&userId](const User& user) { return user.id == userId; });
    
    if (it != m_users.end()) {
        m_users.erase(it);
        return true;
    }
    
    return false; // 用户不存在
}

std::unique_ptr<User> UserService::getUser(const std::string& userId) {
    for (const auto& user : m_users) {
        if (user.id == userId) {
            return std::make_unique<User>(user);
        }
    }
    
    return nullptr; // 用户不存在
}

std::unique_ptr<User> UserService::getUserByUsername(const std::string& username) {
    for (const auto& user : m_users) {
        if (user.username == username) {
            return std::make_unique<User>(user);
        }
    }
    
    return nullptr; // 用户不存在
}

std::vector<User> UserService::getAllUsers() {
    return m_users;
}

bool UserService::hasPermission(const std::string& userId, Permission permission) {
    auto userPtr = getUser(userId);
    if (!userPtr) {
        return false; // 用户不存在
    }
    
    // 检查用户角色是否包含该权限
    auto rolePermissions = m_rolePermissions[userPtr->role];
    return rolePermissions.find(permission) != rolePermissions.end() || 
           userPtr->permissions.find(PermissionUtils::permissionToString(permission)) != userPtr->permissions.end();
}

bool UserService::grantPermission(const std::string& userId, Permission permission) {
    for (auto& user : m_users) {
        if (user.id == userId) {
            user.permissions.insert(PermissionUtils::permissionToString(permission));
            return true;
        }
    }
    
    return false; // 用户不存在
}

bool UserService::revokePermission(const std::string& userId, Permission permission) {
    for (auto& user : m_users) {
        if (user.id == userId) {
            user.permissions.erase(PermissionUtils::permissionToString(permission));
            return true;
        }
    }
    
    return false; // 用户不存在
}

std::string UserService::authenticateUser(const std::string& username, const std::string& password) {
    auto credIt = m_userCredentials.find(username);
    if (credIt == m_userCredentials.end()) {
        return ""; // 用户不存在
    }
    
    if (verifyPassword(password, credIt->second)) {
        // 认证成功，更新最后登录时间
        auto userPtr = getUserByUsername(username);
        if (userPtr) {
            for (auto& user : m_users) {
                if (user.username == username) {
                    user.lastLoginTime = std::chrono::system_clock::now();
                    break;
                }
            }
            return userPtr->id;
        }
    }
    
    return ""; // 密码错误
}

// PermissionUtils 实现
std::string PermissionUtils::roleToString(UserRole role) {
    switch (role) {
        case UserRole::Viewer: return "Viewer";
        case UserRole::Guest: return "Guest";
        case UserRole::Analyst: return "Analyst";
        case UserRole::Researcher: return "Researcher";
        case UserRole::Manager: return "Manager";
        case UserRole::Admin: return "Admin";
        default: return "Unknown";
    }
}

std::string PermissionUtils::statusToString(UserStatus status) {
    switch (status) {
        case UserStatus::Active: return "Active";
        case UserStatus::Inactive: return "Inactive";
        case UserStatus::Suspended: return "Suspended";
        case UserStatus::Pending: return "Pending";
        default: return "Unknown";
    }
}

std::string PermissionUtils::permissionToString(Permission permission) {
    switch (permission) {
        case Permission::Read: return "Read";
        case Permission::Write: return "Write";
        case Permission::Delete: return "Delete";
        case Permission::Share: return "Share";
        case Permission::ManageUsers: return "Manage Users";
        case Permission::ManageSystem: return "Manage System";
        case Permission::ExportData: return "Export Data";
        case Permission::ImportData: return "Import Data";
        default: return "Unknown";
    }
}

bool PermissionUtils::roleHasPermission(UserRole role, Permission permission) {
    switch (role) {
        case UserRole::Viewer:
            return permission == Permission::Read;
            
        case UserRole::Guest:
            return permission == Permission::Read;
            
        case UserRole::Analyst:
            return permission == Permission::Read || 
                   permission == Permission::ExportData || 
                   permission == Permission::ImportData;
            
        case UserRole::Researcher:
            return permission == Permission::Read || 
                   permission == Permission::Write || 
                   permission == Permission::ExportData || 
                   permission == Permission::ImportData;
            
        case UserRole::Manager:
            return permission != Permission::ManageSystem;
            
        case UserRole::Admin:
            return true; // 管理员拥有所有权限
            
        default:
            return false;
    }
}

} // namespace Collaboration
} // namespace Core
} // namespace BondForge