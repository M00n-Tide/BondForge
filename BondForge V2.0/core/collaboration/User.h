#pragma once

#include <string>
#include <vector>
#include <set>
#include <chrono>

namespace BondForge {
namespace Core {
namespace Collaboration {

/**
 * @brief 用户角色枚举
 */
enum class UserRole {
    Viewer,        // 查看者 - 只能查看数据
    Guest,         // 访客 - 可以查看和添加评论
    Analyst,       // 分析师 - 可以查看、评论和分析数据
    Researcher,    // 研究员 - 可以查看、评论、分析和编辑数据
    Manager,       // 管理员 - 可以管理用户和数据
    Admin          // 系统管理员 - 拥有所有权限
};

/**
 * @brief 用户状态枚举
 */
enum class UserStatus {
    Active,        // 活跃
    Inactive,      // 非活跃
    Suspended,     // 暂停
    Pending        // 待激活
};

/**
 * @brief 用户信息结构
 */
struct User {
    std::string id;                    // 用户ID
    std::string username;                // 用户名
    std::string email;                   // 邮箱
    std::string fullName;                // 全名
    UserRole role;                      // 角色
    UserStatus status;                  // 状态
    std::string department;              // 部门
    std::chrono::system_clock::time_point createdTime;  // 创建时间
    std::chrono::system_clock::time_point lastLoginTime;  // 最后登录时间
    std::set<std::string> permissions;  // 额外权限
    std::string profilePicturePath;      // 头像路径
    std::string bio;                    // 个人简介
};

/**
 * @brief 权限操作枚举
 */
enum class Permission {
    Read,          // 读取数据
    Write,         // 写入数据
    Delete,        // 删除数据
    Share,         // 分享数据
    ManageUsers,    // 管理用户
    ManageSystem,   // 管理系统
    ExportData,    // 导出数据
    ImportData     // 导入数据
};

/**
 * @brief 用户服务接口
 */
class IUserService {
public:
    virtual ~IUserService() = default;
    
    /**
     * @brief 添加新用户
     * 
     * @param user 用户信息
     * @return 是否成功
     */
    virtual bool addUser(const User& user) = 0;
    
    /**
     * @brief 更新用户信息
     * 
     * @param user 用户信息
     * @return 是否成功
     */
    virtual bool updateUser(const User& user) = 0;
    
    /**
     * @brief 删除用户
     * 
     * @param userId 用户ID
     * @return 是否成功
     */
    virtual bool deleteUser(const std::string& userId) = 0;
    
    /**
     * @brief 获取用户信息
     * 
     * @param userId 用户ID
     * @return 用户信息（如果存在）
     */
    virtual std::unique_ptr<User> getUser(const std::string& userId) = 0;
    
    /**
     * @brief 根据用户名获取用户
     * 
     * @param username 用户名
     * @return 用户信息（如果存在）
     */
    virtual std::unique_ptr<User> getUserByUsername(const std::string& username) = 0;
    
    /**
     * @brief 获取所有用户
     * 
     * @return 所有用户列表
     */
    virtual std::vector<User> getAllUsers() = 0;
    
    /**
     * @brief 检查用户是否有特定权限
     * 
     * @param userId 用户ID
     * @param permission 权限
     * @return 是否有权限
     */
    virtual bool hasPermission(const std::string& userId, Permission permission) = 0;
    
    /**
     * @brief 授权给用户
     * 
     * @param userId 用户ID
     * @param permission 权限
     * @return 是否成功
     */
    virtual bool grantPermission(const std::string& userId, Permission permission) = 0;
    
    /**
     * @brief 撤销用户权限
     * 
     * @param userId 用户ID
     * @param permission 权限
     * @return 是否成功
     */
    virtual bool revokePermission(const std::string& userId, Permission permission) = 0;
    
    /**
     * @brief 验证用户登录
     * 
     * @param username 用户名
     * @param password 密码
     * @return 用户ID（验证成功）或空字符串（失败）
     */
    virtual std::string authenticateUser(const std::string& username, const std::string& password) = 0;
};

/**
 * @brief 用户服务实现类
 * 
 * 使用内存存储用户数据（实际生产中可替换为数据库实现）
 */
class UserService : public IUserService {
private:
    std::vector<User> m_users;
    std::map<std::string, std::string> m_userCredentials; // 用户名 -> 密码哈希
    std::map<UserRole, std::set<Permission>> m_rolePermissions;
    
    /**
     * @brief 初始化角色权限映射
     */
    void initializeRolePermissions();
    
    /**
     * @brief 生成密码哈希
     * 
     * @param password 明文密码
     * @return 密码哈希
     */
    std::string hashPassword(const std::string& password);
    
    /**
     * @brief 验证密码
     * 
     * @param password 明文密码
     * @param hash 密码哈希
     * @return 是否匹配
     */
    bool verifyPassword(const std::string& password, const std::string& hash);
    
public:
    UserService();
    
    bool addUser(const User& user) override;
    bool updateUser(const User& user) override;
    bool deleteUser(const std::string& userId) override;
    std::unique_ptr<User> getUser(const std::string& userId) override;
    std::unique_ptr<User> getUserByUsername(const std::string& username) override;
    std::vector<User> getAllUsers() override;
    bool hasPermission(const std::string& userId, Permission permission) override;
    bool grantPermission(const std::string& userId, Permission permission) override;
    bool revokePermission(const std::string& userId, Permission permission) override;
    std::string authenticateUser(const std::string& username, const std::string& password) override;
};

/**
 * @brief 权限管理工具
 */
class PermissionUtils {
public:
    /**
     * @brief 将角色转换为字符串
     * 
     * @param role 角色
     * @return 字符串表示
     */
    static std::string roleToString(UserRole role);
    
    /**
     * @brief 将状态转换为字符串
     * 
     * @param status 状态
     * @return 字符串表示
     */
    static std::string statusToString(UserStatus status);
    
    /**
     * @brief 将权限转换为字符串
     * 
     * @param permission 权限
     * @return 字符串表示
     */
    static std::string permissionToString(Permission permission);
    
    /**
     * @brief 检查角色是否包含权限
     * 
     * @param role 角色
     * @param permission 权限
     * @return 是否包含
     */
    static bool roleHasPermission(UserRole role, Permission permission);
};

} // namespace Collaboration
} // namespace Core
} // namespace BondForge